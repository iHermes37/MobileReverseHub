package com.android.retoolkit;

import android.app.AndroidAppHelper;
import android.content.Context;
import android.content.res.AssetManager;
import android.os.Build;
import android.util.Log;
import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.Arrays;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class MonitorService implements IXposedHookLoadPackage {

    private static Context moduleContext = null;
    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {
        if (!lpparam.packageName.equals("com.mfw.roadbook")) {
            return;
        }

        try {
            // 1. 安全获取目标应用的 Context
            Context appContext = null;
            try {
                Class<?> activityThreadClass = XposedHelpers.findClass("android.app.ActivityThread", null);
                Object activityThread = XposedHelpers.callStaticMethod(activityThreadClass, "currentActivityThread");
                appContext = (Context) XposedHelpers.callMethod(activityThread, "getSystemContext");
            } catch (Throwable e) {
                XposedBridge.log("获取系统 Context 失败: " + e);
            }

            if (appContext == null) {
                XposedBridge.log("警告：使用 AndroidAppHelper 回退方案");
                appContext = AndroidAppHelper.currentApplication();
            }

            if (appContext == null) {
                XposedBridge.log("错误：无法获取任何 Context");
                return;
            }

            // 2. 初始化模块 Context
            if (moduleContext == null) {
                moduleContext = appContext.createPackageContext(
                        "com.android.retoolkit",
                        Context.CONTEXT_IGNORE_SECURITY
                );
                if (moduleContext == null) {
                    XposedBridge.log("错误：模块 Context 创建失败");
                    return;
                }
                XposedBridge.log("模块 Context 初始化成功");
            }

            // 3. 处理 .so 文件
            AssetManager assets = moduleContext.getAssets();
            String soAssetPath = "libs/libretoolkit.so";

//            XposedBridge.log("路径是"+assets.open(soAssetPath));

            if (runRootCommand("setenforce 0")) {
                XposedBridge.log("已临时禁用 SELinux");
            }


            File localSoFile = new File(moduleContext.getFilesDir(), "libretoolkit.so");
            runRootCommand("chmod 777 /data/user/0/com.android.retoolkit/files/libretoolkit.so");

            XposedBridge.log("路径是"+localSoFile.getAbsolutePath());

            try (InputStream is = assets.open(soAssetPath);
                 FileOutputStream fos = new FileOutputStream(localSoFile)) {
                XposedBridge.log("文件 libretoolkit.so 可正常打开，大小: " + is.available() + " 字节");
                byte[] buffer = new byte[1024];
                int length;
                while ((length = is.read(buffer)) > 0) {
                    fos.write(buffer, 0, length);
                }
                XposedBridge.log(".so 文件复制成功: " + localSoFile.getAbsolutePath());
            }

            XposedBridge.log("hello");


            // 4. 设置权限并加载
            // 2. 通过 root 命令移动到系统目录

            if (runRootCommand("cp " + localSoFile.getAbsolutePath() + " /data/local/tmp/") &&
                    runRootCommand("chmod 777 /data/local/tmp/libretoolkit.so")) {
                XposedBridge.log("成功写入系统目录");
            }

            hookActivityThread(lpparam, true);

        } catch (Throwable e) {
            XposedBridge.log("模块崩溃: " + e);
            e.printStackTrace();
        }
    }


    public static boolean runRootCommand(String command) {
        try {
            Process process = Runtime.getRuntime().exec("su");
            DataOutputStream os = new DataOutputStream(process.getOutputStream());

            // 使用 && 和 || 判断命令是否成功
            os.writeBytes(command + " && echo SUCCESS || echo FAILED\n");
            os.writeBytes("exit\n");
            os.flush();

            // 读取输出
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.contains("SUCCESS")) {
                    return true;
                } else if (line.contains("FAILED")) {
                    return false;
                }
            }

            // 如果没有匹配到 SUCCESS/FAILED，检查进程退出码
            process.waitFor();
            return process.exitValue() == 0;
        } catch (Exception e) {
            XposedBridge.log("Root 命令异常: " + e.getMessage());
            return false;
        }
    }


    //注入so
    private void hookActivityThread(XC_LoadPackage.LoadPackageParam lpparam,boolean issystemlib) throws ClassNotFoundException {

        //判断是否是系统so
        if(issystemlib){
            XposedHelpers.findAndHookMethod("android.app.Application", lpparam.classLoader, "attach",
                    Context.class, new XC_MethodHook() {
                        @Override
                        protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                            Context context = (Context) param.args[0];
                            injectSoToTarget(lpparam, context, true);
                        }
                    });
        }else{
            XposedHelpers.findAndHookMethod(
                    "java.lang.Runtime",
                    lpparam.classLoader,
                    "nativeLoad",
                    String.class,
                    ClassLoader.class,
                    new XC_MethodHook() {
                        @Override
                        protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                            Context context = AndroidAppHelper.currentApplication();
                            injectSoToTarget(lpparam, context, true);
                        }
                    }
            );
        }

    }

    // 修改返回值为boolean
    public boolean injectSo(Object classLoader, String soPath) {
        try {
            // ① 检查路径合法性
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD) {
                if (soPath == null || soPath.isEmpty()) {
                    XposedBridge.log("[injectSo] Error: soPath is null or empty.");
                    return false;
                }
            }

            // ② 检查 ClassLoader 是否为空
            if (classLoader == null) {
                XposedBridge.log("[injectSo] Error: ClassLoader is null.");
                return false;
            }

            // ③ 检查是否已经加载
            if (isLibraryLoaded(soPath)) {
                XposedBridge.log("[injectSo] Already loaded: " + soPath);
                return true;
            }

            // ④ 检查 so 文件是否存在
            File soFile = new File(soPath);
            if (!soFile.exists()) {
                XposedBridge.log("[injectSo] Error: SO file not found: " + soPath);
                return false;
            }

            // ⑤ 执行注入
            String result = null;
            int version = Build.VERSION.SDK_INT;
            XposedBridge.log("[injectSo] Injecting SO on Android " + version + " with loader: " + classLoader);

            if (version >= Build.VERSION_CODES.P) {
                result = (String) XposedHelpers.callMethod(
                        Runtime.getRuntime(), "nativeLoad", soPath, classLoader
                );
            } else {
                result = (String) XposedHelpers.callMethod(
                        Runtime.getRuntime(), "doLoad", soPath, classLoader
                );
            }

            // ⑥ 判断是否注入成功
            if (result != null) {
                XposedBridge.log("[injectSo] nativeLoad/doLoad failed: " + result);
                throw new UnsatisfiedLinkError(result);
            }

            // ⑦ 检查是否真的成功（可选，需在 native 层 JNI_OnLoad 打印日志配合验证）
            XposedBridge.log("[injectSo] Successfully injected: " + soPath);
            return true;

        } catch (UnsatisfiedLinkError e) {
            XposedBridge.log("[injectSo] UnsatisfiedLinkError: " + Log.getStackTraceString(e));
        } catch (Exception e) {
            XposedBridge.log("[injectSo] Exception: " + Log.getStackTraceString(e));
        }

        return false;
    }

    // 检查SO是否已加载
    public boolean isLibraryLoaded(String soPath) {
        try {
            BufferedReader reader = new BufferedReader(new FileReader("/proc/self/maps"));
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.contains(soPath)) {
                    return true;
                }
            }
            reader.close();
        } catch (IOException e) {
            XposedBridge.log("Error reading /proc/self/maps: " + Log.getStackTraceString(e));
        }
        return false;
    }


//    private  void copyCurtoTarget(){
//
//    }


    private void injectSoToTarget(XC_LoadPackage.LoadPackageParam lpparam, Context context, boolean isSystemLib) {
        String moduleSoName = "libretoolkit.so";
        String privateSoPath = extractSoToPrivateDir(context, moduleSoName);
        if (privateSoPath == null) return;

        String targetLibPath = null;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD) {
            targetLibPath = lpparam.appInfo.nativeLibraryDir;
        }
        String targetSoPath = targetLibPath + "/" + moduleSoName;

        // 使用 su 复制文件
        try {
            Process process = Runtime.getRuntime().exec("su");
            DataOutputStream os = new DataOutputStream(process.getOutputStream());
            os.writeBytes("cp " + privateSoPath + " " + targetSoPath + "\n");
            os.writeBytes("chmod 777 " + targetSoPath + "\n");
            os.writeBytes("exit\n");
            os.flush();
            process.waitFor();
            XposedBridge.log("SO 已复制到: " + targetSoPath);

            // 加载 SO
            if (injectSo(context.getClassLoader(), targetSoPath)) {
                XposedBridge.log("SO 注入成功");
            }
        } catch (Exception e) {
            XposedBridge.log("操作失败: " + e.getMessage());
        }
    }

    private String extractSoToPrivateDir(Context context, String soName) {
        try {
            // 1. 检查输入参数有效性
            if (context == null) {
                XposedBridge.log("[ERROR] Context is null!");
                return null;
            }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD) {
                if (soName == null || soName.isEmpty()) {
                    XposedBridge.log("[ERROR] soName is null or empty!");
                    return null;
                }
            }

            // 2. 创建/获取私有目录
            File privateDir = context.getDir("libs", Context.MODE_PRIVATE);
            if (!privateDir.exists() && !privateDir.mkdirs()) {
                XposedBridge.log("[ERROR] Failed to create private directory: " + privateDir.getAbsolutePath());
                return null;
            }

            // 3. 准备目标文件路径
            File destSo = new File(privateDir, soName);
            XposedBridge.log("[DEBUG] Target so path: " + destSo.getAbsolutePath());

            // 4. 检查assets中是否存在该so文件
            try {
                String[] assetsList = context.getAssets().list("libs");
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                    if (assetsList == null || Arrays.stream(assetsList).noneMatch(soName::equals)) {
                        XposedBridge.log("[ERROR] SO file not found in assets/libs/: " + soName);
                        return null;
                    }
                }
            } catch (IOException e) {
                XposedBridge.log("[ERROR] Failed to list assets/libs: " + e.getMessage());
                return null;
            }

            // 5. 执行文件拷贝
            try (InputStream is = context.getAssets().open("libs/" + soName);
                 OutputStream os = new FileOutputStream(destSo)) {

                XposedBridge.log("[DEBUG] Start copying SO file...");
                byte[] buffer = new byte[1024];
                int length;
                long totalBytes = 0;

                while ((length = is.read(buffer)) > 0) {
                    os.write(buffer, 0, length);
                    totalBytes += length;
                }

                XposedBridge.log("[DEBUG] Copied " + totalBytes + " bytes to " + destSo.getAbsolutePath());
            }

            // 6. 设置文件权限
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD) {
                if (!destSo.setExecutable(true)) {
                    XposedBridge.log("[ERROR] Failed to set executable permission for: " + destSo.getAbsolutePath());
                    return null;
                }
            }

            // 7. 验证文件完整性
            if (!destSo.exists() || destSo.length() == 0) {
                XposedBridge.log("[ERROR] Extracted SO file is invalid: " + destSo.getAbsolutePath());
                return null;
            }

            XposedBridge.log("[SUCCESS] SO extracted successfully: " + destSo.getAbsolutePath());
            return destSo.getAbsolutePath();

        } catch (SecurityException e) {
            XposedBridge.log("[ERROR] Security violation: " + e.getMessage());
        } catch (IOException e) {
            XposedBridge.log("[ERROR] IO Exception: " + e.getMessage() +
                    "\nStackTrace: " + Log.getStackTraceString(e));
        } catch (Exception e) {
            XposedBridge.log("[ERROR] Unexpected error: " + e.getMessage() +
                    "\nStackTrace: " + Log.getStackTraceString(e));
        }
        return null;
    }


}



