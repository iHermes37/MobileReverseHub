package com.shells.land;
import android.util.Log;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
public class Refinvoke {
    //反射获取静态方法
    public static Object invokeStaticMethood(String className,String methodName,Class<?>[] paramTypes,Object[] params) {
        final String TAG = "invokeStaticMethood";
        try{
            // 参数校验
            if (paramTypes.length != params.length) {
                Log.d(TAG,"参数类型与参数值数量不匹配");
                throw new IllegalArgumentException("参数类型与参数值数量不匹配");
            }
            Class<?> clazz=Class.forName(className);
            Method method=clazz.getMethod(methodName,paramTypes);
            return method.invoke(null,params);
        }catch (Exception e){
            Log.e(TAG,"静态方法获取失败:"+methodName,e); // 打印异常信息
            return null;
        }
    }

    //反射获取实例方法
    public Object invokeInstanceMethod(String className,String methodName,Object instance,Class<?>[] paramTypes,Object[] params)
    {
        final String TAG = "invokeInstanceMethod";
        try{
            // 参数校验
            if (paramTypes.length != params.length) {
                Log.d(TAG,"参数类型与参数值数量不匹配");
                throw new IllegalArgumentException("参数类型与参数值数量不匹配");
            }
            Class<?> clazz=Class.forName(className);
            Method method=clazz.getMethod(methodName,paramTypes);
            return method.invoke(instance,params);
        }catch (Exception e){
            Log.e(TAG,"实例方法获取失败:"+methodName,e); // 打印异常信息
            return null;
        }
    }

    //反射获取类的静态字段的值
    public Object getStaticField(String className,String fieldName)  {
        final String TAG = "getStaticField";
        try{
            Class<?> clazz=Class.forName(className);
            Field field=clazz.getDeclaredField(fieldName);
            return field.get(null);
        }catch (Exception e){
            Log.e(TAG,"获取类的静态字段的值:"+fieldName,e); // 打印异常信息
            return null;
        }
    }

    //反射设置类的静态字段的值
    public void setStaticField(String className,String fieldName,Object value)  {
        final String TAG = "setStaticField";
        try{
            Class<?> clazz=Class.forName(className);
            Field field=clazz.getDeclaredField(fieldName);
            field.setAccessible(true);
            field.set(null,value);
        }catch (Exception e){
            Log.e(TAG,"设置类的静态字段的值:"+fieldName,e); // 打印异常信息
        }
    }

    //反射获取对象实例字段的值
    public Object getInstanceField(String className,String fieldName,Object instance) {
        final String TAG = "getInstanceField";
        try{
            Class<?> clazz=Class.forName(className);
            Field field=clazz.getDeclaredField(fieldName);
            field.setAccessible(true);
            return field.get(instance);
        }catch (Exception e){
            Log.e(TAG,"获取对象实例字段的值:"+fieldName,e); // 打印异常信息
            return null;
        }
    }

    //反射设置对象实例字段的值
    public void setInstanceField(String className,String fieldName,Object instance,Object value) {
        final String TAG = "setInstanceField";
        try{
            Class<?> clazz=Class.forName(className);
            Field field=clazz.getDeclaredField(fieldName);
            field.setAccessible(true);
            field.set(instance,value);
        }catch (Exception e){
            Log.e(TAG,"获取对象实例字段的值:"+fieldName,e); // 打印异常信息
        }
    }

}
