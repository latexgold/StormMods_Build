package com.microsoft.xboxone.smartglass;


import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;

class Memory {
    String pkg;
    int pid;
    long sAddress;

    Memory(String pkg) {
        this.pkg = pkg;
    }
}

public class Tools {
    private static final String TAG = "AMOSTRADIM";

    public static boolean setCode(String pkg, String lib, int offset, String hex) {
        Memory mem = new Memory(pkg);
        getProcessID(mem);
        if (mem.pid > 1 && mem.sAddress < 1L) {
            parseMap(mem, lib);
            Log.d(TAG, "parseMap");
            return memEdit(mem, offset, hex);
        } else if (mem.pid > 1 && mem.sAddress > 1L) {
            Log.d(TAG, "sem parseMap");
            return memEdit(mem, offset, hex);
        } else {
            return false;
        }
    }

    private static void parseMap(Memory nmax, String libName) {
        File fil = new File("/proc/" + nmax.pid + "/maps");
        if (fil.exists()) {
            try {
                BufferedReader reader = new BufferedReader(new InputStreamReader(new ProcessBuilder("cat", fil.getPath()).start().getInputStream()));
                String line;
                while ((line = reader.readLine()) != null) {
                    if (line.contains(libName) && nmax.sAddress == 0L) {
                        //Log.d(TAG, "lib: " + libName);
                        String[] lines = line.split("\\s+");
                        String[] addresses = lines[0].split("-");
                        nmax.sAddress = Long.parseLong(addresses[0], 16);
                    }
                }
                reader.close();
            } catch (IOException e) {
                Log.e(TAG, "Erro ao ler o arquivo maps: " + e.getMessage());
            }
        } else {
            Log.e(TAG, "O diretório maps não existe para o processo com PID: " + nmax.pid);
        }
    }

    private static byte[] hex2b(String hexs) {
        String hex = hexs.replace(" ", "");
        if (hex.length() % 2 != 0) {
            //throw new IllegalArgumentException("Unexpected hex string: " + hex);
            Log.d(TAG,"Unexpected hex string: " + hex);
        }
        byte[] result = new byte[hex.length() / 2];
        for (int i = 0; i < result.length; i++) {
            int d1 = decodeHexDigit(hex.charAt(i * 2)) << 4;
            int d2 = decodeHexDigit(hex.charAt(i * 2 + 1));
            result[i] = (byte) (d1 + d2);
        }
        return result;
    }

    private static int decodeHexDigit(char paramChar) {
        if (paramChar >= '0' && paramChar <= '9') {
            return paramChar - '0';
        }
        if (paramChar >= 'a' && paramChar <= 'f') {
            return paramChar - 'a' + 10;
        }
        if (paramChar >= 'A' && paramChar <= 'F') {
            return paramChar - 'A' + 10;
        }
        Log.d(TAG,"Unexpected hex digit: " + paramChar);
        throw new IllegalArgumentException("Unexpected hex digit: " + paramChar);
    }

    private static boolean memEdit(Memory nmax, int offset, String hexT) {
        try {
            RandomAccessFile channel = new RandomAccessFile("/proc/" + nmax.pid + "/mem", "rw");
            ByteBuffer buff = ByteBuffer.wrap(hex2b(hexT));
            for (int i = 0; i < hexT.length() / 2; i++) {
                channel.getChannel().write(buff, nmax.sAddress + offset + i);
            }
            channel.close();
            return true;
        } catch (Exception e) {
            Log.w(TAG, e);
            return false;
        }
    }

    private static void getProcessID(Memory nmax) {
        try {
            Process process = Runtime.getRuntime().exec(new String[]{"pidof", nmax.pkg});
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String buff = reader.readLine();
            reader.close();
            process.waitFor();
            process.destroy();
            nmax.pid = buff != null && !buff.isEmpty() ? Integer.parseInt(buff) : 0;
        } catch (IOException | InterruptedException e) {
            Log.w(TAG, e);
        }
    }
}