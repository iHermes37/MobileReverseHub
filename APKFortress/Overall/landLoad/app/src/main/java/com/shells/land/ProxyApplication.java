package com.shells.land;

import android.app.Application;
import android.app.Instrumentation;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.ArrayMap;
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipInputStream;

import dalvik.system.InMemoryDexClassLoader;

public class ProxyApplication extends Application {

//    @Override
//    protected void attachBaseContext(Context base) {
//        super.attachBaseContext(base);
//        Log.i("ProxyApplication", "进入 attachBaseContext");
//
//        try {
//            // 1. 从 assets 读取 APK
//            byte[] srcApkData = readApkFromAssets(base, "src.apk");
//            Log.i("ProxyApplication", "APK 大小: " + srcApkData.length + " bytes");
//
//            // 2. 准备目录（使用 getDir 确保正确路径）
//            File apkDir = base.getDir("apk_out", Context.MODE_PRIVATE);
//            if (!apkDir.exists()) {
//                apkDir.mkdirs();
//            }
//            String srcApkPath = new File(apkDir, "src.apk").getAbsolutePath();
//
//            // 3. 写入文件
//            writeApkFile(srcApkPath, srcApkData);
//            Log.i("ProxyApplication", "APK 写入完成: " + srcApkPath);
//
//            // 4. 加载 APK
//            File optDir = base.getDir("opt_dex", Context.MODE_PRIVATE);
//            File libDir = base.getDir("lib_dex", Context.MODE_PRIVATE);
//
//            DexClassLoader dexClassLoader = new DexClassLoader(
//                    srcApkPath,
//                    optDir.getAbsolutePath(),
//                    libDir.getAbsolutePath(),
//                    getClassLoader()
//            );
//            Log.d("111111111",dexClassLoader.toString());
//            // 5. 替换 ClassLoader
//            replaceClassLoader(base, dexClassLoader);
//            Log.i("ProxyApplication", "ClassLoader 替换完成");
//
//
//        } catch (Exception e) {
//            Log.e("ProxyApplication", "加载失败", e);
//            throw new RuntimeException("APK 加载失败", e);
//        }
//    }
    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        Log.i("ProxyApplication", "进入 attachBaseContext");

        try {
            // 1. 从 assets 读取 APK 字节码
            byte[] apkBytes = readApkFromAssets(base, "src.apk");
            Log.i("ProxyApplication", "APK 大小: " + apkBytes.length + " bytes");

            // 2. 提取 DEX 字节码（从 APK 中解析出 classes.dex）
            byte[] dexBytes = extractDexBytesFromApk(apkBytes);
            if (dexBytes == null) {
                throw new RuntimeException("APK 中未找到 DEX 文件");
            }

            // 3. 使用 InMemoryDexClassLoader 加载（Android 8.0+）
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                // 创建内存加载器
                InMemoryDexClassLoader dexClassLoader = new InMemoryDexClassLoader(
                        ByteBuffer.wrap(dexBytes),  // 直接传入 DEX 字节码
                        getClassLoader()            // 父加载器
                );
                Log.d("ClassLoader", "InMemoryDexClassLoader 创建成功");

                // 4. 替换 ClassLoader
                replaceClassLoader(base, dexClassLoader);
                Log.i("ProxyApplication", "ClassLoader 替换完成（内存模式）");
            }
        } catch (Exception e) {
            Log.e("ProxyApplication", "加载失败", e);
            throw new RuntimeException("APK 加载失败", e);
        }
    }

    // 从 APK 字节码中提取 DEX 文件
    private byte[] extractDexBytesFromApk(byte[] apkBytes) throws IOException {
        try (ZipInputStream zis = new ZipInputStream(new ByteArrayInputStream(apkBytes))) {
            ZipEntry entry;
            while ((entry = zis.getNextEntry()) != null) {
                if (entry.getName().endsWith(".dex")) {
                    ByteArrayOutputStream bos = new ByteArrayOutputStream();
                    byte[] buffer = new byte[1024];
                    int len;
                    while ((len = zis.read(buffer)) > 0) {
                        bos.write(buffer, 0, len);
                    }
                    return bos.toByteArray();
                }
            }
        }
        return null;
    }

    // 从assets读取APK文件
    private byte[] readApkFromAssets(Context context, String assetName) throws IOException {
        try (InputStream is = context.getAssets().open(assetName);
             ByteArrayOutputStream bos = new ByteArrayOutputStream()) {
            byte[] buffer = new byte[4096];
            int len;
            while ((len = is.read(buffer)) != -1) {
                bos.write(buffer, 0, len);
            }
            return bos.toByteArray();
        }
    }

    // 写入APK文件到内部存储
    private void writeApkFile(String filePath, byte[] apkData) throws IOException {
        File apkFile = new File(filePath);
        try (FileOutputStream fos = new FileOutputStream(apkFile)) {
            fos.write(apkData);
        }
        apkFile.setReadOnly(); // 设置为只读防止被修改
    }

    @Override
    public void onCreate(){
        super.onCreate();
        Log.i("oncreate","进入oncreate");
        Refinvoke refinvoke=new Refinvoke();

        String applicationName="";
        ApplicationInfo manifest=null;
        try {
            manifest=getPackageManager().getApplicationInfo(getPackageName(), PackageManager.GET_META_DATA);
            if (manifest.metaData!=null){
                applicationName=manifest.metaData.getString("APPLICATION_CLASS_NAME");
                Log.d("oncrtead",applicationName);
            }
        } catch (PackageManager.NameNotFoundException e) {
            Log.e("oncreate","获取applicationName失败");
        }

        Object activityThreadIns= Refinvoke.invokeStaticMethood("android.app.ActivityThread","currentActivityThread",new Class[]{},new Object[]{});
        //mBoundApplication 是 ActivityThread 的成员变量，类型为 ActivityThread.AppBindData，存储了应用绑定的信息（如 Application 对象、LoadedApk 等）。
        Object mBoundApplication=refinvoke.getInstanceField("android.app.ActivityThread","mBoundApplication",activityThreadIns);
        Log.d("test","mInitialApplication is:"+mBoundApplication.toString());
        //LoadedApk 是 Android 内部类，表示一个已加载的 APK 的运行时信息（如资源、类加载器、Application 实例等）
        //info 是 AppBindData 的成员变量，类型为 LoadedApk。
        Object info=refinvoke.getInstanceField("android.app.ActivityThread$AppBindData","info",mBoundApplication);//？？？
        Log.d("test","info is:"+info.toString());
        refinvoke.setInstanceField("android.app.LoadedApk","mApplication",info,null);


        Object mInitApplication=refinvoke.getInstanceField("android.app.ActivityThread","mInitialApplication",activityThreadIns);
        ArrayList<Application> mAllApplications= (ArrayList<Application>) refinvoke.getInstanceField("android.app.ActivityThread","mAllApplications",activityThreadIns);
        mAllApplications.remove(mInitApplication);


        ApplicationInfo mApplicationInfo= (ApplicationInfo) refinvoke.getInstanceField("android.app.LoadedApk","mApplicationInfo",info);
        ApplicationInfo appinfo= (ApplicationInfo) refinvoke.getInstanceField("android.app.ActivityThread$AppBindData","appInfo",mBoundApplication);
        mApplicationInfo.className=applicationName;
        appinfo.className=applicationName;
        Log.d("Test","name is: "+mApplicationInfo+appinfo);

        Application app= (Application) refinvoke.invokeInstanceMethod("android.app.LoadedApk","makeApplication",info,new Class[]{boolean.class, Instrumentation.class},new Object[]{false,null});
        refinvoke.setInstanceField("android.app.ActivityThread","mInitialApplication",activityThreadIns,app);



        ArrayMap mProviderMap= (ArrayMap) refinvoke.getInstanceField("android.app.ActivityThread","mProviderMap",activityThreadIns);
        Iterator iterator=mProviderMap.values().iterator();
        while (iterator.hasNext()){
            Object mProviderClientRecord=iterator.next();
            Object mLocalProvider=refinvoke.getInstanceField("android.app.ActivityThread$ProviderClientRecord","mLocalProvider",mProviderClientRecord);
            refinvoke.setInstanceField("android.content.ContentProvider","mContext",mLocalProvider,app);
        }

        app.onCreate();
    }
    public static void replaceClassLoader(Context base, InMemoryDexClassLoader sourceClassLoader) {
        ClassLoader curClassLoader=ProxyApplication.class.getClassLoader();
        try{
            Class<?> activityThread = curClassLoader.loadClass("android.app.ActivityThread");
            Method currentActivityThread=activityThread.getMethod("currentActivityThread");
            Object activityThreadIns=currentActivityThread.invoke(null);

            Field mPackages=activityThread.getDeclaredField("mPackages");
            mPackages.setAccessible(true);
            ArrayMap<String, WeakReference<?>> mPackageIns=(ArrayMap<String, WeakReference<?>>)mPackages.get(activityThreadIns);

            String packageName=base.getPackageName();
            WeakReference<?> ref=(WeakReference<?>)mPackageIns.get(packageName);
            Object LoadedApkInstance = ref.get();
            //Log.i("Test","成功找到LoadedApkInstance"+LoadedApkInstance);

            Class<?> LoadedApk=curClassLoader.loadClass("android.app.LoadedApk");
            Field mClassLoaderField=LoadedApk.getDeclaredField("mClassLoader");
            mClassLoaderField.setAccessible(true);

            Object mclassLoader=mClassLoaderField.get(LoadedApkInstance);
            Log.i("mclassLoader",mclassLoader.toString());
            Log.i("33333333333333333", sourceClassLoader.toString());
            mClassLoaderField.set(LoadedApkInstance,sourceClassLoader);

            Log.i("2222222222222222222222", LoadedApkInstance.toString());
        }catch (Exception e){
            Log.e("replaceClassLoader","修正加载器失败"+e);
        }
    }

    private ZipFile getApkZip() throws IOException {
        Log.i("demo",this.getApplicationInfo().sourceDir);
        ZipFile apkZipFile=new ZipFile(this.getApplicationInfo().sourceDir);
        return apkZipFile;
    }

    private byte[] readDexFileFromApk() throws IOException {
        ZipFile ApkZip=this.getApkZip();
        //获取 classes.dex 文件的条目
        ZipEntry zipEntry=ApkZip.getEntry("Class.dex");
        //打开 classes.dex 文件的输入流
        InputStream inputStream=ApkZip.getInputStream(zipEntry);
        //读取文件内容到字节数组中
        byte[] buffer=new byte[128];
        //一个字节输出流，它可以动态扩展大小，适合逐步写入数据。
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        int length;
        while((length=inputStream.read(buffer))>0){
            baos.write(buffer, 0, length);
        }
        return baos.toByteArray();
    }

    private byte[] decryptoSourceApk(byte[] sourceApkdata) {
        for (int i = 0; i < sourceApkdata.length; i++){
            sourceApkdata[i] ^= 0xff;
        }
        return sourceApkdata;
    }

    private byte[] splitSrcApkFromDex(byte[] dexFileData) throws IOException {
        //求合并apk的大小
        int length=dexFileData.length;
        //求源apk的大小
        //将这 8 个字节包装成一个 ByteBuffer 对象
        ByteBuffer byteBuffer=ByteBuffer.wrap(Arrays.copyOfRange(dexFileData,length-8,length));
        //apk存储文件为小端存储
        byteBuffer.order(java.nio.ByteOrder.LITTLE_ENDIAN);
        long SrcApkLength = byteBuffer.getLong();
        //从class.dex中读取相应大小的字节
        byte[] SrcApk= Arrays.copyOfRange(dexFileData,(int)(length-SrcApkLength-8),(length-8));
        //解密apk
        byte[] srcApkData=decryptoSourceApk(SrcApk);
        return SrcApk;
    }
}
