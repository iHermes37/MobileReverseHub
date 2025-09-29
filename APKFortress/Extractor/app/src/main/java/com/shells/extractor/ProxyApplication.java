package com.shells.extractor;

import android.app.Application;


public class ProxyApplication extends Application {

    static {
        System.loadLibrary("extractor");
    }

    private native void initHook();

    @Override
    public void onCreate() {
        super.onCreate();
        // 初始化 ByteHook
        ByteHook.init();
        // 加载包含 Hook 逻辑的 native 库
        System.loadLibrary("extractor");

        initHook();  // 初始化 native hook
    }
}
