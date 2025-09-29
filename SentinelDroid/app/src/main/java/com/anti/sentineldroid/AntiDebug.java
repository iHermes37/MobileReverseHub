package com.anti.sentineldroid;

public class AntiDebug {

    static {
        System.loadLibrary("AntiDebug");
    }
    public static native void setAntiDebugCallback(IDetectorCallback callback);
}
