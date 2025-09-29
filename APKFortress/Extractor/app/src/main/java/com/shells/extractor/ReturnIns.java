package com.shells.extractor;
import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

import com.android.dex.ClassData;
import com.android.dex.ClassDef;
import com.android.dex.Code;
import com.android.dex.Dex;
import com.google.gson.Gson;
import com.google.gson.JsonSyntaxException;
import com.google.gson.reflect.TypeToken;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.ObjectInputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.lang.reflect.Type;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.security.DigestException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.zip.Adler32;


public class ReturnIns {


    public static Map<String, Instruction> loadInstructionMapFromAssets(Context context, String assetFileName) {
        AssetManager assetManager = context.getAssets();
        try (InputStream inputStream = assetManager.open(assetFileName);
             ObjectInputStream ois = new ObjectInputStream(inputStream)) {

            return (Map<String, Instruction>) ois.readObject();

        } catch (IOException | ClassNotFoundException e) {
            System.err.println("Failed to load instruction map from assets: " + e.getMessage());
            return null;
        }
    }


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

    public void InstructionPatching(Context context) throws IOException {
        // 准备输入输出文件
        File inputFile = new File(context.getCacheDir(), "modified.dex");
        File outputFile = new File(context.getFilesDir(), "patched.dex");

        // 从assets复制DEX文件到缓存目录
        try (InputStream is = context.getAssets().open("modified.dex");
             OutputStream os = new FileOutputStream(inputFile)) {
            byte[] buffer = new byte[8192];  // 使用更大的缓冲区提高复制效率
            int length;
            while ((length = is.read(buffer)) > 0) {
                os.write(buffer, 0, length);
            }
        }

        // 加载指令映射文件
        Map<String, Instruction> insTable = loadInstructionMapFromAssets(context, "element.dat");
        if (insTable == null || insTable.isEmpty()) {
            throw new IOException("无法加载或解析element.dat文件");
        }

        try (RandomAccessFile dexFile = new RandomAccessFile(inputFile, "rw");
              FileChannel outChannel = new FileOutputStream(outputFile).getChannel()) {

            // 1. 读取整个DEX文件到内存
            byte[] dexBytes = new byte[(int)dexFile.length()];
            dexFile.readFully(dexBytes);
            dexFile.seek(0); // 重置文件指针

            Dex dex = new Dex(dexBytes); // 使用内存中的字节数组创建Dex对象
            boolean modified = false;

            // 2. 处理类和方法
            for (ClassDef classDef : dex.classDefs()) {
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
                    ClassData classinfo = dex.readClassData(classDef);
                    processMethods(dex, classDef,
                            List.of(classinfo.getDirectMethods()),
                            insTable, dexBytes);
                    processMethods(dex, classDef,
                            List.of(classinfo.getVirtualMethods()),
                            insTable, dexBytes);
                } catch (IllegalArgumentException e) {
                    Log.e("DEX处理", "解析失败: " + dex.typeNames().get(classDef.getTypeIndex()), e);
                }
            }
            //modified=true;
            // 3. 如果有修改，更新校验和和签名
//            if (modified) {
//                updateDexChecksumAndSignature(dexBytes);
//            }

            // 4. 写入处理后的数据
            try (ByteArrayInputStream bis = new ByteArrayInputStream(dexBytes)) {
                byte[] buffer = new byte[8192];
                int length;
                while ((length = bis.read(buffer)) > 0) {
                    outChannel.write(ByteBuffer.wrap(buffer, 0, length));
                }
            }

            Log.d("DEX处理", "处理完成，输出文件: " + outputFile.getAbsolutePath());
        } catch (IOException e) {
            Log.e("DEX处理", "文件操作失败", e);
            throw e;
        }
    }

    // 新增的校验和更新方法（保持其他代码不变）
    private static void updateDexChecksumAndSignature(byte[] dexBytes) throws IOException {
        // 跳过magic(8字节)和checksum(4字节)，从第12字节开始计算
        Adler32 checksum = new Adler32();
        checksum.update(dexBytes, 12, dexBytes.length - 12);
        int checksumValue = (int) checksum.getValue();

        // 更新checksum (小端序，偏移量8)
        System.arraycopy(ByteBuffer.allocate(4)
                .order(ByteOrder.LITTLE_ENDIAN)
                .putInt(checksumValue).array(), 0, dexBytes, 8, 4);

        // 计算SHA-1签名(从32字节开始到文件结束)
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-1");
            int signature = md.digest(dexBytes, 32, dexBytes.length - 32);
            System.arraycopy(signature, 0, dexBytes, 12, 20); // 签名放在12-31字节
        } catch (NoSuchAlgorithmException e) {
            throw new IOException("SHA-1算法不可用", e);
        } catch (DigestException e) {
            throw new RuntimeException(e);
        }
    }

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

    private boolean isAllZero(byte[] data) {
        for (byte b : data) {
            if (b != 0) return false;
        }
        return true;
    }

    private void processMethods(Dex dex, ClassDef classDef,
                                List<ClassData.Method> methods,
                                Map<String, Instruction> instable,
                                byte[] dexBytes) throws IOException {

        for (ClassData.Method method : methods) {
            String className = dex.typeNames().get(classDef.getTypeIndex());

            //找到回填的指令
            String methodName = dex.strings().get(dex.methodIds().get(method.getMethodIndex()).getNameIndex());
            String fullMethodName = className + "." + methodName;

            Instruction ins = instable.get(fullMethodName);
            if (ins == null || method.getCodeOffset() == 0) continue;

            Log.d("Self_test","method is "+ methodName+" data is: "+bytesToHex(ins.getInstructionsData(),ins.getInstructionDataSize()));


            //验证
            int insSize=ins.getInstructionDataSize();
            byte[] bytes=ins.getInstructionsData();

            if (isAllZero(bytes)) {
                throw new IOException("无效的全零指令数据: " + fullMethodName);
            }
            Log.d("DexDebug", "即将填冲的数据： "+bytesToHex(bytes, insSize));


            //获取需要回填的方法名
            Code code = dex.readCode(method);
            if (ins.getInstructionDataSize() > code.getInstructions().length * 2) {
                Log.e("DexPatch", "指令数据超出方法体边界: " + methodName);
                continue;
            }

            short[] ins1=code.getInstructions();
            int insSize1=ins1.length * 2;
            byte[] bytecode1 = new byte[insSize * 2];
            for (int i = 0; i < ins1.length; i++) {
                bytecode1[i*2] = (byte) (ins1[i] & 0xFF);
                bytecode1[i*2+1] = (byte) ((ins1[i] >> 8) & 0xFF);
            }
            Log.d("DexDebug", "填充前指令： "+bytesToHex(bytecode1, insSize1));

            //计算指令偏移量（CodeItem头部16字节）
            int insnsOffset = method.getCodeOffset() + 16;// code_item头部固定16字节

            if (insnsOffset + ins.getInstructionDataSize() > dexBytes.length) {
                throw new IOException("写入位置超出文件末尾");
            }

            System.arraycopy(ins.getInstructionsData(), 0,
                    dexBytes, insnsOffset,
                    ins.getInstructionDataSize());

            Log.d("DexPatch", "回填方法: " + methodName +
                    " 偏移: 0x" + Integer.toHexString(insnsOffset) +
                    " 回填指令大小: " + ins.getInstructionDataSize() + "字节"+
                    " 原来指令大小: " + insSize1 + "字节"
                    );
        }
    }
}
