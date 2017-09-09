/**
 * 最简单的视音频数据处理示例
 * 雷霄骅 Lei Xiaohua
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
	NALU_TYPE_SLICE    = 1,
	NALU_TYPE_DPA      = 2,
	NALU_TYPE_DPB      = 3,
	NALU_TYPE_DPC      = 4,
	NALU_TYPE_IDR      = 5,
	NALU_TYPE_SEI      = 6,
	NALU_TYPE_SPS      = 7,
	NALU_TYPE_PPS      = 8,
	NALU_TYPE_AUD      = 9,
	NALU_TYPE_EOSEQ    = 10,
	NALU_TYPE_EOSTREAM = 11,
	NALU_TYPE_FILL     = 12,
} NaluType;

typedef enum {
	NALU_PRIORITY_DISPOSABLE = 0,
	NALU_PRIRITY_LOW         = 1,
	NALU_PRIORITY_HIGH       = 2,
	NALU_PRIORITY_HIGHEST    = 3
} NaluPriority;


typedef struct
{
	int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
	unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	unsigned max_size;            //! Nal Unit Buffer size
	int forbidden_bit;            //! should be always FALSE
	int nal_reference_idc;        //! NALU_PRIORITY_xxxx
	int nal_unit_type;            //! NALU_TYPE_xxxx
	char *buf;                    //! contains the first byte followed by the EBSP
} NALU_t;

FILE *h264bitstream = NULL;                //!< the bit stream file

int info2=0, info3=0;

static int FindStartCode2 (unsigned char *Buf){
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //0x000001?
	else return 1;
}

static int FindStartCode3 (unsigned char *Buf){
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//0x00000001?
	else return 1;
}

int GetAnnexbNALU (NALU_t *nalu){
	int pos = 0;
	int StartCodeFound, rewind;
	unsigned char *Buf;
	int i;
	unsigned char c;

	if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL)
		printf ("GetAnnexbNALU: Could not allocate Buf memory\n");

	nalu->startcodeprefix_len=3;

	if (3 != fread (Buf, 1, 3, h264bitstream)){
		free(Buf);
		return 0;
	}
	info2 = FindStartCode2 (Buf);
	if(info2 != 1) {
		if(1 != fread(Buf+3, 1, 1, h264bitstream)){
			free(Buf);
			return 0;
		}
		info3 = FindStartCode3 (Buf);
		if (info3 != 1){
			free(Buf);
			return -1;
		}
		else {
			pos = 4;
			nalu->startcodeprefix_len = 4;
		}
	}
	else{
		nalu->startcodeprefix_len = 3;
		pos = 3;
	}
	StartCodeFound = 0;
	info2 = 0;
	info3 = 0;

	//printf("%d\n",Buf[0]);//0
	//printf("%d\n",Buf[1]);//0
	//printf("%d\n",Buf[2]);//0
	//printf("%d\n",Buf[3]);//1
	//printf("pos:%d\n",pos);//4

	while (!StartCodeFound){
		if (feof (h264bitstream)){
			nalu->len = (pos-1)-nalu->startcodeprefix_len;
			memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
			nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
			nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
			nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
			free(Buf);
			return pos-1;
		}
		Buf[pos++] = fgetc (h264bitstream);
		info3 = FindStartCode3(&Buf[pos-4]);
		//printf("%d\n", Buf[pos-4]);
		//printf("info3:%d\n", info3);
		if(info3 != 1)
			info2 = FindStartCode2(&Buf[pos-3]);
		StartCodeFound = (info2 == 1 || info3 == 1);
		//printf("StartCodeFound:%d\n", StartCodeFound);
		//exit(1);
	}

	//printf("pos:%d\n",pos);//747
	//printf("info2:%d\n",info2);//0
	//printf("info3:%d\n",info3);//1
	//info3 = FindStartCode3(&Buf[pos-4]);

	//c = (unsigned char) Buf[pos-4-2];
	//printf("%x\n", c);
	//c = (unsigned char) Buf[pos-4-1];
	//printf("%x\n", c);
	//for (i=0; i<4 ; i++) {
	//	c = (unsigned char) Buf[pos-4+i];
	//	printf("%x\n", c);
	//}
	//exit(1);

	//找到另外一个NALU,也就是说第一个NALU已经结束
	// Here, we have found another start code (and read length of startcode bytes more than we should
	// have.  Hence, go back in the file
	rewind = (info3 == 1)? -4 : -3;

	if (0 != fseek (h264bitstream, rewind, SEEK_CUR)){
		free(Buf);
		printf("GetAnnexbNALU: Cannot fseek in the bit stream file");
	}

	//printf("pos:%d\n", pos);//747
	//printf("rewind:%d\n", rewind);//-4
	//printf("%d\n",nalu->startcodeprefix_len);//4
	//exit(1);

	//NALU头结构
	//长度：1byte
	//forbidden_bit(1bit) + nal_reference_bit(2bit) + nal_unit_type(5bit)
	//1.forbidden_bit:
	//禁止位，初始为0，当网络发现NAL单元有比特错误时可设置该比特为1，以便接收方纠错或丢掉该单元。
	//2.nal_reference_bit:
	//nal重要性指示，标志该NAL单元的重要性，值越大，越重要，解码器在解码处理不过来的时候，可以丢掉重要性为0的NALU。

	// Here the Start code, the complete NALU, and the next start code is in the Buf.
	// The size of Buf is pos, pos+rewind are the number of bytes excluding the next
	// start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code
	nalu->len = (pos+rewind)-nalu->startcodeprefix_len;//739
	memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//
	nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
	nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
	nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
	free(Buf);

	//printf("%d\n",nalu->forbidden_bit);//0
	//printf("%d\n",nalu->nal_reference_idc);//0
	//printf("%d\n",nalu->nal_unit_type    );//6

	//for (i = 0; i < nalu->len; i++) {
	//	printf("i:%d\n", i);
	//	c = (unsigned char) nalu->buf[i];
	//	printf("%x\n", c);
	//}
	//exit(1);

	return (pos+rewind);//747-4
}

/**
 * Analysis H.264 Bitstream
 * @param url    Location of input H.264 bitstream file.
 */
int simplest_h264_parser(char *url){

	NALU_t *n;
	int buffersize=100000;

	//FILE *myout=fopen("output_log.txt","wb+");
	FILE *myout=stdout;

	h264bitstream=fopen(url, "rb+");
	if (h264bitstream==NULL){
		printf("Open file error\n");
		return 0;
	}

	n = (NALU_t*)calloc (1, sizeof (NALU_t));
	if (n == NULL){
		printf("Alloc NALU Error\n");
		return 0;
	}

	n->max_size=buffersize;
	n->buf = (char*)calloc (buffersize, sizeof (char));
	if (n->buf == NULL){
		free (n);
		printf ("AllocNALU: n->buf");
		return 0;
	}

	int data_offset=0;
	int nal_num=0;
	printf("-----+-------- NALU Table ------+---------+\n");
	printf(" NUM |    POS  |    IDC |  TYPE |   LEN   |\n");
	printf("-----+---------+--------+-------+---------+\n");

	while(!feof(h264bitstream))
	{
		int data_lenth;
		data_lenth=GetAnnexbNALU(n);

		//printf("---%d---\n", data_lenth);//743
		//exit(1);

		char type_str[20]={0};
		switch(n->nal_unit_type){
			case NALU_TYPE_SLICE:sprintf(type_str,"SLICE");break;
			case NALU_TYPE_DPA:sprintf(type_str,"DPA");break;
			case NALU_TYPE_DPB:sprintf(type_str,"DPB");break;
			case NALU_TYPE_DPC:sprintf(type_str,"DPC");break;
			case NALU_TYPE_IDR:sprintf(type_str,"IDR");break;
			case NALU_TYPE_SEI:sprintf(type_str,"SEI");break;
			case NALU_TYPE_SPS:sprintf(type_str,"SPS");break;
			case NALU_TYPE_PPS:sprintf(type_str,"PPS");break;
			case NALU_TYPE_AUD:sprintf(type_str,"AUD");break;
			case NALU_TYPE_EOSEQ:sprintf(type_str,"EOSEQ");break;
			case NALU_TYPE_EOSTREAM:sprintf(type_str,"EOSTREAM");break;
			case NALU_TYPE_FILL:sprintf(type_str,"FILL");break;
		}

		char idc_str[20]={0};
		switch(n->nal_reference_idc>>5){
			case NALU_PRIORITY_DISPOSABLE:sprintf(idc_str,"DISPOS");break;
			case NALU_PRIRITY_LOW:sprintf(idc_str,"LOW");break;
			case NALU_PRIORITY_HIGH:sprintf(idc_str,"HIGH");break;
			case NALU_PRIORITY_HIGHEST:sprintf(idc_str,"HIGHEST");break;
		}

		fprintf(myout,"%5d| %8d| %7s| %6s| %8d|\n",nal_num,data_offset,idc_str,type_str,n->len);

		data_offset=data_offset+data_lenth;

		nal_num++;
	}

	//Free
	if (n){
		if (n->buf){
			free(n->buf);
			n->buf=NULL;
		}
		free (n);
	}
	return 0;
}

/*
./1.h264 |head -20
-----+-------- NALU Table ------+---------+
 NUM |    POS  |    IDC |  TYPE |   LEN   |
-----+---------+--------+-------+---------+
    0|        0|  DISPOS|    SEI|      739|
    1|      743| HIGHEST|    SPS|       24|
    2|      771| HIGHEST|    PPS|        5|
    3|      780| HIGHEST|    IDR|    10499|
    4|    11282|    HIGH|  SLICE|      386|
    5|    11672|    HIGH|  SLICE|       40|
    6|    11716|  DISPOS|  SLICE|       30|
    7|    11750|    HIGH|  SLICE|      445|
    8|    12199|    HIGH|  SLICE|       38|
    9|    12241|  DISPOS|  SLICE|       24|
   10|    12269|    HIGH|  SLICE|      288|
*/
int main(int argc, const char *argv[])
{
	simplest_h264_parser("/root/www/video_sound_data/3.h264_data/out.h264");
	return 0;
}

