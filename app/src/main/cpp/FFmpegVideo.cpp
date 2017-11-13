//
// Created by Administrator on 2017/11/7.
//

#include "FFmpegVideo.h"

/**
 * ANativeWindow绘制的方法回调
 * @param frame
 */
static void (*video_call)(AVFrame *frame);

/**
 * 视频播放线程
 * @param data
 * @return
 */
void *playVideo(void *data) {
    LOGE("播放视频线程");
    FFmpegVideo *video = (FFmpegVideo *) data;
    AVCodecContext *pContext = video->avCodecContext;
    //像素格式
    AVPixelFormat pixelFormat = AV_PIX_FMT_RGBA;
    SwsContext *swsContext = sws_getContext(pContext->width,
                                            pContext->height,
                                            pContext->pix_fmt,
                                            pContext->width,
                                            pContext->height,
                                            pixelFormat,
                                            SWS_BICUBIC,
                                            NULL,
                                            NULL,
                                            NULL);
    LOGE("获取swsContext完成");
    //要画在window上的frame
    AVFrame *rgb_frame = av_frame_alloc();
    uint8_t *out_buffer = (uint8_t *) av_malloc(
            (size_t) avpicture_get_size(pixelFormat, pContext->width, pContext->height));
    avpicture_fill((AVPicture *) rgb_frame, out_buffer, pixelFormat, pContext->width,
                   pContext->height);
    LOGE("设置rgb_frame完成");
    int got_frame;
    AVFrame *frame = av_frame_alloc();
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    av_init_packet(packet);
    while (video->isPlay) {
        video->get(packet);
        avcodec_decode_video2(pContext, frame, &got_frame, packet);
        if (!got_frame) {
            continue;
        }
        sws_scale(swsContext, (const uint8_t *const *) frame->data, frame->linesize, 0,
                  frame->height, rgb_frame->data, rgb_frame->linesize);
        video_call(rgb_frame);
        usleep(16 * 1000);//这边先暂定时间是16毫秒
    }
}

FFmpegVideo::FFmpegVideo() {
    pthread_mutex_init(&mutex, NULL);//初始化互斥锁
    pthread_cond_init(&cond, NULL);//初始化条件
}

FFmpegVideo::~FFmpegVideo() {

}

int FFmpegVideo::get(AVPacket *packet) {
    LOGE("获取视频包");
    pthread_mutex_lock(&mutex);
    if (isPlay) {
        if (video_queue.empty()) {
            LOGE("列表为空");
            pthread_cond_wait(&cond, &mutex);
        } else {
            AVPacket *packet1 = video_queue.front();
            video_queue.pop();
            if (av_packet_ref(packet, packet1) < 0) {
                LOGE("获取包.....克隆失败");
                return 0;
            }
            av_free_packet(packet1);
        }
    }
    pthread_mutex_unlock(&mutex);
    return 1;
}

int FFmpegVideo::put(AVPacket *packet) {
    LOGE("插入视频包");
    AVPacket *packet1 = (AVPacket *) malloc(sizeof(AVPacket));
    if (av_copy_packet(packet1, packet) < 0) {
        LOGE("克隆失败");
        return 0;
    }
    pthread_mutex_lock(&mutex);
    video_queue.push(packet1);
    av_free_packet(packet);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    return 1;
}

void FFmpegVideo::play() {
    isPlay = 1;
    pthread_create(&tid, NULL, playVideo, this);
}

void FFmpegVideo::stop() {
    isPlay = 0;
}

void FFmpegVideo::setAVCodecPacket(AVCodecContext *avCodecContext) {
    this->avCodecContext = avCodecContext;
}

void FFmpegVideo::setPlayCall(void (*call)(AVFrame *)) {
    video_call = call;
}
