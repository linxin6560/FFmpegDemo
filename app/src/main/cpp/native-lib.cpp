#include <jni.h>
#include <string>
#include "FFmpegVideo.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>

pthread_t main_tid;
int isPlaying;
ANativeWindow *window;
const char *path;
FFmpegVideo *video;

jobject jobj;
JavaVM *jvm;

void call_video_play(AVFrame *frame) {
    if (!window) {
        LOGE("window is null");
        return;
    }
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(window, &buffer, NULL) < 0) {
        LOGE("window 锁住失败");
        return;
    }
    uint8_t *dst = (uint8_t *) buffer.bits;
    int dstStride = buffer.stride * 4;
    uint8_t *src = frame->data[0];
    int srcStride = frame->linesize[0];
    for (int i = 0; i < video->avCodecContext->height; ++i) {
        memcpy(dst + i * dstStride, src + i * srcStride, (size_t) srcStride);
    }
    ANativeWindow_unlockAndPost(window);
}

void *proccess(void *data) {
    av_register_all();//使用ffmpeg必须要注册
    avformat_network_init();//如果播放网络视频，需要注册

    AVFormatContext *formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, path, NULL, NULL) < 0) {
        LOGE("打开视频失败");
    }
    LOGE("打开视频成功");
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGE("寻找流信息失败");
    }
    LOGE("寻找流信息成功");
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        AVStream *stream = formatContext->streams[i];
        AVCodecContext *codecContext = stream->codec;
        //获取解码器
        AVCodec *codec = avcodec_find_decoder(codecContext->codec_id);
        if (avcodec_open2(codecContext, codec, NULL) < 0) {
            LOGE("打开解码器失败");
            continue;
        }
        if (codecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
            video->index = i;
            video->setAVCodecPacket(codecContext);
            int width = codecContext->width;
            int height = codecContext->height;
            LOGE("视频:宽=%d,高宽=%d", width, height);
            JNIEnv *env;
            jvm->AttachCurrentThread(&env, 0);
            LOGE("获取env");
            jclass clazz = env->GetObjectClass(jobj);
            LOGE("Native found Java jobj Class :%d", clazz ? 1 : 0);
            jmethodID mid = env->GetMethodID(clazz, "onNativeGetVideoSize", "(II)V");
            if (env && jobj && mid) {
                LOGE("给JAVA中设置宽高");
                env->CallVoidMethod(jobj, mid, width, height);
            }
            ANativeWindow_setBuffersGeometry(window, width, height,
                                             WINDOW_FORMAT_RGBA_8888);
        }
    }
    LOGE("开始播放");
    video->play();
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));
    while (isPlaying) {
        if (av_read_frame(formatContext, packet) < 0) {
            LOGE("读取帧失败");
            av_packet_unref(packet);
            continue;
        }
        if (video && video->isPlay && video->index == packet->stream_index) {
            video->put(packet);
        }
        av_packet_unref(packet);
    }
    isPlaying = 0;
    if (video && video->isPlay) {
        video->stop();
    }
    av_free_packet(packet);
    avformat_free_context(formatContext);
    pthread_exit(0);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jvm = vm;
    JNIEnv *env = NULL;
    jint result = -1;
    if (jvm) {
        LOGE("jvm init success");
    }
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return result;
    }
    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_levylin_ffmpegdemo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_levylin_ffmpegdemo_PlayerView_setSurface(JNIEnv *env, jobject instance, jobject surface,
                                                  jint width, jint height) {
    if (!window) {
        window = ANativeWindow_fromSurface(env, surface);
    }
    if (!jobj) {
        jobj = env->NewGlobalRef(instance);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_levylin_ffmpegdemo_PlayerView_play(JNIEnv *env, jobject instance, jstring path_) {
    path = env->GetStringUTFChars(path_, 0);
    video = new FFmpegVideo;
    video->setPlayCall(call_video_play);
    isPlaying = 1;
    pthread_create(&main_tid, NULL, proccess, NULL);
    env->ReleaseStringUTFChars(path_, path);
}