/*package com.microsoft.root;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class Initialize {

    public String daemonEXE = "BKLKILL";
    public static String socket;
    private static final String TAG = "DAEMON_INIT";

    public Initialize(final Context context) {
        System.loadLibrary("BKLSEC");

        try {
            PackageManager pm = context.getPackageManager();
            Intent launchIntent = pm.getLaunchIntentForPackage("com.dts.freefiremax");
            if (launchIntent != null) {
                context.startActivity(launchIntent);
            } else {
                Log.e(TAG, "Game package not found!");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    SharedPreferences prefs = context.getSharedPreferences("BKL_PREFS", Context.MODE_PRIVATE);
                    int rootMethod = prefs.getInt("ROOT_METHOD_INDEX", 0);

                    String internalPath = context.getFilesDir().getPath();
                    String internalExec = internalPath + "/" + daemonEXE;
                    String finalExecPath = "/data/local/tmp/" + daemonEXE;

                    if (!CopyFromAssets(context, internalPath, daemonEXE)) {
                        Log.e(TAG, "Failed to copy daemon from assets");
                        return;
                    }

                    StringBuilder script = new StringBuilder();
                    script.append("killall -9 ").append(daemonEXE).append("\n");
                    script.append("sleep 0.2\n");

                    String[] selinuxRules = {
                            "allow su untrusted_app process ptrace",
                            "allow su untrusted_app_32 process ptrace",
                            "allow su untrusted_app_30 process ptrace",
                            "allow su untrusted_app_29 process ptrace",
                            "allow su priv_app process ptrace",
                            "allow su untrusted_app file { read write open getattr }",
                            "allow su untrusted_app_32 file { read write open getattr }",
                            "allow su untrusted_app_30 file { read write open getattr }",
                            "allow su priv_app file { read write open getattr }",
                            "allow su su file { execute execute_no_trans open read write }"
                    };

                    switch (rootMethod) {
                        case 0:
                            //Log.d(TAG, "Using method: Magisk/Delta/Alpha");
                            script.append("if command -v magiskpolicy >/dev/null 2>&1; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    magiskpolicy --live \"").append(rule).append("\"\n");
                            }
                            script.append("elif command -v supolicy >/dev/null 2>&1; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    supolicy --live \"").append(rule).append("\"\n");
                            }
                            script.append("else\n");
                            for (String rule : selinuxRules) {
                                script.append("    sepolicy-inject --live \"").append(rule).append("\" >/dev/null 2>&1\n");
                            }
                            script.append("fi\n");
                            script.append("setenforce 0\n");
                            break;

                        case 1:
                            //Log.d(TAG, "Using method: KernelSU/APatch");
                            for (String rule : selinuxRules) {
                                script.append("magiskpolicy --live \"").append(rule).append("\" >/dev/null 2>&1\n");
                                script.append("supolicy --live \"").append(rule).append("\" >/dev/null 2>&1\n");
                            }
                            script.append("if ! command -v magiskpolicy >/dev/null && ! command -v supolicy >/dev/null; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    sepolicy-inject --live \"").append(rule).append("\" >/dev/null 2>&1\n");
                            }
                            script.append("fi\n");
                            script.append("setenforce 0\n");
                            break;

                        case 2:
                        default:
                            //Log.d(TAG, "Using method: Emulator/Cloud");
                            script.append("if command -v magiskpolicy >/dev/null 2>&1; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    magiskpolicy --live \"").append(rule).append("\"\n");
                            }
                            script.append("elif command -v supolicy >/dev/null 2>&1; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    supolicy --live \"").append(rule).append("\"\n");
                            }
                            script.append("fi\n");
                            script.append("setenforce 0\n");
                            break;
                    }

                    script.append("cp ").append(internalExec).append(" ").append(finalExecPath).append("\n");
                    script.append("chmod 777 ").append(finalExecPath).append("\n");
                    socket = "su -c " + finalExecPath;
                    script.append("nohup ").append(finalExecPath).append(" > /dev/null 2>&1 &\n");
                    script.append("rm -f ").append(internalExec).append("\n");

                    java.lang.Process suProcess = Runtime.getRuntime().exec("su");
                    DataOutputStream os = new DataOutputStream(suProcess.getOutputStream());
                    os.writeBytes(script.toString());
                    os.writeBytes("exit\n");
                    os.flush();
                    suProcess.waitFor();
                    os.close();

                    Log.d(TAG, "Daemon started successfully.");

                    new Handler(Looper.getMainLooper()).post(new Runnable() {
                        @Override
                        public void run() {
                            new Floater(context);
                        }
                    });

                } catch (Exception e) {
                    Log.e(TAG, "Error in background init: " + e.getMessage());
                    e.printStackTrace();
                }
            }
        }).start();
    }

    public static boolean CopyFromAssets(Context ctx, String outPath, String fileName) {
        File file = new File(outPath);
        if (!file.exists()) {
            file.mkdirs();
        }

        InputStream inputStream = null;
        FileOutputStream fileOutputStream = null;

        try {
            inputStream = ctx.getAssets().open(fileName);
            File outFile = new File(file, fileName);
            fileOutputStream = new FileOutputStream(outFile);

            byte[] buffer = new byte[4096];
            int byteRead;
            while ((byteRead = inputStream.read(buffer)) != -1) {
                fileOutputStream.write(buffer, 0, byteRead);
            }

            fileOutputStream.flush();
            return true;
        } catch (IOException e) {
            Log.e(TAG, "Copy Error: " + e.getMessage());
            return false;
        } finally {
            try {
                if (inputStream != null) inputStream.close();
                if (fileOutputStream != null) fileOutputStream.close();
            } catch (IOException e) {
                // Ignore
            }
        }
    }
}*/

package com.microsoft.xboxone.smartglass;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.io.DataOutputStream;

public class Initialize {

    private static final String TAG = "DAEMON_INIT";
    public static native String getDaemonMemFdPath();

    static {
        System.loadLibrary("cnative");
    }

    public Initialize(final Context context) {
        try {
            PackageManager pm = context.getPackageManager();
            Intent launchIntent = pm.getLaunchIntentForPackage("com.dts.freefiremax");
            if (launchIntent != null) {
                context.startActivity(launchIntent);
            } else {
                Log.e(TAG, "Game not found");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    SharedPreferences prefs = context.getSharedPreferences("BKL_PREFS", Context.MODE_PRIVATE);
                    int rootMethod = prefs.getInt("ROOT_METHOD_INDEX", 0);

                    String result = getDaemonMemFdPath();
                    if (result == null || result.isEmpty()) {
                        Log.e(TAG, "Failed memfd");
                        return;
                    }
                    String[] parts = result.split("\\|");
                    if (parts.length != 2) {
                        Log.e(TAG, "Invalid memfd: " + result);
                        return;
                    }
                    String daemonPath = parts[0];
                    String daemonName = parts[1];
                    Log.d(TAG, "Dae path: " + daemonPath + ", name: " + daemonName);

                    StringBuilder script = new StringBuilder();
                    script.append("killall -9 ").append(daemonName).append("\n");
                    script.append("sleep 0.2\n");
                    String[] selinuxRules = {
                            "allow su untrusted_app process ptrace",
                            "allow su untrusted_app_32 process ptrace",
                            "allow su untrusted_app_30 process ptrace",
                            "allow su untrusted_app_29 process ptrace",
                            "allow su priv_app process ptrace",
                            "allow su untrusted_app file { read write open getattr }",
                            "allow su untrusted_app_32 file { read write open getattr }",
                            "allow su untrusted_app_30 file { read write open getattr }",
                            "allow su priv_app file { read write open getattr }",
                            "allow su su file { execute execute_no_trans open read write }"
                    };

                    /*switch (rootMethod) {
                        case 0: // Magisk/Delta/Alpha
                            script.append("if command -v magiskpolicy >/dev/null 2>&1; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    magiskpolicy --live \"").append(rule).append("\"\n");
                            }
                            script.append("elif command -v supolicy >/dev/null 2>&1; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    supolicy --live \"").append(rule).append("\"\n");
                            }
                            script.append("else\n");
                            for (String rule : selinuxRules) {
                                script.append("    sepolicy-inject --live \"").append(rule).append("\" >/dev/null 2>&1\n");
                            }
                            script.append("fi\n");
                            script.append("setenforce 0\n");
                            break;

                        case 1: // KernelSU/APatch
                            for (String rule : selinuxRules) {
                                script.append("magiskpolicy --live \"").append(rule).append("\" >/dev/null 2>&1\n");
                                script.append("supolicy --live \"").append(rule).append("\" >/dev/null 2>&1\n");
                            }
                            script.append("if ! command -v magiskpolicy >/dev/null && ! command -v supolicy >/dev/null; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    sepolicy-inject --live \"").append(rule).append("\" >/dev/null 2>&1\n");
                            }
                            script.append("fi\n");
                            script.append("setenforce 0\n");
                            break;

                        case 2: // Emulator/Cloud
                        default:
                            script.append("if command -v magiskpolicy >/dev/null 2>&1; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    magiskpolicy --live \"").append(rule).append("\"\n");
                            }
                            script.append("elif command -v supolicy >/dev/null 2>&1; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    supolicy --live \"").append(rule).append("\"\n");
                            }
                            script.append("fi\n");
                            script.append("setenforce 0\n");
                            break;
                    }*/
                    switch (rootMethod) {
                        case 0: // Magisk
                            script.append("if command -v magiskpolicy >/dev/null 2>&1; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    magiskpolicy --live \"").append(rule).append("\"\n");
                            }
                            script.append("elif command -v supolicy >/dev/null 2>&1; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    supolicy --live \"").append(rule).append("\"\n");
                            }
                            script.append("fi\n");
                            break;

                        case 1: // KernelSU/APatch
                            for (String rule : selinuxRules) {
                                script.append("if command -v magiskpolicy >/dev/null 2>&1; then\n");
                                script.append("    magiskpolicy --live \"").append(rule).append("\"\n");
                                script.append("elif command -v supolicy >/dev/null 2>&1; then\n");
                                script.append("    supolicy --live \"").append(rule).append("\"\n");
                                script.append("fi\n");
                            }
                            break;

                        case 2: // Emulator
                            script.append("if command -v magiskpolicy >/dev/null 2>&1; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    magiskpolicy --live \"").append(rule).append("\"\n");
                            }
                            script.append("elif command -v supolicy >/dev/null 2>&1; then\n");
                            for (String rule : selinuxRules) {
                                script.append("    supolicy --live \"").append(rule).append("\"\n");
                            }
                            script.append("fi\n");
                            break;
                        case 3:
                            LoginActivity.setKernelDriverMode(true);
                            break;
                        default:
                            LoginActivity.setKernelDriverMode(false);
                            break;
                    }

                    script.append("nohup ").append(daemonPath).append(" > /dev/null 2>&1 &\n");
                    Process suProcess = Runtime.getRuntime().exec("su");
                    DataOutputStream os = new DataOutputStream(suProcess.getOutputStream());
                    os.writeBytes(script.toString());
                    os.writeBytes("exit\n");
                    os.flush();
                    suProcess.waitFor();
                    os.close();

                    Log.d(TAG, "Daemon started : " + daemonName);

                    new Handler(Looper.getMainLooper()).post(new Runnable() {
                        @Override
                        public void run() {
                            new Floater(context);
                        }
                    });

                } catch (Exception e) {
                    Log.e(TAG, "Error background init: " + e.getMessage());
                    e.printStackTrace();
                }
            }
        }).start();
    }
}
