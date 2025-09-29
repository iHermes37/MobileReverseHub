package com.android.retoolkit;


import com.bytedance.android.bytehook.ByteHook;
import com.bytedance.shadowhook.ShadowHook;


import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class AntiAntiDebug implements IXposedHookLoadPackage {
    static String TAG = "anti_anti_debug";

    public static synchronized void init() {
        ByteHook.init();
        ShadowHook.init(new ShadowHook.ConfigBuilder()
                .setMode(ShadowHook.Mode.UNIQUE)
                .build());
    }



    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable{
        anti_isDebuggerConnected(lpparam.classLoader);
    }



    //-----------具体方法---------------------
    private void anti_isDebuggerConnected(ClassLoader classLoader) {

        XposedBridge.log("[ReToolkit] SSL Hook 启动日志 anti_isDebuggerConnected");

        XposedHelpers.findAndHookMethod("android.os.Debug", classLoader, "isDebuggerConnected",
                new XC_MethodHook() {
                    @Override
                    protected void afterHookedMethod(MethodHookParam param){
                        Object res=param.getResult();
                        param.setResult(false);
                    }

                });
    }

    //------------native hook 关键函数-------------------------------
    static{
        System.loadLibrary("anti2dbg");
    }

    private  native  void anti2dbg();




}
