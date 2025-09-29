package com.android.retoolkit;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

// 主入口类
public class MainModule implements IXposedHookLoadPackage {
    private final IXposedHookLoadPackage[] modules = {
             new SslUnpinModule(),
//            new AntiAntiDebug(),
//            new MonitorService()
            // 添加其他模块...
    };

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) {

//        XposedBridge.log("[ReToolkit] SSL Hook 启动日志 5555555555555555");
        for (IXposedHookLoadPackage module : modules) {
            try {
                module.handleLoadPackage(lpparam);
            } catch (Throwable t) {
                XposedBridge.log("Module failed: " + module.getClass().getSimpleName());
                XposedBridge.log(t);
            }
        }
    }
}
