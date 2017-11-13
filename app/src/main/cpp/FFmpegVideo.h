//
// Created by Administrator on 2017/11/7.
//

#ifndef FFMPEGDEMO_FFMPEGVIDEO_H
#define FFMPEGDEMO_FFMPEGVIDEO_H

#include "my-log.h"
#include <queue>
#include <unistd.h>
#include <pthread.h>

extern "C" {
#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

using namespace std;

class FFmpegVideo {
public:
    FFmpegVideo();

    ~FFmpegVideo();

    /**
     * 从队列中获取一个包
     * @param packet
     * @return
     */
    int get(AVPacket *packet);

    /**
     * 往队列中插入一个包
     * @param packet
     * @return
     */
    int put(AVPacket *packet);

    /**
     * 播放
     */
    void play();

    /**
     * 结束
     */
    void stop();

    /**
     * 设置解码器上下文
     * @param avCodecContext
     */
    void setAVCodecPacket(AVCodecContext *avCodecContext);

    /**
     * 播放回调
     * @param call
     */
    void setPlayCall(void(*call)(AVFrame *frame));

public:
    int isPlay;//是否播放
    int index;//视频流索引
    queue<AVPacket *> video_queue;//包队列
    pthread_t tid;//播放线程id
    AVCodecContext *avCodecContext;//解码器上下文
    pthread_mutex_t mutex;//互斥锁
    pthread_cond_t cond;
};

#endif //FFMPEGDEMO_FFMPEGVIDEO_H
