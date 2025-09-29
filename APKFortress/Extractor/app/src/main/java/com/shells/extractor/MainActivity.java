package com.shells.extractor;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import java.io.ObjectOutputStream;
import java.util.LinkedHashMap;
import java.util.Map;

import androidx.appcompat.app.AppCompatActivity;

import com.android.dex.ClassData;
import com.android.dex.ClassDef;
import com.android.dex.Code;
import com.android.dex.Dex;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.nio.file.Files;
import java.security.SecureRandom;
import java.util.HashMap;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;


public class MainActivity extends AppCompatActivity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        new Thread(() -> {
             Log.d("11","开始任务");

//            Test tester=new Test();
//            tester.test_loadInstructionMapFromAssets(this);
//                ReturnIns ri = new ReturnIns();
//            try {
////                ri.InstructionPatching(MainActivity.this);
//            } catch (IOException e) {
//                throw new RuntimeException(e);
//            }
//            try {
//                extractDEXFiles(this);
//            } catch (IOException e) {
//                throw new RuntimeException(e);
//            }
            runOnUiThread(() -> {
                Toast.makeText(MainActivity.this, "DEX回填完成", Toast.LENGTH_SHORT).show();
            });
        }).start();
    }

    public void extractDEXFiles(Context context) throws IOException {
        String currentDir = new File(".").getAbsolutePath();
        Log.d("CurrentPath: ", currentDir);

        // 1. Read APK from assets
        byte[] apkBytes = readApkFromAssets(this, "src.apk");
        Log.i("DEX Extraction", "APK size: " + apkBytes.length + " bytes");

        // 2. Extract DEX bytes from APK
        byte[] dexBytes = extractDexBytesFromApk(apkBytes);
        if (dexBytes == null) {
            throw new IOException("No DEX file found in APK");
        }

        Log.i("DEX Extraction", "Successfully extracted DEX, size: " + dexBytes.length + " bytes");

        // Optional: Save the DEX file to internal storage for verification
        saveDexToStorage(dexBytes,context);
    }

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

    private void saveDexToStorage(byte[] dexBytes,Context context) {
        File outputFile = new File(getFilesDir(), "extracted_classes.dex");
        try (FileOutputStream fos = new FileOutputStream(outputFile)) {
            fos.write(dexBytes);
            Log.i("DEX Extraction", "DEX saved to: " + outputFile.getAbsolutePath());
            // 如果dex文件以.dex结尾，则将文件名中的.dex替换为_extracted.dat，否则将文件名设置为_extracted.dat
            extractAllMethods(outputFile.getAbsolutePath(),context);
        } catch (IOException e) {
            Log.e("DEX Extraction", "Failed to save DEX file", e);
        }
    }

    public static void saveInstructionMap(Map<String, Instruction> instructionMap, String filePath) {
        try (ObjectOutputStream oos = new ObjectOutputStream(
                new FileOutputStream(filePath))) {
            oos.writeObject(instructionMap);
            System.out.println("Instruction map saved successfully to " + filePath);
        } catch (IOException e) {
            System.err.println("Failed to save instruction map: " + e.getMessage());
        }
    }


//    public void extractAllMethods(String dexPath) throws IOException {
//        File dexFile = new File(dexPath);
//        Dex dex = new Dex(dexFile);
//        Iterable<ClassDef> classDefs = dex.classDefs();
//
//        for (ClassDef classDef : classDefs) {
//            // 跳过没有 class_data_item 的类
//            if (classDef.getClassDataOffset() == 0) {
//                Log.d("提取", "跳过无数据的类: " + dex.typeNames().get(classDef.getTypeIndex()));
//                continue;
//            }
//            try {
//                //------------------DexClassData
//                ClassData dexclassData = dex.readClassData(classDef);
//                Log.d("提取", "成功提取到：" + dexclassData);
//                //------------DexMethod
//                //virtualMethods
//                ClassData.Method[] virtualMethods =null;
//                virtualMethods=dexclassData.getVirtualMethods();
//                //directMethods
//                ClassData.Method[] directMethods =null;
//                directMethods=dexclassData.getVirtualMethods();
//                //----DexMethodId
//
//                for(ClassData.Method  virtualMethod : virtualMethods){
//                   String methodname=dex.typeNames().get(virtualMethod.getMethodIndex());
//                   codeitem=virtualMethod.getCodeOffset();
//                    insnsSize=codeitem.insnsSize
//                }
//                //----DexCode
//
//
//            } catch (IllegalArgumentException e) {
//                Log.e("提取", "解析失败: " +  dex.typeNames().get(classDef.getTypeIndex()), e);
//            }
//        }
//    }
    private static final String[] excludeRule = {
            "Landroid/[^/]*",
            "Landroidx/.*",
            "Lcom/squareup/okhttp/.*",
            "Lokio/.*", "Lokhttp3/.*",
            "Lkotlin/.*",
            "Lcom/google/.*",
            "Lrx/.*",
            "Lorg/apache/.*",
            "Lretrofit2/.*",
            "Lcom/alibaba/.*",
            "Lcom/amap/api/.*",
            "Lcom/sina/weibo/.*",
            "Lcom/xiaomi/.*",
            "Lcom/eclipsesource/.*",
            "Lcom/blankj/utilcode/.*",
            "Lcom/umeng/.*",
            "Ljavax/.*",
            "Lorg/slf4j/.*",
             "Lkotlinx/.*"
    };

    public void extractAllMethods(String dexPath,Context context) throws IOException {

        Map<String, Instruction> maptable = new LinkedHashMap<>();

        Dex dex = new Dex(new File(dexPath));
        File modifiedDexFile = new File(context.getFilesDir(), "modified.dex");
        String FilePath = context.getFilesDir()+ "/element.dat";

        if (!modifiedDexFile.exists()) {
            Files.write(modifiedDexFile.toPath(), dex.getBytes());
        }

        Iterable<ClassDef> classDefs = dex.classDefs();

        for (ClassDef classDef : classDefs) {
            // 跳过没有 class_data_item 的类
            if (classDef.getClassDataOffset() == 0) {
                continue;
            }

            boolean skip = false;
            for(String rule : excludeRule) {
                if(classDef.toString().matches(rule)){
                    skip = true;
                    break;
                }
            }
            if(skip) {
                continue;
            }

            try {
                ClassData classData = dex.readClassData(classDef);
                for (ClassData.Method method : classData.getDirectMethods()) {

                    String className = dex.typeNames().get(dex.methodIds().get(method.getMethodIndex()).getDeclaringClassIndex());
                    String methodName = dex.strings().get(dex.methodIds().get(method.getMethodIndex()).getNameIndex());
                    String fullMethodName = className + "." + methodName;
                    Instruction ins=processMethod(dex, method, modifiedDexFile);

                    maptable.put(fullMethodName,ins);
                }
                for (ClassData.Method method : classData.getVirtualMethods()) {

                    String className = dex.typeNames().get(dex.methodIds().get(method.getMethodIndex()).getDeclaringClassIndex());
                    String methodName = dex.strings().get(dex.methodIds().get(method.getMethodIndex()).getNameIndex());
                    String fullMethodName = className + "." + methodName;
                    Instruction ins=processMethod(dex, method, modifiedDexFile);

                    maptable.put(fullMethodName,ins);
                }
                saveInstructionMap(maptable,FilePath);

            } catch (IllegalArgumentException e) {
                Log.e("提取", "解析失败: " + dex.typeNames().get(classDef.getTypeIndex()), e);
            }
        }
    }

    private static Instruction processMethod(Dex dex, ClassData.Method method, File modifiedDexFile) throws IOException {

        // 0. 基础检查
        if (method.getCodeOffset() == 0) {
            Log.d("processMethod", "跳过无代码的方法: offset=0");
            return null;
        }

        Instruction instructions = new Instruction();

        try {
            // 获取方法信息
            String className = dex.typeNames().get(dex.methodIds().get(method.getMethodIndex()).getDeclaringClassIndex());
            String methodName = dex.strings().get(dex.methodIds().get(method.getMethodIndex()).getNameIndex());
            String returnTypeName = dex.typeNames().get(dex.protoIds().get(dex.methodIds().get(method.getMethodIndex()).getProtoIndex()).getReturnTypeIndex());


            //------------------------获取原始信息--------------------
            //获取代码偏移量（CodeItem头部固定16字节）
            //0x0C	insns_size	4字节	指令的2字节单位数量
            //0x10	insns[]   (insns_size * 2)字节	实际指令开始
            int insOffset = method.getCodeOffset() + 16;
            // 读取原始代码
            Code code = dex.readCode(method);
            if (code == null || code.getInstructions().length == 0) {
                Log.d("processMethod", "跳过无指令的方法: " + className + "." + methodName);
                return null;
            }
            short[] ins=code.getInstructions();
            int insSize=ins.length * 2;
            byte[] bytecode = new byte[insSize * 2];
            for (int i = 0; i < ins.length; i++) {
                bytecode[i*2] = (byte) (ins[i] & 0xFF);
                bytecode[i*2+1] = (byte) ((ins[i] >> 8) & 0xFF);
            }

            //设置提取到到原始元素
            instructions.setMethodIndex(method.getMethodIndex());
            instructions.setInstructionDataSize(insSize);
            instructions.setInstructionsData(bytecode);

            //-----------------------------------------------------


            //--------------准备返回指令-------------------------
            byte[] returnBytes = getReturnByteCodes(returnTypeName);
            //验证空间是否足够
            if (returnBytes.length > insSize) {
                Log.w("processMethod", "指令空间不足: " + className + "." + methodName);
                return null;
            }

            //-----------------------修改DEX文件------------------------
            try (RandomAccessFile raf = new RandomAccessFile(modifiedDexFile, "rw")) {
                //清空方法体
                raf.seek(insOffset);
                for (int i = 0; i < code.getInstructions().length; i++) {
                    raf.writeShort(0); // 写入NOP (0x0000)
                }
                //写入返回指令
                raf.seek(insOffset);
                raf.write(returnBytes);
                raf.getFD().sync();


                Log.d("processMethod", "成功修改: " + className + "." + methodName +
                        " | 原始大小: " + insOffset + "B | 返回指令: " + returnBytes.length + "B");
                // 详细日志（十六进制输出前16字节）
                Log.d("DexDebug", String.format(
                        "方法 %s.%s @ 0x%X\n原始指令: %s\n返回指令: %s",
                        className, methodName, insOffset,
                        bytesToHex(bytecode, insSize),
                        bytesToHex(returnBytes, returnBytes.length)
                ));

            }
            return instructions;
        } catch (Exception e) {
            Log.e("processMethod", "处理方法失败: " + e.getMessage(), e);
            return null;
        }
    }

    // 辅助方法：字节数组转十六进制字符串
    private static String bytesToHex(byte[] bytes, int maxLength) {
        StringBuilder sb = new StringBuilder();
        int length = Math.min(bytes.length, maxLength);
        for (int i = 0; i < length; i++) {
            sb.append(String.format("%02X ", bytes[i]));
        }
        if (bytes.length > maxLength) {
            sb.append("...");
        }
        return sb.toString();
    }
//    private static Instruction processMethod(Dex dex,ClassData.Method method) {
//        try {
//            Instruction instructions=new Instruction();
//            // 获取方法名和类型
//            String methodName = dex.strings().get(dex.methodIds().get(method.getMethodIndex()).getNameIndex());
//            String className = dex.typeNames().get(dex.methodIds().get(method.getMethodIndex()).getDeclaringClassIndex());
//            String protoDesc = dex.readTypeList(dex.protoIds().get(dex.methodIds().get(method.getMethodIndex()).getProtoIndex()).getParametersOffset()).toString();
//            String returnTypeName = dex.typeNames().get(dex.protoIds().get(dex.methodIds().get(method.getMethodIndex()).getProtoIndex()).getReturnTypeIndex());
//            int  insnsOffset = method.getCodeOffset() + 16;
//            Log.d("提取方法", "类: " + className + ", 方法: " + methodName + ", 描述: " + protoDesc+", CodeItem入口： "+insnsOffset);
//            Code code = dex.readCode(method);
//            //Fault-tolerant handling
//            if(code.getInstructions().length == 0){
//                Log.d("processmethoc","method has no code,name =  %s.%s , returnType = %s");
//            }
//            int insnsCapacity = code.getInstructions().length;
//            //The insns capacity is not enough to store the return statement, skip it
//            // 获取返回类型的字节码
//            byte[] returnByteCodes = getReturnByteCodes(returnTypeName);
//
//            if(code.getInstructions().length * 2 < returnByteCodes.length) {
//                Log.w("Space Check", String.format(
//                        "指令空间不足: 需要%d字节但只有%d字节",
//                        returnByteCodes.length,
//                        code.getInstructions().length * 2
//                ));
//            }
//
//            Log.d("Instructions", "----- 原始指令 -----");
//            for (int i = 0; i < code.getInstructions().length; i++) {
//                short instruction = code.getInstructions()[i];
//                Log.d("Instruction", String.format(
//                        "[0x%04x] 0x%04x",
//                        insnsOffset + (i * 2),
//                        instruction & 0xFFFF
//                ));
//            }
//            RandomAccessFile outRandomAccessFile = null;
//            outRandomAccessFile = new RandomAccessFile(extractedDexFile, "rw");
//            //Here, MethodIndex corresponds to the index of the method_ids area
//            instructions.setMethodIndex(method.getMethodIndex());
//            //Note: Here is the size of the array
//            instructions.setInstructionDataSize(insnsCapacity * 2);
//            byte[] byteCode = new byte[insnsCapacity * 2];
//            //Write random bytes
//            SecureRandom insRandom = new SecureRandom();
//            for (int i = 0; i < insnsCapacity; i++) {
//                outRandomAccessFile.seek(insnsOffset + (i * 2));
//                byteCode[i * 2] = outRandomAccessFile.readByte();
//                byteCode[i * 2 + 1] = outRandomAccessFile.readByte();
//                outRandomAccessFile.seek(insnsOffset + (i * 2));
//                outRandomAccessFile.writeShort(insRandom.nextInt());
//            }
//            instructions.setInstructionsData(byteCode);
//            outRandomAccessFile.seek(insnsOffset);
//
//            return instructions;
//
//        } catch (Exception e) {
//            Log.e("提取方法", "处理方法失败", e);
//        }
//
//        return null;
//    }
//private static Instruction processMethod(Dex dex, ClassData.Method method,File modifiedDexFile) {
//    try {
//        Instruction instructions = new Instruction();
//        // 获取方法名和类型
//        String methodName = dex.strings().get(dex.methodIds().get(method.getMethodIndex()).getNameIndex());
//        String className = dex.typeNames().get(dex.methodIds().get(method.getMethodIndex()).getDeclaringClassIndex());
//        String protoDesc = dex.readTypeList(dex.protoIds().get(dex.methodIds().get(method.getMethodIndex()).getProtoIndex()).getParametersOffset()).toString();
//        String returnTypeName = dex.typeNames().get(dex.protoIds().get(dex.methodIds().get(method.getMethodIndex()).getProtoIndex()).getReturnTypeIndex());
//        int insnsOffset = method.getCodeOffset() + 16;
//        Log.d("提取方法", "类: " + className + ", 方法: " + methodName + ", 描述: " + protoDesc + ", CodeItem入口： " + insnsOffset);
//
//        Code code = dex.readCode(method);
//        // 容错处理
//        if (code.getInstructions().length == 0) {
//            Log.d("processmethod", "method has no code, name = " + className + "." + methodName + ", returnType = " + returnTypeName);
//            return null;
//        }
//
//        int insnsCapacity = code.getInstructions().length;
//        // 获取返回类型的字节码
//        byte[] returnByteCodes = getReturnByteCodes(returnTypeName);
//
//        if (code.getInstructions().length * 2 < returnByteCodes.length) {
//            Log.w("Space Check", String.format(
//                    "指令空间不足: 需要%d字节但只有%d字节",
//                    returnByteCodes.length,
//                    code.getInstructions().length * 2
//            ));
//            return null;
//        }
//
//        Log.d("Instructions", "----- 原始指令 -----");
//        for (int i = 0; i < code.getInstructions().length; i++) {
//            short instruction = code.getInstructions()[i];
//            Log.d("Instruction", String.format(
//                    "[0x%04x] 0x%04x",
//                    insnsOffset + (i * 2),
//                    instruction & 0xFFFF
//            ));
//        }
//
//        try (RandomAccessFile outRandomAccessFile = new RandomAccessFile(modifiedDexFile, "rw")) {
//            // 复制原始dex内容
//            byte[] originalDexBytes = dex.getBytes();
//            outRandomAccessFile.write(originalDexBytes);
//
//            // 设置Instruction对象
//            instructions.setMethodIndex(method.getMethodIndex());
//            instructions.setInstructionDataSize(insnsCapacity * 2);
//            byte[] byteCode = new byte[insnsCapacity * 2];
//
//            // 读取原始指令并填充随机值
//            SecureRandom insRandom = new SecureRandom();
//            for (int i = 0; i < insnsCapacity; i++) {
//                outRandomAccessFile.seek(insnsOffset + (i * 2));
//                byteCode[i * 2] = outRandomAccessFile.readByte();
//                byteCode[i * 2 + 1] = outRandomAccessFile.readByte();
//                outRandomAccessFile.seek(insnsOffset + (i * 2));
//                outRandomAccessFile.writeShort(0);
//            }
//            instructions.setInstructionsData(byteCode);
//
//            // 写入返回指令
//            outRandomAccessFile.seek(insnsOffset);
//            outRandomAccessFile.write(returnByteCodes);
//
//            Log.d("DEX修改", "修改后的DEX已保存到: " + modifiedDexFile.getAbsolutePath());
//        }
//
//        return instructions;
//    } catch (Exception e) {
//        Log.e("提取方法", "处理方法失败", e);
//        return null;
//    }
//}

    public static byte[] getReturnByteCodes(String typeName){
        byte[] returnVoidCodes = {(byte)0x0e , (byte)(0x0)};
        byte[] returnCodes = {(byte)0x12 , (byte)0x0 , (byte) 0x0f , (byte) 0x0};
        byte[] returnWideCodes = {(byte)0x16 , (byte)0x0 , (byte) 0x0 , (byte) 0x0, (byte) 0x10 , (byte) 0x0};
        byte[] returnObjectCodes = {(byte)0x12 , (byte)0x0 , (byte) 0x11 , (byte) 0x0};
        switch (typeName){
            case "V":
                return returnVoidCodes;
            case "B":
            case "C":
            case "F":
            case "I":
            case "S":
            case "Z":
                return returnCodes;
            case "D":
            case "J":
                return returnWideCodes;
            default: {
                return returnObjectCodes;
            }
        }
    }

}
