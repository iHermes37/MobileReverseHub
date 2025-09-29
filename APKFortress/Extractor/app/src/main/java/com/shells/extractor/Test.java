package com.shells.extractor;

import android.content.Context;
import android.util.Log;

import java.util.Map;

public class Test {

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


    public void test_loadInstructionMapFromAssets(Context context){

        ReturnIns ri=new ReturnIns();

        Map<String, Instruction>  maptable= ReturnIns.loadInstructionMapFromAssets(context,"element.dat");

        assert maptable != null;

        for (Map.Entry<String, Instruction> entry : maptable.entrySet()) {
            String key = entry.getKey();
            Instruction value = entry.getValue();
            if (value == null) {
                Log.e("Test", "Null Instruction for key: " + key);
                continue; // 跳过 null 值
            }

            byte[] data=value.getInstructionsData();

            Log.d("Test","key is: "+key+
                    "  data is: "+bytesToHex(data, value.getInstructionDataSize()));
            // 在这里可以对每个键值对进行操作
        }

        //key is: Lcom/android/sourceapp/Test;.testWithParams  data is: 22 00 10 15 1A 01 A7 19 70 20 0B A7 10 00 6E 20

       //方法 Lcom/android/sourceapp/Test;.testWithParams @ 0x2E8758 原始指令: 22 00 10 15 1A 01 A7 19 70 20 0B A7 10 00 6E 20

        //Self_test  D  method is testWithParams data is: 22 00 10 15 1A 01 A7 19 70 20

        //DexDebug   22 00 00 00 10 00 15 00 1A 00 01 00 A7 FF 19 00 70 00 20 00 0B 00 A7

        //即将填冲的数据： 22 00 10 15 1A 01 A7 19 70 20 0B A7


    }
}
