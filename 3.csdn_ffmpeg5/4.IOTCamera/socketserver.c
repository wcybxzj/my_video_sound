
#include "socketserver.h"
#include "msgdefine.h"
#include "h264capture.h"


pthread_mutex_t  mutex_cSend=PTHREAD_MUTEX_INITIALIZER;


int 	m_ConnectSocket = 0;
int		m_ListeningSocket = 0;

char	*m_TempBuffer;
int		m_nRevDataSize = 0;


int		m_video_status = 0;
int		m_audio_status = 0;
int		m_talk_status = 0;

pthread_mutex_t m_sendDataMutex;

void* start_send_video(void* arg);

int detach_thread_create(pthread_t *thread, void * start_routine, void *arg)
{
    pthread_attr_t attr;
    pthread_t thread_t;
    int ret = 0;

    if(thread==NULL){
        thread = &thread_t;
    }
    //初始化线程的属性
    if(pthread_attr_init(&attr))
    {
        printf("pthread_attr_init fail!\n");
        return -1;
    }

    //设置线程detachstate属性。该表示新线程是否与进程中其他线程脱离同步
    if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
    {//新线程不能用pthread_join()来同步，且在退出时自行释放所占用的资源。
        printf("pthread_attr_setdetachstate fail!\n");
        goto error;
    }

    ret = pthread_create(thread, &attr, (void *(*)(void *))start_routine, arg);
    if(ret < 0){
        printf("pthread_create fail!\n");
        goto error;
    }

    //将状态改为unjoinable状态，确保资源的释放。
   ret =  pthread_detach(thread_t);

error:
    pthread_attr_destroy(&attr);

    return ret;
}

int init_socket_params(int  SenSocket)
{
    int result;
    int opt;
    struct timeval to;
    
    opt = 1;
    result = setsockopt(SenSocket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    opt = SOCK_SNDRCV_LEN;
    result = setsockopt(SenSocket, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt));
    
    to.tv_sec  = 5;
    to.tv_usec = 0;
    result = setsockopt(SenSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&to,sizeof(to));

    fcntl(SenSocket, F_SETFL, O_NONBLOCK);
    return 1;
}


int tcp_receive(int hSock, char *pBuffer, unsigned int nSize)
{
    int ret = 0;
    fd_set fset;
    struct timeval to;
    unsigned int  dwRecved = 0;
    
    memset(&to,0,sizeof(to));
    
    if (hSock < 0 || hSock > 65535 || nSize <= 0)
    {
        return -2;
    }
    
    while (dwRecved < nSize)
    {
        FD_ZERO(&fset);
        FD_SET(hSock, &fset);
        to.tv_sec = 10;
        to.tv_usec = 0;
        
        ret = select(hSock+1, &fset, NULL, NULL, &to);  //&to
        if ( ret == 0 || (ret == -1 && errno == EINTR))
        {
            return -1;
        }
        if (ret == -1)
        {
            return -2;
        }
        
        if(!FD_ISSET(hSock, &fset))
        {
            return -1;
        }
        
        ret = recv(hSock, pBuffer + dwRecved, nSize - dwRecved, 0);
        if( (ret < 0) && (errno == EAGAIN || errno == EINTR ||errno == EWOULDBLOCK))
        {
            continue;
        }
        if (ret <= 0)
        {
            printf("TcpReceive recv return %d errno = %d\n",ret,errno);
            return -2;
        }
        dwRecved += ret;
    }
    return dwRecved;
}


int mega_send_data(int nSock, char *buf, int len)
{
    // if nSock is closed, return bytes is -1
    int bytes = -1;
    
    int length = len;
    
    do
    {
    ReSend:
        bytes = send(nSock, buf, length, 0);
        if ( bytes > 0 )
        {
            length -= bytes;
            buf += bytes;
        }
        else if (bytes <0)
        {
            if (errno == EINTR || errno == EWOULDBLOCK ||errno ==EAGAIN)
                goto ReSend;
        }
        else if(bytes==0 || bytes ==-1){
            return -1;
        }
    }
    while ( length > 0 && bytes > 0 );
    if ( length == 0 )
        bytes = len;
    return bytes;
}

int xsocket_tcp_send(int sock, char *buf, int len)
{
    int		ret;
    
    fd_set wfds;
    struct timeval tv;
    
    while(1)
    {
        FD_ZERO(&wfds);
        FD_SET(sock, &wfds);
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        ret = select(sock + 1, NULL, &wfds, NULL, &tv);
        if (ret == 0)  /* timeout */
        {
            continue;
        }
        if(ret < 0 && ret == EINTR)
        {
            usleep(1);
            continue;
        }
        break;
    }
    
    if( ret < 0 )
    {
        return -1; //timeout obut the socket is not ready to write; or socket is error
    }
    
    ret = mega_send_data(sock, buf, len);

    return ret;
}


int resp_net_Send(int socket, CC_MsgHeader	*pMsgHead, void *pSendBuf)
{
    int ret = 0;
    char *lpbuf = 0;

    
    ret = xsocket_tcp_send(socket, (char *)pMsgHead, sizeof(CC_MsgHeader));

    if(ret != sizeof(CC_MsgHeader))
    {
        printf("JB_Video_Net_Send Client messageHeader(%s) Command(%d) Failed\n", pMsgHead->messageHeader, pMsgHead->controlMask);
        return -2;
    }
    
    ret = xsocket_tcp_send(socket, pSendBuf+sizeof(CC_MsgHeader), pMsgHead->commandLength);

    if(ret != pMsgHead->commandLength)
    {
        printf("JB_Video_Net_Send Client messageHeader(%s) Command(%d) Failed\n", pMsgHead->messageHeader, pMsgHead->controlMask);
        return -2;
    }

    return ret;
}



int process_client_message(int socket, CC_MsgHeader *lpcommhead)
{
    char 				*pCommData = NULL;
    int 				ret;
    CC_MsgHeader 	toclientMsgHeader;
    char				replyBuf[512];
    memset(replyBuf, 0, sizeof(replyBuf));
    
    if(lpcommhead->commandLength > 0)
    {
        pCommData = (char *)malloc(lpcommhead->commandLength);
        ret = tcp_receive(socket, pCommData, lpcommhead->commandLength);
        if (-2 == ret)
        {
            return -1;
        }
        if (-1 == ret)
        {
            return 0;
        }
    }
    

    toclientMsgHeader.messageHeader[0] = 'C';
    toclientMsgHeader.messageHeader[1] = 'C';
    toclientMsgHeader.messageHeader[2] = 'T';
    toclientMsgHeader.messageHeader[3] = 'C';
    
    
    switch(lpcommhead->controlMask)
    {
        case CONTROLLCODE_LOGINREQUEST:
        {
            CC_LoginRequestReply *reply = (CC_LoginRequestReply *)(replyBuf + sizeof(CC_MsgHeader));
            
            reply->result = 0;
            strcpy(reply->devID, "CC0123456");
            strcpy(reply->devVersion, "111");
            toclientMsgHeader.controlMask = CONTROLLCODE_LOGINREPLY;
            toclientMsgHeader.commandLength = sizeof(CC_LoginRequestReply);
            resp_net_Send(socket, &toclientMsgHeader, replyBuf);
        }
            break;
        case CONTROLLCODE_VERIFIYREQUEST:
        {
            CC_VerifyRequestCommContent *pReq = (CC_VerifyRequestCommContent *)pCommData;
            CC_VerifyRequestReply *reply = (CC_VerifyRequestReply *)(replyBuf + sizeof(CC_MsgHeader));
            
            //比较用户名和密码
            //...
            reply->result = VERIFYREPLYRETURNVALUE_OK;
            toclientMsgHeader.controlMask = CONTROLLCODE_VERIFIYREPLY;
            toclientMsgHeader.commandLength = sizeof(CC_VerifyRequestReply);
            resp_net_Send(socket, &toclientMsgHeader, replyBuf);
        }
            break;
        case CONTROLLCODE_VIDEOTRANSLATION_REQUEST:
        {
            CC_videoTranslationRequest *pReq = (CC_videoTranslationRequest *)pCommData;
            CC_videoTranslationRequestReply *reply = (CC_videoTranslationRequestReply *)(replyBuf + sizeof(CC_MsgHeader));
            reply->result = 0;
            reply->videoID = 1;
            toclientMsgHeader.controlMask = CONTROLLCODE_VIDEOTRANSLATION_REPLY;
            toclientMsgHeader.commandLength = sizeof(CC_videoTranslationRequestReply);
            resp_net_Send(socket, &toclientMsgHeader, replyBuf);
        }
            break;
        case CONTROLLCODE_VIDEOTRANSLATION_STOP:
        {
            if(m_video_status)
            {
                m_video_status = 0;
            }
        }
            break;
        case CONTROLLCODE_LISTENSTART_COMMAND:
        {
            printf("START LISTEN:::\n");
            CC_audioRequestCommand *pReq = (CC_audioRequestCommand *)pCommData;
            CC_audioRequestCommandReply *reply = (CC_audioRequestCommandReply *)(replyBuf + sizeof(CC_MsgHeader));
            reply->result = 0;
            reply->audioID = 1;
            toclientMsgHeader.controlMask = CONTROLLCODE_LISTENSTART_REPLY;
            toclientMsgHeader.commandLength = sizeof(CC_audioRequestCommandReply);
            resp_net_Send(socket, &toclientMsgHeader, replyBuf);
            m_audio_status = 1;

        }
            break;
        case CONTROLLCODE_LISTENSTOP_COMMAND:
        {
            printf("STOP LISTEN:::\n");
            if(m_audio_status)
            {
                m_audio_status = 0;
            }
        }
         break;

        case CONTROLLCODE_DECODER_CONTROLL:
        {

        }
            break;
        default:
            break;
    }
    
    if(pCommData != NULL)
    {
        free(pCommData);
        pCommData = NULL;
    }
    return 0;
}


void* thread_process_message(void *lpVoid)
{
    int 				nKeepAlive = 0;
    int					msg_socket;
    fd_set      		readfds;
    struct timeval		tv;
    int					nMaxfd;
    CC_MsgHeader		commhead;
    memset(&commhead, 0, sizeof(commhead));
    int nSocket = -1;
    nSocket = *((int*)lpVoid);
    if(lpVoid)
    {
        free(lpVoid);
    }
    while(1)
    {
        nMaxfd = 0;
        nMaxfd = nSocket;
        msg_socket = nSocket;
        //printf("---------------------nMaxfd:%d  nKeepAlive:%d\n",nMaxfd, nKeepAlive);
        
        tv.tv_sec  = 1; // 秒数
        tv.tv_usec = 0; // 微秒
        
        FD_ZERO(&readfds);
        FD_SET(msg_socket, &readfds);
        
        //readfds:
        //	1、If listen has been called and a connection is pending, accept will succeed.
        //	2、Data is available for reading (includes OOB data if SO_OOBINLINE is enabled).
        //	3、Connection has been closed/reset/terminated.
        errno = 0;	//清除错误
        int nreturn = select(nMaxfd + 1,&readfds,NULL, NULL,&tv);//执行成功则返回文件描述词状态已改变的个数
        //printf("nreturn %d \n", nreturn);
        if(nreturn == 0)
        {
            usleep(1000);
            //超时，给各个的KeepAlive+1
            nKeepAlive++;
            if(nKeepAlive > 60)
            {
                printf("line:%d fun:%s -------------\n", __LINE__, __FUNCTION__);
                close(msg_socket);
                break;
            }
            continue;
        }
        if(nreturn > 0)
        {
            //printf("line:%d fun:%s -------------nreturn(%d) nKeepAlive:%d msg_socket:%d\n", __LINE__, __FUNCTION__, nreturn, nKeepAlive, msg_socket);
            
            int fdSet = FD_ISSET(msg_socket, &readfds);
            usleep(5);
            //printf("fdSet %d \n", fdSet);
            if(fdSet)
            {
                int bytes = 0;
                int nerr = errno;
                ioctl(msg_socket, FIONREAD, &bytes);
                if(bytes == 0)
                {
                    nerr = errno;
                    nKeepAlive++;		//主动关闭会导致连续读0
                    if(nKeepAlive > 30)
                    {
                        printf("line:%d fun:%s -------------\n", __LINE__, __FUNCTION__);
                        close(msg_socket);
                        continue;
                    }
                }
                nreturn = recv(msg_socket,&commhead,sizeof(commhead),0);
                //if(commhead.controlMask != CONTROLLCODE_KEEPALIVECOMMAND)
                //{
                //	printf("nreturn %d \n", nreturn);
                //}
                if(nreturn < 0)
                {
                    //printf("line:%d fun:%s -------------\n", __LINE__, __FUNCTION__);
                    close(msg_socket);
                    msg_socket = -1;
                    continue;
                }
                if(nreturn == sizeof(commhead))
                {

                    nKeepAlive = 0;
                    if(commhead.controlMask == CONTROLLCODE_KEEPALIVECOMMAND)
                    {
                        printf("KEEP ALIVE PACAKGE commhead.nCommand [%d]\n", commhead.controlMask);
                        nKeepAlive = 0;
                        continue ;
                    }

                    if(commhead.commandLength >= 0)
                    {
                        printf("messageHeader[%s] controlMask[%d] commandLength[%d]-------\n", commhead.messageHeader, commhead.controlMask, commhead.commandLength);
                        process_client_message(msg_socket, &commhead);
                    }
                    
                }
            }
            else
            {
                //没有发生网络消息，则在一般情况下，未来的1秒内会收到至少1次nKeepAlive，把计数器清0
                nKeepAlive++;
                //printf("line:%d fun:%s -------------lpInfo->nKeepAlive:%d lpInfo->nSocket:%d\n", __LINE__, __FUNCTION__,lpInfo3->nKeepAlive,lpInfo3->nSocket);
                if(nKeepAlive > 60)
                {
                    printf("line:%d fun:%s -------------\n", __LINE__, __FUNCTION__);
                    close(msg_socket);
                    continue;
                }
            }
        }
        else
        {
            int nerr = errno;
            printf("select errno %d %s\n", nerr,strerror(nerr));
            close(msg_socket);
            m_video_status = 0;
            m_audio_status = 0;
            break;
        }
    }
    return ;
}




int process_login_command(struct sockaddr_in *lpaddr)
{
    init_socket_params(m_ConnectSocket);
    //把非阻塞变为阻塞
    int flags = fcntl(m_ConnectSocket, F_GETFL, 0);
    fcntl(m_ConnectSocket, F_SETFL,flags & ~O_NONBLOCK);
    int cmdSocket = -1;
    char *pcmdSocket = (char *)malloc(4);
    cmdSocket = m_ConnectSocket;
    memcpy(pcmdSocket, &cmdSocket, 4);

    detach_thread_create(NULL,thread_process_message,pcmdSocket);

    m_ConnectSocket = -1;
    return 0;
}


#define READ_STATUS 			0
#define WRITE_STATUS 			1
#define EXCPT_STATUS 			2

int net_select(int s, int sec, int usec, short x)
{
    int st = errno;
    struct timeval to;
    fd_set fs;
    to.tv_sec = sec;
    to.tv_usec = usec;
    FD_ZERO(&fs);
    FD_SET(s, &fs);
    
    switch(x)
    {
        case READ_STATUS:
            st = select(s+1, &fs, 0, 0, &to);
            break;
        case WRITE_STATUS:
            st = select(s+1, 0, &fs, 0, &to);
            break;
        case EXCPT_STATUS:
            st = select(s+1, 0, 0, &fs, &to);
            break;
    }
    return(st);
}


int block_socket_send(int  SenSocket, char *databuffer, int size)
{

    signal(SIGPIPE, SIG_IGN);
    int buffersize              = size;
    int leave_size              = size;
    int nRet                    = 0;
    int	sec                     = 20;
    int usec                    = 0;
    int part_sensize            = 8000;
    
    if(SenSocket <= 0)
    {
        printf("Invailid Socket %d, Net_Protocol::BlockSendData() failed.\n", SenSocket);
        return nRet;
    }
    
    while(leave_size > 0)
    {
        int nneedsend = 0;
        if(leave_size  >= part_sensize)
            nneedsend = part_sensize;
        else
            nneedsend = leave_size;
        nRet = net_select(SenSocket, sec, usec, WRITE_STATUS);
        if(nRet <= 0)
        {
            printf("%s %d socket %d Error:%d %s\n",__FUNCTION__,__LINE__,SenSocket, errno,strerror(errno));
            return -1;
        }

        nRet = send(SenSocket, databuffer + buffersize - leave_size , nneedsend, 0 );

        if(nRet < 0)
        {
            if(errno==EAGAIN || errno == EINTR ||errno == EWOULDBLOCK){
                usleep(100);
                continue;
            }
            printf("BlockSendData %d socket %d send : %d %d\n",__LINE__,SenSocket,errno,nRet);
        }
        if(nRet== -1 || nRet ==0){

            return -1;
        }

        if(leave_size == 0)
        {
            nRet = 1;
            break;
        }

        leave_size -= nRet;
    }
    return nRet;
}


int process_socket_command(struct sockaddr_in *lpaddr)
{
    int ret = 0;
    CC_MsgHeader *lpHead = (CC_MsgHeader *)m_TempBuffer;

    printf("ProcessSocketCommand Socket=%d\n",m_ConnectSocket);
    
    if(lpHead->messageHeader[3] == 'C')
    {
        //先登录
        if(lpHead->controlMask == CONTROLLCODE_LOGINREQUEST)
        {
            char				replyBuf[512];
            memset(replyBuf, 0, sizeof(replyBuf));
            CC_LoginRequestReply *reply = (CC_LoginRequestReply *)(replyBuf + sizeof(CC_MsgHeader));
            printf("\n");
            reply->result = 0;
            strcpy(reply->devID, "CC0123456");
            strcpy(reply->devVersion, "111");
            lpHead->controlMask = CONTROLLCODE_LOGINREPLY;
            lpHead->commandLength = sizeof(CC_LoginRequestReply);
            
            resp_net_Send(m_ConnectSocket, lpHead, (void *)replyBuf);
        }
        
        printf("处理命令 Controll mask: %d\n",lpHead->controlMask);
        process_login_command(lpaddr);
    }
    else if(lpHead->messageHeader[3] == 'D')
    {
        printf("open video!!\n");
        if(lpHead->commandLength > 0)
        {
            char 	*pCommData = NULL;
            pCommData = (char *)malloc(lpHead->commandLength);
            ret = tcp_receive(m_ConnectSocket, pCommData, lpHead->commandLength);
            if (-2 == ret)
            {
                return -1;
            }
            if (-1 == ret)
            {
                return 0;
            }
            if(lpHead->controlMask == CONTROLLCODE_VIDEOTRANSPORTCOMMD)
            {
                detach_thread_create(NULL,start_send_video,&m_ConnectSocket);
            }

            if(pCommData)
            {
                free(pCommData);
            }
        }
    }
    else
    {
        
    }
    
    return 0;
}

int client_soketProess(struct sockaddr_in *lpaddr)
{
    fd_set 		read_set;
    int  		wait_count = 0;
    int 		revsize  = 0;
    
    int 		buffersize = sizeof(CC_MsgHeader);//CMD_PART_LENTH;
    struct timeval 	tmval; 
    int			soketerror = 0;
    
    tmval.tv_sec = 20;
    tmval.tv_usec = 0;
    
    if(init_socket_params(m_ConnectSocket) < 0 || m_TempBuffer == 0)
    {
        if(m_ConnectSocket)
        {
            close(m_ConnectSocket);
            m_ConnectSocket = 0;
        }
        return -1;
    }
    
    
    FD_ZERO(&read_set);
    FD_SET(m_ConnectSocket,&read_set);
    printf("\n");
    m_nRevDataSize = 0;
    while((wait_count < 10) && (!soketerror) )
    {
        int ret = 0;
        //m_nRevDataSize = 0;
        ret = select(m_ConnectSocket+1,&read_set, NULL, NULL,&tmval);
        printf("ret = %d\n",ret);
        switch(ret)
        {
            case -1:  
                soketerror = 1;
                break;
            case 0:
                wait_count++;
                break;
            default:
                if( FD_ISSET( m_ConnectSocket, &read_set ))
                {
                    revsize = recv(m_ConnectSocket, m_TempBuffer+m_nRevDataSize, buffersize, 0 );
                    printf("revsize[%d] sizeof(CC_MsgHeader)[%d]\n", revsize,(int)sizeof(CC_MsgHeader));
                    if(revsize <= 0)
                    {
                        soketerror = 1;
                        break;
                    }

                    printf("m_nRevDataSize [%d]\n",m_nRevDataSize);
                    if(m_nRevDataSize >= sizeof(CC_MsgHeader))
                    {
                        CC_MsgHeader *lpHead = (CC_MsgHeader *)m_TempBuffer;
                        printf("messageHeader[%s] controlMask[%d] commandLength[%d]-------\n", lpHead->messageHeader, lpHead->controlMask, lpHead->commandLength);
                        //if(lpHead->nFlag == 9000)
                        {
                            break;
                        }
                    }
                    m_nRevDataSize += revsize;
                    buffersize = CMD_PART_LENTH - m_nRevDataSize;
                }
                else
                {
                    wait_count++;
                }
                break;
        }
        if(m_nRevDataSize == CMD_PART_LENTH)
            break;
        if(m_nRevDataSize == sizeof(CC_MsgHeader))
        {
            CC_MsgHeader *lpHead = (CC_MsgHeader *)m_TempBuffer;
            printf("messageHeader[%s] controlMask[%d] commandLength[%d]-------\n", lpHead->messageHeader, lpHead->controlMask, lpHead->commandLength);
            //if(lpHead->nFlag == 9000)
            {
                break;
            }
        }
    }
    if((soketerror || wait_count >= 5) && (!(m_nRevDataSize == CMD_PART_LENTH || m_nRevDataSize == sizeof(CC_MsgHeader))))
    {
        if(m_ConnectSocket)
        {
            close(m_ConnectSocket);
            m_ConnectSocket = 0;
        }
        return -1;
    }
    
    if(m_nRevDataSize == sizeof(CC_MsgHeader))
    {
        process_socket_command(lpaddr);
        return 0;
    }
    if(m_ConnectSocket)
    {
        close(m_ConnectSocket);
        m_ConnectSocket = 0;
    }
    return -1;
}



int accept_process()
{
    struct sockaddr_in    	peerIpAddr;
    int 					csocket;
    socklen_t               size;
    m_TempBuffer = (char *)malloc(TEMP_FILE_LIST_SIZE);
    pthread_mutex_init (&m_sendDataMutex, NULL);
    while( 1 )
    {
        size = sizeof(struct sockaddr_in);
        csocket = accept(m_ListeningSocket , ( struct sockaddr *)&peerIpAddr , &size);
        if (csocket > 0)
        {
            m_ConnectSocket = csocket;
            printf("======= AcceptProcess Address:%s\n",inet_ntoa(peerIpAddr.sin_addr));
            printf("Connected socket =  %d   m_ListeningSocket = %d\n", csocket, m_ListeningSocket);
            client_soketProess(&peerIpAddr);
            printf("\n");
        }
        usleep(2000000);
    }
    return 1;
}


void start_tcp_server()
{
    printf("tcp_toolingtest_start Client port %d \n", LISTEN_POT);
    
    struct sockaddr_in		client_addr;
    int				opt = 1;
    int				len = sizeof(int);
    
    if((m_ListeningSocket = socket(AF_INET,SOCK_STREAM,0))<0) 
    {
        goto failed;
    } 
    printf("socket success...sockfd: %d \n",m_ListeningSocket);
    if(setsockopt(m_ListeningSocket,SOL_SOCKET,SO_REUSEADDR,&opt,len)<0)
    { 
        goto failed;
    } 		 	
    
    fcntl(m_ListeningSocket, F_SETFL, O_NONBLOCK);
    bzero(&client_addr,sizeof(struct sockaddr_in)); 
    
    client_addr.sin_family = AF_INET; 
    client_addr.sin_port = htons(LISTEN_POT); 
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    
    int ret=bind(m_ListeningSocket, (struct sockaddr *)&client_addr, sizeof(client_addr));
    printf("Bind: %d\n",ret);
    if(ret < 0)
    {
        goto failed;
    }
    
    if(listen(m_ListeningSocket, 10) == -1)
    {
        goto failed;
    }


    accept_process();

    
failed:
    if(m_ListeningSocket)
    {
        close(m_ListeningSocket);
        m_ListeningSocket = 0;
    }
    
    pthread_exit(NULL);//return NULL;
}


void* start_send_video(void* arg)
{

    int socket= *((int*)arg);

    printf("Start发送视频..socket %d \n", socket);

    CC_videoDataContent	dataContentVideo;
    CC_MsgHeader 		msgHeaderVideo;

    m_video_status = 1;

    msgHeaderVideo.messageHeader[0] = 'C';
    msgHeaderVideo.messageHeader[1] = 'C';
    msgHeaderVideo.messageHeader[2] = 'T';
    msgHeaderVideo.messageHeader[3] = 'D';
    msgHeaderVideo.controlMask = CONTROLLCODE_VIDEOTRANSPORTREPLY;

    ////////////////////////////////////////////

    IOTC_Camera *cam = (IOTC_Camera *)malloc(sizeof(IOTC_Camera));
    if (!cam) {
       printf("malloc camera failure!\n");
       return;
    }

    memset(cam, 0, sizeof(IOTC_Camera));
    cam->device_name = "/dev/video0";
    cam->buffers = NULL;
    cam->width = 640;
    cam->height = 360;
    cam->display_depth = 5;		/* RGB24 */

    camera_open(cam);
    camera_init(cam);
    camera_capturing_start(cam);
    h264_encoder_init(&cam->encoder, cam->width, cam->height);
    cam->h264_buf = (uint8_t *) malloc(sizeof(uint8_t) * cam->width * cam->height * 3);	// 设置缓冲区

    //////////////////////////////////////////////

    CC_audioDataContent	dataContentAudio;
    CC_MsgHeader 		msgHeaderAudio;

    msgHeaderAudio.messageHeader[0] = 'C';
    msgHeaderAudio.messageHeader[1] = 'C';
    msgHeaderAudio.messageHeader[2] = 'T';
    msgHeaderAudio.messageHeader[3] = 'D';
    msgHeaderAudio.controlMask = CONTROLLCODE_LISTENTRANSPORTREPLY;

    int retValue;
    snd_pcm_t *captureHandle;
    snd_pcm_hw_params_t *params;
    int dir;
    snd_pcm_uframes_t frames=1024;
    unsigned int sampleRate = 8000;

    /* 打开 PCM capture 捕获设备 */
    retValue = snd_pcm_open(&captureHandle, "default",SND_PCM_STREAM_CAPTURE, 0);
    if (retValue < 0) {
        printf("unable to open pcm device: %s\n",snd_strerror(retValue));
        return;
    }

    /* 分配一个硬件参数结构体 */
    snd_pcm_hw_params_alloca(&params);

    /* 使用默认参数 */
    snd_pcm_hw_params_any(captureHandle, params);
    snd_pcm_hw_params_set_access(captureHandle, params,SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(captureHandle, params,SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(captureHandle, params, 1);
    snd_pcm_hw_params_set_rate_near(captureHandle, params,&sampleRate, &dir);
    snd_pcm_hw_params_set_period_size_near(captureHandle,params, &frames, &dir);// 一个周期有 32 帧

    retValue = snd_pcm_hw_params(captureHandle, params);// 参数生效
    if (retValue < 0) {
        printf("unable to set hw parameters: %s\n",snd_strerror(retValue));
        return;
    }
    snd_pcm_hw_params_get_period_size(params,&frames, &dir);// 得到一个周期的数据大小

   int buffSize = frames * 2; // 2 bytes/sample, 2 channels   // 16位 双通道，所以要 *4
    char *readBuffer = (char *) malloc(buffSize);

   while (m_video_status) {

       int readFrame=read_and_encode_frame(cam);

       msgHeaderVideo.commandLength = sizeof(CC_videoDataContent) + cam->encodedLength;
       dataContentVideo.timeStamp = 120015;
       dataContentVideo.pictureLength = cam->encodedLength;


       if(!block_socket_send(socket, (char*)&msgHeaderVideo, sizeof(msgHeaderVideo)))
       {
           printf("send msgHeader error\n");

           if(socket != -1)
           {
               close(socket);
               socket = -1;
           }

           break;
       }

       if(!block_socket_send(socket, (char*)&dataContentVideo, sizeof(dataContentVideo)))
       {
           printf("send msg dataContent error\n");
           if(socket != -1)
           {
               close(socket);
               socket = -1;
           }

           break;
       }


       if (readFrame < 0)
       {
           fprintf(stderr, "read_fram fail in thread\n");

           break;
       }
       else
       {

           if(!block_socket_send(socket,(char*)cam->h264_buf, cam->encodedLength))
           {
               printf("send video data error\n");
               if(socket != -1)
               {
                   close(socket);
                   socket = -1;
               }

               break;
           }
       }


       ///////////////////////////////////////////////////////////////////////////////
       /////////////////////////////////////////////////////////////////////////////
       // 捕获数据
       retValue = snd_pcm_readi(captureHandle, readBuffer, frames);
       if (retValue == -EPIPE) {
           printf("overrun occurred\n");  // EPIPE means overrun
           snd_pcm_prepare(captureHandle);
       } else if (retValue < 0) {
           printf("error from read: %s\n",snd_strerror(retValue));
       } else if (retValue != (int)frames) {
           printf("short read, read %d frames\n", retValue);
       }

       printf("SEND AUDIOxxxxxxxx %d\n",retValue);
       msgHeaderAudio.commandLength = sizeof(CC_audioDataContent) + buffSize;
       dataContentAudio.timeStamp = 1025400041;
       dataContentAudio.packageNumber = 0;
       dataContentAudio.dataLength = buffSize;
       dataContentAudio.audioFormat = 0;

       if((socket > 0) && (buffSize > 0))
       {

           if(block_socket_send(socket, (char *)&msgHeaderAudio, sizeof(msgHeaderAudio)) <= 0 )
           {
               printf("-------------------send msgHeader error\n");
               if(socket != -1)
               {
                   close(socket);
                   socket = -1;
               }

               break;
           }
           if(block_socket_send(socket, (char *)&dataContentAudio, sizeof(dataContentAudio)) <= 0 )
           {
               printf("-------------------send dataContent error\n");
               if(socket != -1)
               {
                   close(socket);
                   socket = -1;
               }

               break;
           }

           printf("read audio frame size = %d---------------------------\n",  buffSize);
           if(block_socket_send(socket, readBuffer, buffSize) <= 0 )
           {
               printf("-------------------send audio data error\n");
               if(socket != -1)
               {
                   close(socket);
                   socket = -1;
               }

               break;
           }

       }
       else
       {

       }


   }

   ////////////////////////////////////////////////
   h264_encoder_uninit(&cam->encoder);

   if(cam->h264_buf!=NULL){
       free(cam->h264_buf);
   }

   camera_capturing_stop(cam);
   camera_uninit(cam);
   camera_close(cam);
   free(cam);

    //////////////////////////////////////////////////////

   snd_pcm_drain(captureHandle);
   snd_pcm_close(captureHandle);
   free(readBuffer);

   /////////////////////////////////////////////////////

   if(socket != -1)
   {
       close(socket);
       socket = -1;
   }


   printf("\nSend VIDEO CLOSE:::::::::\n");
   return ;
}


