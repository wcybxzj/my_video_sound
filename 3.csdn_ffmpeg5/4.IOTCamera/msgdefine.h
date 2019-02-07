//
//  CC_MSGDataDefine.h
//  IOTC_Camera
//
//  Created by chenchao on 13-2-27.
//  Copyright (c) 2013年 chenchao. All rights reserved.
//

#ifndef CC_MSGDataDefine_h
#define CC_MSGDataDefine_h

#define MJCAM_DEVICE_MAX_NUM  9

#define    CONTROLLCODE_LOGINREQUEST                0    //登陆请求  (操作命令)
#define    CONTROLLCODE_LOGINREPLY                  1    //登陆回应  (操作命令)
#define    CONTROLLCODE_VERIFIYREQUEST              2    //验证请求  (操作命令)
#define    CONTROLLCODE_VERIFIYREPLY                3    //验证回应  (操作命令)
#define    CONTROLLCODE_VIDEOTRANSLATION_REQUEST    4    //视频请求  (操作命令)
#define    CONTROLLCODE_VIDEOTRANSLATION_REPLY      5    //视频回应  (操作命令)
#define    CONTROLLCODE_VIDEOTRANSLATION_STOP       6    //视频停止  (操作命令)

#define    CONTROLLCODE_LISTENSTART_COMMAND         8    //开始音频  (操作命令)
#define    CONTROLLCODE_LISTENSTART_REPLY           9    //音频回应  (操作命令)
#define    CONTROLLCODE_LISTENSTOP_COMMAND          10   //停止音频  (操作命令)



#define    CONTROLLCODE_SEARCH_BROADCAST_REQUEST    0   //广播请求
#define    CONTROLLCODE_SEARCH_BROADCAST_REPLY      1   //广播回应

#define    CONTROLLCODE_DECODER_CONTROLL            14   //解码器      (操作命令)
#define    CONTROLLCODE_REPLY_ALARM                 25   //报警        (操作命令)
#define    CONTROLLCODE_MULTIVIDEOINFOREPLY         28   //多路信息操   (操作命令)

#define    CONTROLLCODE_VIDEOTRANSPORTCOMMD         0   //视频传输请   (传输命令)
#define    CONTROLLCODE_VIDEOTRANSPORTREPLY         1   //视频传输回应   (传输命令)
#define    CONTROLLCODE_LISTENTRANSPORTREPLY        2   //监听回应操作码      (传输命令)
#define    CONTROLLCODE_TALKTRANSPORTREPLY          3   //对讲回应操作码      (传输命令)


#define    CONTROLLCODE_USERDATA_REQ               32
#define    CONTROLLCODE_USERDATA_REPLY             33

#define    CONTROLLCODE_LASER_CTRL_REQ             34
#define    CONTROLLCODE_LASER_CTRL_REPLY           35
#define    CONTROLLCODE_AIR_QUALITY_REQ            36
#define    CONTROLLCODE_AIR_QUALITY_REPLY          37
#define    CONTROLLCODE_WRITE_UID_REQ              38
#define    CONTROLLCODE_WRITE_UID_REPLY            39
#define    CONTROLLCODE_SET_WIFI_REQ               40
#define    CONTROLLCODE_SET_WIFI_REPLY             41
#define    CONTROLLCODE_GET_SD_REQ                 42
#define    CONTROLLCODE_GET_SD_REPLY               43

#define    CONTROLLCODE_KEEPALIVECOMMAND            255

#define    LOGINREQUESTREPLYVALUE_OK                0
#define    LOGINREQUESTREPLYVALUE_BADACCESS         1

#define     VERIFYREPLYRETURNVALUE_OK               0   //较验正确
#define     VERIFYREPLYRETURNVALUE_USER_ERROR       1   //用户名出错
#define     VERIFYREPLYRETURNVALUE_PASS_ERROR       5   //密码出错

#define     VIDEOIREQUESTREPLY_OK                   0   //同意连接
#define     VIDEOIREQUESTREPLY_USERFULL             2   //用户已经满
#define     VIDEOIREQUESTREPLY_FORBIDEN             8   //禁止连接



#pragma pack(1)

//协议头文件.
typedef struct CC_MessageHeader
{
    unsigned char           messageHeader[4];    //协议头  摄像头操作协议: "CCTC" 摄像头传输协议 "CCTD"
    short                   controlMask;         //操作码，区分同一协议中不同命令.
    unsigned char           reserved0;            //保留，默认=0;
    unsigned char           reserved1[8];         //保留
    int                     commandLength;        //命令中正文的长度
    int                     reserved2;            //保留
    
}CC_MsgHeader;

//网络连接信息结构体
typedef struct NetConnectInfomation
{
    char server_ip[64];   //camIP
    int  port;            //端口
    char user_name[13];	  //用户名
    char pass_word[13];	  //密码
    
}CC_NetConnectInfo;


//登陆响应结构体
typedef struct LoginRequestReply
{
    short           result;   //返回 0 OK,2 已经达到最大连接许可，连接将断开.
    unsigned char   devID[13];  //返回设备id.
    unsigned char   reserved0[4];  //保留
    unsigned char   reserved1[4];  //保留
    unsigned char   devVersion[4];  //摄像头系统固件版本.
    
}CC_LoginRequestReply;



//较验请求正文结构体
typedef struct VerifyRequestCommContent
{
    unsigned char   userName[13];      //用户名
    unsigned char   password[13];      //密码
    
}CC_VerifyRequestCommContent;


//较验响应结构体
typedef struct verifyRequestReply
{
    short           result; //0 较验正确 1 用户名出错 2 密码出错.
    char            reserved;    //保留
    
}CC_VerifyRequestReply;


//视频传输正文
typedef struct videoTranslationRequest
{
    char          reserved;   //保留
    
}CC_videoTranslationRequest;

//视频响应.
typedef struct videoTranslationRequestReply
{
    short                   result;   //0: 同意 2 超过最大连接数被拒绝.
    unsigned int             videoID;  //当Result=0 并且之前没有进行因视频传输时，本字段才存在.用来标识数据连接的ID.
    
}CC_videoTranslationRequestReply;


//发送视频数据请求
typedef struct videoTranslationRequestID
{
    //CC_MsgHeader    msgHeader;
    int                videoID;
    
}CC_videoTranslationRequestID;


//视频正文结构体
typedef struct videoDataContent
{
    unsigned int             timeStamp; //时间戳
    unsigned int             frameTime; //帧采集时间
    unsigned char            reserved;  //保留
    unsigned int             pictureLength;  //图片长度
    
}CC_videoDataContent;

//音频正文结构体
typedef struct listeningDataContent
{
    unsigned int             timeStamp;//时间戳
    unsigned int             packageNumber; //包序号
    unsigned int             collectionTime; //采集时间
    char                     audioFormat;    //音频格式
    unsigned int             dataLength;     //数据长度
    
}CC_audioDataContent;


//监听请求结构体
typedef struct audioRequestCommand
{
    //CC_MsgHeader msgHeader;
    char            reserved;    //保留
}CC_audioRequestCommand;


//音频接收响应正文
typedef struct audioRequestCommandReply
{
    short           result; //0：同意 2:超过最大连接数拒绝 7: 机器不支持此功能
    int             audioID; //数据连接ID.
}CC_audioRequestCommandReply;


#pragma pack()


#define     INT8           unsigned char
#define     INT16          unsigned short     
#define     INT32          unsigned int 

#endif
