//package com.android.retoolkit;
//
//
//import android.app.AndroidAppHelper;
//import android.content.Context;
//import android.os.Build;
//import android.util.Log;
//
//import java.io.BufferedReader;
//import java.io.File;
//import java.io.FileReader;
//import java.io.IOException;
//
//import de.robv.android.xposed.IXposedHookLoadPackage;
//import de.robv.android.xposed.XC_MethodHook;
//import de.robv.android.xposed.XposedBridge;
//import de.robv.android.xposed.XposedHelpers;
//import de.robv.android.xposed.callbacks.XC_LoadPackage;
//
//public class test implements IXposedHookLoadPackage {
//
//    @Override
//    public void handleLoadPackage(final XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {
//        if (!lpparam.packageName.equals("com.mfw.roadbook")) return;
//
//        XposedBridge.log("开始 hook nativeLoad");
//
////        XposedHelpers.findAndHookMethod(
////                "java.lang.Runtime",
////                lpparam.classLoader,
////                "nativeLoad",
////                String.class,
////                ClassLoader.class,
////                new XC_MethodHook() {
////                    @Override
////                    protected void afterHookedMethod(MethodHookParam param) throws Throwable {
////                        String soPath = (String) param.args[0];
////                        XposedBridge.log("[1] 进入 Hook, soPath: " + soPath); // 确认是否执行
////
////                        ClassLoader targetCL = (ClassLoader) param.args[1];
////                        Context context = AndroidAppHelper.currentApplication();
////                        String libPath3 = context.getApplicationInfo().nativeLibraryDir + "/libretoolkit.so";
////
////                        XposedBridge.log("[2] 修改后的路径: " + libPath3);
////                        XposedBridge.log("[3] ClassLoader: " + targetCL);
////
////
////                        // 检查文件是否存在
////                        if (new File(libPath3).exists()) {
////                            XposedBridge.log("[4] libretoolkit.so 存在");
////                        } else {
////                            XposedBridge.log("[4] libretoolkit.so 不存在");
////                        }
////
////                        // 调用原始方法（谨慎操作）
////                        try {
//////                            XposedBridge.invokeOriginalMethod(param.method, param.thisObject, new Object[]{
//////                                    libPath3, targetCL
//////                            });
////
////                            if(injectSo(targetCL,libPath3)){
////                              XposedBridge.log("[5---5] so 加载成功，准备执行 native hook");
////                           }
////
////                            XposedBridge.log("[5] 原始方法调用完成");
////                        } catch (Throwable e) {
////                            XposedBridge.log("[5] 原始方法调用失败: " + e);
////                        }
////                    }
////                }
////        );
//
//
//        XposedHelpers.findAndHookMethod("android.app.Application", lpparam.classLoader, "attach",
//                Context.class, new XC_MethodHook() {
//                    @Override
//                    protected void afterHookedMethod(MethodHookParam param) throws Throwable {
//                        Context context = (Context) param.args[0];
//                        ClassLoader appCL = context.getClassLoader();
//
//                        XposedBridge.log("当前ClassLoader: " + appCL.toString());
//
//                        try {
//                            String libPath = context.getApplicationInfo().nativeLibraryDir + "/libretoolkit.so";
//                            XposedBridge.log("当前 App 的 so 路径：" + libPath);
//                            if (injectSo(appCL, libPath)) {
//                                XposedBridge.log("[ReToolkit] so 加载成功，准备执行 native hook");
//                            }
//                        } catch (Throwable t) {
//                            XposedBridge.log("[ReToolkit] so 加载失败: " + t.getMessage());
//                        }
//                    }
//                });
//
//
//    }
//
//
//    public boolean injectSo(Object classLoader, String soPath) {
//        try {
//            // ① 检查路径合法性
//            if (soPath == null || soPath.isEmpty()) {
//                XposedBridge.log("[injectSo] Error: soPath is null or empty.");
//                return false;
//            }
//
//            // ② 检查 ClassLoader 是否为空
//            if (classLoader == null) {
//                XposedBridge.log("[injectSo] Error: ClassLoader is null.");
//                return false;
//            }
//
//            // ③ 检查是否已经加载
//            if (isLibraryLoaded(soPath)) {
//                XposedBridge.log("[injectSo] Already loaded: " + soPath);
//                return true;
//            }
//
//            // ④ 检查 so 文件是否存在
//            File soFile = new File(soPath);
//            if (!soFile.exists()) {
//                XposedBridge.log("[injectSo] Error: SO file not found: " + soPath);
//                return false;
//            }
//
//            // ⑤ 执行注入
//            String result = null;
//            int version = Build.VERSION.SDK_INT;
//            XposedBridge.log("[injectSo] Injecting SO on Android " + version + " with loader: " + classLoader);
//
//            if (version >= Build.VERSION_CODES.P) {
//                result = (String) XposedHelpers.callMethod(
//                        Runtime.getRuntime(), "nativeLoad", soPath, classLoader
//                );
//            } else {
//                result = (String) XposedHelpers.callMethod(
//                        Runtime.getRuntime(), "doLoad", soPath, classLoader
//                );
//            }
//
//            // ⑥ 判断是否注入成功
//            if (result != null) {
//                XposedBridge.log("[injectSo] nativeLoad/doLoad failed: " + result);
//                throw new UnsatisfiedLinkError(result);
//            }
//
//            // ⑦ 检查是否真的成功（可选，需在 native 层 JNI_OnLoad 打印日志配合验证）
//            XposedBridge.log("[injectSo] Successfully injected: " + soPath);
//            return true;
//
//        } catch (UnsatisfiedLinkError e) {
//            XposedBridge.log("[injectSo] UnsatisfiedLinkError: " + Log.getStackTraceString(e));
//        } catch (Exception e) {
//            XposedBridge.log("[injectSo] Exception: " + Log.getStackTraceString(e));
//        }
//
//        return false;
//    }
//
//
//
//    // 检查SO是否已加载
//    public boolean isLibraryLoaded(String soPath) {
//        try {
//            BufferedReader reader = new BufferedReader(new FileReader("/proc/self/maps"));
//            String line;
//            while ((line = reader.readLine()) != null) {
//                if (line.contains(soPath)) {
//                    return true;
//                }
//            }
//            reader.close();
//        } catch (IOException e) {
//            XposedBridge.log("Error reading /proc/self/maps: " + Log.getStackTraceString(e));
//        }
//        return false;
//    }
//
//
//
//}
