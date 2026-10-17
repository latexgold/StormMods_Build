package com.microsoft.xboxone.smartglass;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;
import com.topjohnwu.superuser.ipc.RootService;

public class RootServices extends RootService implements Handler.Callback {

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "RootServices: onBind");
        Handler handler = new Handler(Looper.getMainLooper(), this);
        Messenger messenger = new Messenger(handler);
        return messenger.getBinder();
    }

    @Override
    public boolean handleMessage(Message msg) {
        if (msg.what != MSG_GETINFO) return false;
        Message reply = Message.obtain();
        Bundle data = new Bundle();
        String pkg = msg.getData().getString("pkg");
        String fileSo = msg.getData().getString("fileSo");
        int offset = msg.getData().getInt("offset");
        String hex = msg.getData().getString("hexNumber");
        data.putString("result", convert(Tools.setCode(pkg, fileSo, offset, hex)));
        reply.setData(data);
        try {
            msg.replyTo.send(reply);
        } catch (RemoteException e) {
            Log.e(TAG, "Remote error", e);
        }
        return false;
    }

    public String convert(boolean boo){
        if(boo)
            return "true";
        else
            return "false";
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d(TAG, "RootServices: onUnbind, client process unbound");
        // Default returns false, which means NOT daemon mode
        return super.onUnbind(intent);
    }

    public static final String TAG = "IPC";
    public static final int MSG_GETINFO = 1;
}