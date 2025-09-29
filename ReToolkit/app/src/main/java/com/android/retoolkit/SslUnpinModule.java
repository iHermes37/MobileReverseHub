package com.android.retoolkit;

import android.net.http.SslError;
import android.os.Build;
import android.util.Log;
import android.webkit.SslErrorHandler;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebView;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.cert.X509Certificate;
import java.util.Date;
import java.util.List;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.KeyManager;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

//客户端验证服务端证书
public class SslUnpinModule implements IXposedHookLoadPackage {
    
    static String TAG = "hookSSLContext";
    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) throws Throwable {
        XposedBridge.log("ReToolkit loaded for: " + lpparam.packageName);

        safeHook(() -> hookSSLContext(lpparam.classLoader), "SSLContext");
        safeHook(() -> hookOkHttp(lpparam.classLoader), "OkHttp");
        safeHook(() -> hookWebView(lpparam.classLoader), "WebView");
        safeHook(() -> hookjsse(lpparam.classLoader), "JSSE");
        safeHook(() -> hookTrustManagerImpl(lpparam), "TrustManagerImpl");
        safeHook(() -> hookOpenSSLSocketImpl(lpparam), "OpenSSLSocketImpl");
        safeHook(() -> hookTrustKit(lpparam), "TrustKit");
        safeHook(() -> hookCronet(lpparam.classLoader), "Cronet");
        safeHook(() -> hookXutils(lpparam.classLoader), "XUtils");
        safeHook(() -> hookHttpClientAndroidLib(lpparam.classLoader), "HttpClientAndroidLib");
    }

    private void safeHook(Runnable hook, String name) {
        try {
            hook.run();
            XposedBridge.log("[ReToolkit] Hook " + name + " success");
        } catch (Throwable t) {
            XposedBridge.log("[ReToolkit] Hook " + name + " failed");
            XposedBridge.log(t);
        }
    }

    // -------------------工具函数-------------------------------
    private static class UniversalTrustManagerInvocationHandler implements InvocationHandler {
        @Override
        public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
            String methodName = method.getName();

            // 处理所有checkServerTrusted变体
            if (methodName.startsWith("checkServerTrusted")) {
                Log.d(TAG, "Bypassing server certificate check: " + methodName);
                return null; // 信任所有证书
            }

            // 处理checkClientTrusted
            if (methodName.startsWith("checkClientTrusted")) {
                Log.d(TAG, "Bypassing client certificate check");
                return null; // 信任所有客户端证书
            }

            // 处理getAcceptedIssuers
            if ("getAcceptedIssuers".equals(methodName)) {
                Log.d(TAG, "Returning empty accepted issuers");
                return new X509Certificate[0];
            }

            // 处理Android 7.0+的额外方法
            if ("isSameTrustConfiguration".equals(methodName)
                    || "isUserAddedCertificate".equals(methodName)) {
                return false;
            }

            // 默认行为
            Log.w(TAG, "Unhandled TrustManager method: " + methodName);
            return method.invoke(this, args);
        }
    }

    private Object createUniversalTrustManager(ClassLoader classLoader) throws ClassNotFoundException {
        try{

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q){
                return Proxy.newProxyInstance(
                        classLoader,
                        new Class<?>[] {
                                Class.forName("javax.net.ssl.X509TrustManager", false, classLoader),
                                // 针对某些厂商实现可能需要额外接口
                                Class.forName("android.net.http.X509TrustManagerExtensions", false, classLoader)
                        },
                        new UniversalTrustManagerInvocationHandler());
            }else{
                    // Android 9及以下使用常规实现
                    Class<?> x509TrustManagerClass = Class.forName(
                            "javax.net.ssl.X509TrustManager", false, classLoader);
                    return Proxy.newProxyInstance(
                            classLoader,
                            new Class<?>[] { x509TrustManagerClass },
                            new UniversalTrustManagerInvocationHandler());
                }
        }catch (Throwable t){
            Log.e(TAG, "Create universal TrustManager failed: " + t.getMessage());
            return null;
        }

    }

    public static SSLSocketFactory getEmptySSLSocketFactory() {
        try {
            // 创建信任所有证书的TrustManager
            TrustManager[] trustAllCerts = new TrustManager[]{
                    new X509TrustManager() {
                        public X509Certificate[] getAcceptedIssuers() {
                            return new X509Certificate[0];
                        }
                        public void checkClientTrusted(X509Certificate[] certs, String authType) {
                        }
                        public void checkServerTrusted(X509Certificate[] certs, String authType) {
                        }
                    }
            };

            // 创建SSLContext并使用上面的TrustManager
            SSLContext sslContext = SSLContext.getInstance("TLS");
            sslContext.init(null, trustAllCerts, new java.security.SecureRandom());

            return sslContext.getSocketFactory();

        } catch (NoSuchAlgorithmException | KeyManagementException e) {
            throw new RuntimeException("Failed to create empty SSLSocketFactory", e);
        }
    }

    public static HostnameVerifier getTrustAllHostnameVerifier() {
        return (hostname, session) -> true;
    }


    //-----------------具体Hook逻辑【客户端验证服务端证书】-----------------------------------
    private void hookSSLContext(ClassLoader classLoader){
        try{
            Object trustManager = createUniversalTrustManager(classLoader);

            XposedHelpers.findAndHookMethod("javax.net.ssl.SSLContext",classLoader,"init",
                    KeyManager[].class,
                    TrustManager[].class,
                    SecureRandom.class,
                    new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(MethodHookParam param) {
                            // 替换为我们的TrustManager
                            param.args[0] = null; // KeyManager
                            param.args[1] = new TrustManager[] {(TrustManager) trustManager};
                            param.args[2] = null; // SecureRandom
                            Log.d(TAG, "SSLContext.init() hooked with universal TrustManager");
                        }
                    });
        }catch(Throwable t) {
            Log.e(TAG, "Universal SSLContext hook failed: " + t.getMessage());
        }

        XposedHelpers.findAndHookMethod(
                "ppcelerator.https.PinningTrustManager",
                classLoader,
                "checkServerTrusted",
                X509Certificate[].class,
                String.class,
                new XC_MethodHook() {
                    @Override
                    protected void beforeHookedMethod(MethodHookParam param) {
                        XposedBridge.log("[+] Bypassed PinningTrustManager");
                        param.setThrowable(null); // 阻止抛出异常
                    }
                }
        );

    }

    private void hookOkHttp(ClassLoader classLoader){
            try{

                XposedHelpers.findAndHookMethod(
                        "okhttp3.CertificatePinner",
                        classLoader,
                        "check",
                        String.class,
                        List.class,
                        new XC_MethodHook() {
                            @Override
                            protected void beforeHookedMethod(MethodHookParam param) {
                                XposedBridge.log("[+] Bypassed CertificatePinner.check()");
                                param.setResult(null); // 直接跳过检查
                            }
                        }
                );

                XposedHelpers.findAndHookMethod(
                        "com.squareup.okhttp.OkHttpClient",
                        classLoader,
                        "setCertificatePinner",
                        "com.squareup.okhttp.CertificatePinner",
                        new XC_MethodHook() {
                            @Override
                            protected void beforeHookedMethod(MethodHookParam param) {
                                param.setResult(param.thisObject); // 使设置无效
                            }
                        }
                );

                XposedHelpers.findAndHookMethod(
                        "com.squareup.okhttp.CertificatePinner",
                        classLoader,
                        "check",
                        String.class,
                        List.class,
                        new XC_MethodHook() {
                            @Override
                            protected void beforeHookedMethod(MethodHookParam param) {
                                XposedBridge.log("[+] Bypassed CertificatePinner.check()");
                                param.setResult(null); // 直接跳过检查
                            }
                        }
                );



                XposedHelpers.findAndHookMethod(
                        "okhttp3.OkHttpClient$Builder",
                        classLoader,
                        "hostnameVerifier",
                        new XC_MethodHook() {
                            @Override
                            protected void afterHookedMethod(MethodHookParam param) {
                                Object alwaysTrueVerifier = Proxy.newProxyInstance(
                                        classLoader,
                                        new Class<?>[] { XposedHelpers.findClass("okhttp3.HostnameVerifier", classLoader) },
                                        (proxy, method, args) -> true // 所有 verify() 调用返回true
                                );
                                param.setResult(alwaysTrueVerifier);
                            }
                        }
                );






            }catch (Throwable e){
                XposedBridge.log("[-] Hook CertificatePinner failed: " + e.getMessage());
            }
    }

    private void hookWebView(ClassLoader classLoader){
            // Hook onReceivedSslError
            XposedHelpers.findAndHookMethod(
                    "android.webkit.WebViewClient",
                    classLoader,
                    "onReceivedSslError",
                    WebView.class,
                    SslErrorHandler.class,
                    SslError.class,
                    new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(MethodHookParam param) {
                            SslErrorHandler handler = (SslErrorHandler) param.args[1];
                            handler.proceed(); // 忽略错误
                            param.setResult(null);
                        }
                    }
            );

            // Hook onReceivedError (旧版)
            XposedHelpers.findAndHookMethod(
                    "android.webkit.WebViewClient",
                    classLoader,
                    "onReceivedError",
                    WebView.class,
                    int.class,
                    String.class,
                    String.class,
                    new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(MethodHookParam param) {
                            param.setResult(null); // 静默处理
                        }
                    }
            );

            XposedHelpers.findAndHookMethod(
                    "android.webkit.WebViewClient",  // 目标类
                   classLoader,            // 类加载器
                    "onReceivedError",              // 方法名
                    WebView.class,                  // 参数1: WebView
                    WebResourceRequest.class,       // 参数2: WebResourceRequest
                    WebResourceError.class,         // 参数3: WebResourceError
                    new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(MethodHookParam param) {
                            // 获取参数
                            WebView webView = (WebView) param.args[0];
                            WebResourceRequest request = (WebResourceRequest) param.args[1];
                            WebResourceError error = (WebResourceError) param.args[2];

                            // 打印日志（可选）
                            XposedBridge.log("[WebView] Error loading URL: " + request.getUrl() +
                                    ", Error: " + error.getDescription());

                            // 阻止默认错误处理（相当于 Frida 的 return 无操作）
                            param.setResult(null);
                        }
                    }
            );





    }

    private  void hookjsse(ClassLoader classLoader){
        // 2. Hook HttpsURLConnectionImpl.setSSLSocketFactory()
        XposedHelpers.findAndHookMethod(
                "com.android.okhttp.internal.huc.HttpsURLConnectionImpl",
                classLoader,
                "setSSLSocketFactory",
                "javax.net.ssl.SSLSocketFactory",
                new XC_MethodHook() {
                    @Override
                    protected void beforeHookedMethod(MethodHookParam param) {
                        XposedBridge.log("HttpsURLConnection.setSSLSocketFactory invoked");
                        // 可在此处修改SSLSocketFactory
                        // param.args[0] = yourCustomSslSocketFactory;
                    }
                }
        );

        // 3. Hook HttpsURLConnectionImpl.setHostnameVerifier()
        XposedHelpers.findAndHookMethod(
                "com.android.okhttp.internal.huc.HttpsURLConnectionImpl",
                classLoader,
                "setHostnameVerifier",
                "javax.net.ssl.HostnameVerifier",
                new XC_MethodHook() {
                    @Override
                    protected void beforeHookedMethod(MethodHookParam param) {
                        XposedBridge.log("HttpsURLConnection.setHostnameVerifier invoked");
                        // 可在此处修改HostnameVerifier
                        // param.args[0] = yourCustomHostnameVerifier;
                    }
                }
        );
   }

    private void hookTrustManagerImpl(XC_LoadPackage.LoadPackageParam lpparam) {
        Class<?> trustManagerImpl = XposedHelpers.findClass(
                "com.android.org.conscrypt.TrustManagerImpl",
                lpparam.classLoader);

        // Android 7.0+ verifyChain方法
        XposedHelpers.findAndHookMethod(trustManagerImpl, "verifyChain",
                XposedHelpers.findClass("java.util.List", lpparam.classLoader), // untrustedChain
                XposedHelpers.findClass("java.util.List", lpparam.classLoader), // trustAnchorChain
                String.class,                                                   // host
                boolean.class,                                                  // clientAuth
                byte[].class,                                                   // ocspData
                byte[].class,                                                   // tlsSctData
                new XC_MethodHook() {
                    @Override
                    protected void beforeHookedMethod(MethodHookParam param) {
                        XposedBridge.log("[+] TrustManagerImpl.verifyChain bypassed");
                        // 直接返回原始证书链（绕过验证）
                        param.setResult(param.args[0]);
                    }
                });

        // 兼容旧版checkTrusted方法（可选）
        try {
            XposedHelpers.findAndHookMethod(trustManagerImpl, "checkTrusted",
                    X509Certificate[].class,
                    String.class,
                    String.class,
                    new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(MethodHookParam param) {
                            XposedBridge.log("[+] TrustManagerImpl.checkTrusted bypassed");
                            param.setResult(param.args[0]); // 返回原始证书数组
                        }
                    });
        } catch (NoSuchMethodError e) {
            XposedBridge.log("checkTrusted method not found");
        }

    }

    private void hookOpenSSLSocketImpl(XC_LoadPackage.LoadPackageParam lpparam) {
        Class<?> openSSLSocketImpl = XposedHelpers.findClass(
                "com.android.org.conscrypt.OpenSSLSocketImpl",
                lpparam.classLoader);

        XposedHelpers.findAndHookMethod(openSSLSocketImpl, "verifyCertificateChain",
                long[].class,  // certRefs (Native指针数组)
                String.class,   // authMethod
                new XC_MethodHook() {
                    @Override
                    protected void beforeHookedMethod(MethodHookParam param) {
                        XposedBridge.log("[+] OpenSSLSocketImpl.verifyCertificateChain bypassed");
                        // 空实现 = 跳过验证
                        param.setResult(null);
                    }
                });

    }

    private void hookTrustKit(XC_LoadPackage.LoadPackageParam lpparam) {

        Class<?> okHostnameVerifier = XposedHelpers.findClass(
                "com.datatheorem.android.trustkit.pinning.OkHostnameVerifier",
                lpparam.classLoader);

        // Hook verify(String, SSLSession)
        XposedHelpers.findAndHookMethod(okHostnameVerifier, "verify",
                String.class,
                XposedHelpers.findClass("javax.net.ssl.SSLSession", lpparam.classLoader),
                new XC_MethodHook() {
                    @Override
                    protected void beforeHookedMethod(MethodHookParam param) {
                        XposedBridge.log("[+] TrustKit.verify(host, session) bypassed");
                        param.setResult(true); // 强制返回true
                    }
                });

        // Hook verify(String, X509Certificate)
        XposedHelpers.findAndHookMethod(okHostnameVerifier, "verify",
                String.class,
                X509Certificate.class,
                new XC_MethodHook() {
                    @Override
                    protected void beforeHookedMethod(MethodHookParam param) {
                        XposedBridge.log("[+] TrustKit.verify(host, cert) bypassed");
                        param.setResult(true); // 强制返回true
                    }
                });

    }

    private void hookCronet(ClassLoader classLoader) {
        try {
            XposedHelpers.findAndHookMethod("org.chromium.net.CronetEngine$Builder",
                    classLoader,
                    "enablePublicKeyPinningBypassForLocalTrustAnchors",
                    boolean.class,
                    new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(MethodHookParam param) {
                            XposedBridge.log("Enables or disables public key pinning bypass for local trust anchors = " + param.args[0]);
                            param.args[0] = true; // 强制设置为true
                        }
                    });

            XposedHelpers.findAndHookMethod("org.chromium.net.CronetEngine$Builder",
                    classLoader,
                    "addPublicKeyPins",
                    String.class, String[].class, boolean.class, Date.class,
                    new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(MethodHookParam param) {
                            XposedBridge.log("cronet addPublicKeyPins hostName = " + param.args[0]);
                            // 不执行原始方法，直接返回Builder对象
                            param.setResult(param.thisObject);
                        }
                    });
        } catch (Throwable t) {
            XposedBridge.log("Cronet pinner not found: " + t);
        }
    }

    private void hookXutils(ClassLoader classLoader) {
        try {
            // 创建自定义HostnameVerifier
            Class<?> hostnameVerifier = XposedHelpers.findClass("javax.net.ssl.HostnameVerifier", classLoader);
            Object trustVerifier = XposedHelpers.newInstance(
                    XposedHelpers.findClass("org.wooyun.TrustHostnameVerifier", classLoader));

            XposedHelpers.findAndHookMethod("org.xutils.http.RequestParams",
                    classLoader,
                    "setSslSocketFactory",
                    SSLSocketFactory.class,
                    new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(MethodHookParam param) {
                            param.args[0] = getEmptySSLSocketFactory();
                        }
                    });

            XposedHelpers.findAndHookMethod("org.xutils.http.RequestParams",
                  classLoader,
                    "setHostnameVerifier",
                    hostnameVerifier,
                    new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(MethodHookParam param) {
                            param.args[0] = trustVerifier;
                        }
                    });
        } catch (Throwable t) {
            XposedBridge.log("Xutils hooks not found: " + t);
        }
    }

    private void hookHttpClientAndroidLib(ClassLoader classLoader) {
        try {
            XposedHelpers.findAndHookMethod("ch.boye.httpclientandroidlib.conn.ssl.AbstractVerifier",
                    classLoader,
                    "verify",
                    String.class, String[].class, String[].class, boolean.class,
                    new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(MethodHookParam param) {
                            XposedBridge.log("httpclientandroidlib Hooks");
                            param.setResult(null);
                        }
                    });
        } catch (Throwable t) {
            XposedBridge.log("httpclientandroidlib Hooks not found: " + t);
        }
    }


    //-----------------------具体导出证书逻辑【服务端验证客户端证书】-------------------------------

}








