package com.levylin.ffmpegdemo

import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    val URL = "rtmp://live.hkstv.hk.lxdns.com/live/hks"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        play_btn.setOnClickListener { playVideo() }
    }

    fun playVideo() {
        playerView.play(URL)
    }

    companion object {

        init {
            System.loadLibrary("native-lib")
        }
    }
}
