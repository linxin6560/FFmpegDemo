package com.levylin.ffmpegdemo;

import android.content.Context;
import android.graphics.Matrix;
import android.graphics.SurfaceTexture;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.TextureView;

/**
 * 播放器视图
 * Created by LinXin on 2017/11/3.
 */
public class PlayerView extends TextureView implements TextureView.SurfaceTextureListener {


    public PlayerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setSurfaceTextureListener(this);
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
        LogUtils.e("onSurfaceTextureAvailable:width=" + width + ",height=" + height);
        setSurface(new Surface(surfaceTexture), width, height);
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int width, int height) {

    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        return false;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {

    }

    /**
     * ndk调用这个方法设置视频的宽高
     *
     * @param videoWidth
     * @param videoHeight
     */
    public void onNativeGetVideoSize(int videoWidth, int videoHeight) {
        LogUtils.e("onNativeGetVideoSize:videoWidth=" + videoWidth + ",videoHeight=" + videoHeight);
        int width = getWidth();
        int height = getHeight();
        float scaleX = videoWidth * 1.0f / width;
        float scaleY = videoHeight * 1.0f / height;
        LogUtils.e("111scaleX=" + scaleX + ",scaleY=" + scaleY);
        float maxScale = Math.max(scaleX, scaleY);//要保证宽度或者高度全屏
        scaleX /= maxScale;
        scaleY /= maxScale;
        LogUtils.e("222scaleX=" + scaleX + ",scaleY=" + scaleY);
        Matrix matrix = new Matrix();
        matrix.setScale(scaleX, scaleY, width / 2, height / 2);
        setTransform(matrix);
    }

    private native void setSurface(Surface surface, int width, int height);

    public native void play(String path);
}
