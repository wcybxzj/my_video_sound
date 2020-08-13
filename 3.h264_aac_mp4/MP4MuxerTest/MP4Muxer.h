/*
 * Car eye 车辆管理平台: www.car-eye.cn
 * Car eye 开源网址: https://github.com/Car-eye-team
 * MP4Muxer.h
 *
 * Author: Wgj
 * Date: 2019-03-04 22:21
 * Copyright 2019
 *
 * MP4混流器类声明文件
 */

#ifndef __MP4_MUXER_H__
#define __MP4_MUXER_H__

#define _TIMESPEC_DEFINED
#include <thread>
#include <string>
#include <mutex>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

#ifdef __cplusplus
}
#endif

// 用于存储流数据的队列项结构
typedef struct _CE_QUEUE_ITEM_T_
{
	// 申请的缓冲区指针
	uint8_t* Bytes;
	// 申请的缓冲区大小
	int TotalSize;
	// 入队填充的数据字节数
	int EnqueueSize;
	// 出队读取的数据字节数
	int DequeueSize;
}CEQueueItem;

// 一个简单的队列结构, 用于暂存流数据
typedef struct _CE_QUEUE_T_
{
	// 队列项指针
	CEQueueItem *Items;
	// 队列的标识, 使用者可以自定义标识进行队列区分
	uint8_t Flag;
	// 队列项个数
	int Count;
	// 出队计数索引
	int DequeueIndex;
	// 入队索引
	int EnqueueIndex;
}CESimpleQueue;

/*!
 * \class MP4Muxer
 *
 * \brief MP4混流器, 合成H264与AAC音视频
 *
 * \author Wgj
 * \date 2019-02-24
 */
class MP4Muxer
{
public:
	MP4Muxer();
	~MP4Muxer();

	/*
	* Comments: 混流器初始化 实例化后首先调用该方法进行内部参数的初始化
	* Param aFileName: 要保存的文件名
	* @Return 是否成功
	*/
	bool MP4Muxer::Start(std::string aFileName);

	/*
	* Comments: 追加一帧AAC数据到混流器
	* Param aBytes: 要追加的字节数据
	* Param aSize: 追加的字节数
	* @Return 成功与否
	*/
	bool MP4Muxer::AppendAudio(uint8_t* aBytes, int aSize);

	/*
	* Comments: 追加一帧H264视频数据到混流器
	* Param aBytes: 要追加的字节数据
	* Param aSize: 追加的字节数
	* @Return 成功与否
	*/
	bool MP4Muxer::AppendVideo(uint8_t* aBytes, int aSize);

	/*
	* Comments: 关闭并释放输出格式上下文
	* Param : None
	* @Return None
	*/
	void MP4Muxer::Stop(void);

private:
	/*
	* Comments: 打开AAC输入上下文
	* Param : None
	* @Return 0成功, 其他失败
	*/
	int MP4Muxer::OpenAudio(bool aNew = false);
	/*
	* Comments: 写入AAC数据帧到文件
	* Param : None
	* @Return void
	*/
	void MP4Muxer::WriteAudioFrame(void);
	/*
	* Comments: 打开H264视频流
	* Param : None
	* @Return 0成功, 其他失败
	*/
	int MP4Muxer::OpenVideo(void);
	/*
	* Comments: 写入H264视频帧到文件
	* Param : None
	* @Return void
	*/
	void MP4Muxer::WriteVideoFrame(void);

private:
	// 要保存的MP4文件路径
	std::string mFileName;
	// 输入AAC音频上下文
	AVFormatContext* mInputAudioContext;
	// 输入H264视频上下文
	AVFormatContext* mInputVideoContext;
	// 输出媒体格式上下文
	AVFormatContext* mOutputFrmtContext;
	// AAC音频位流筛选上下文
	AVBitStreamFilterContext* mAudioFilter;
	// H264视频位流筛选上下文
	AVBitStreamFilterContext* mVideoFilter;
	// 流媒体数据包
	AVPacket mPacket;
	// AAC数据缓存区
	uint8_t* mAudioBuffer;
	// AAC数据读写队列
	CESimpleQueue* mAudioQueue;
	// H264数据缓冲区
	uint8_t* mVideoBuffer;
	// H264数据读写队列
	CESimpleQueue* mVideoQueue;
	// AAC的回调读取是否启动成功
	bool mIsStartAudio;
	// H264的回调读取是否启动成功
	bool mIsStartVideo;
	// 音频在mInputAudioContext->streams流中的索引
	int mAudioIndex;
	// 视频在mInputVideoContext->streams流中的索引
	int mVideoIndex;
	// 输出流中音频帧的索引
	int mAudioOutIndex;
	// 输出流视频帧的索引
	int mVideoOutIndex;
	// 输出帧索引计数
	int mFrameIndex;

	// 线程操作互斥锁
	std::mutex mLock;

};

#endif

