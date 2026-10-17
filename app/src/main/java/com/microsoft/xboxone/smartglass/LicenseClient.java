package com.microsoft.xboxone.smartglass;

import android.content.Context;
import android.provider.Settings;
import java.io.*;
import java.net.*;
import java.security.MessageDigest;
import java.security.KeyFactory;
import java.security.Signature;
import java.security.spec.X509EncodedKeySpec;
import android.util.Base64;
import java.util.UUID;
import javax.net.ssl.HttpsURLConnection;
import org.json.JSONObject;

final class LicenseClient {
    private static final String PUBLIC_KEY="MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAvHsZfTXegYEAOGORdKYNkPS6um79AIcjq3oZ/agVvvmcXktIMRNKkAeHopZNb3hDPT9M3tKwuxdOu0y/NQgdyDq7OCUI9kGm5T975W951nsgGCocYxcgAQLDe1Qp/F20Pvxox4kXuMT51j22/S2ykQzQD73aDWeiXB9NeBMqoZJTfqYv7/ROjZUvG3Oh1kJt1RgEHnpJx+VXzXIQMFeRkXkVy6aC/b40uI/YVgfGWe5TbSCqEZJUUd2KfzytHz4n2oTmeYBFOgF/wqTpA0vnSacCUHnpIJcr1RIFbWNnIknWAc3VTEzktPC7uUSUDrsIkrggMT9qtK5XwYMUoEKd6wIDAQAB";
    static final String ENDPOINT="https://stormxp999.com/storm/license";
    static final class Result {
        final boolean ok; final String message;
        Result(boolean ok,String message){this.ok=ok;this.message=message;}
    }
    static Result verify(Context context,String key){
        HttpsURLConnection connection=null;
        try {
            String id=Settings.Secure.getString(context.getContentResolver(),Settings.Secure.ANDROID_ID);
            if(id==null||id.length()==0)return new Result(false,"Device identity unavailable. Please restart your device.");
            byte[] digest=MessageDigest.getInstance("SHA-256").digest((context.getPackageName()+":"+id).getBytes("UTF-8"));
            StringBuilder device=new StringBuilder();for(byte b:digest)device.append(String.format(java.util.Locale.US,"%02x",b&255));
            String nonce=UUID.randomUUID().toString();
            String form="game=FreeFire&key="+URLEncoder.encode(key,"UTF-8")+"&hwid="+device+"&nonce="+nonce;
            connection=(HttpsURLConnection)new URL(ENDPOINT).openConnection();
            connection.setRequestMethod("POST");connection.setConnectTimeout(12000);connection.setReadTimeout(12000);
            connection.setInstanceFollowRedirects(false);connection.setUseCaches(false);connection.setDoOutput(true);
            connection.setRequestProperty("Content-Type","application/x-www-form-urlencoded; charset=UTF-8");
            connection.setRequestProperty("Accept","application/json");
            byte[] body=form.getBytes("UTF-8");connection.setFixedLengthStreamingMode(body.length);
            try(OutputStream out=connection.getOutputStream()){out.write(body);}
            int code=connection.getResponseCode();
            if(code==429)return new Result(false,"Too many attempts. Wait a minute and try again.");
            if(code>=300&&code<400)return new Result(false,"Panel redirected the request. Check the HTTPS endpoint.");
            InputStream stream=code>=400?connection.getErrorStream():connection.getInputStream();
            if(stream==null)return new Result(false,"Panel unavailable (HTTP "+code+").");
            ByteArrayOutputStream buffer=new ByteArrayOutputStream();
            try(InputStream in=stream){byte[] chunk=new byte[2048];int n;while((n=in.read(chunk))!=-1){if(buffer.size()+n>32768)throw new IOException("Response too large");buffer.write(chunk,0,n);}}
            String raw=buffer.toString("UTF-8");
            if(code==404)return new Result(false,"License endpoint not installed. Contact support.");
            if(!raw.trim().startsWith("{"))return new Result(false,"Panel returned a webpage instead of a license response (HTTP "+code+").");
            JSONObject json=new JSONObject(raw);
            boolean ok=code==200&&json.optBoolean("status",false)&&nonce.equals(json.optString("nonce"))&&"FreeFire".equals(json.optString("game"));
            if(ok){
                byte[] payload=Base64.decode(json.optString("grant"),Base64.DEFAULT);
                Signature verifier=Signature.getInstance("SHA256withRSA");
                verifier.initVerify(KeyFactory.getInstance("RSA").generatePublic(new X509EncodedKeySpec(Base64.decode(PUBLIC_KEY,Base64.DEFAULT))));
                verifier.update(payload);
                if(!verifier.verify(Base64.decode(json.optString("signature"),Base64.DEFAULT)))return new Result(false,"License response signature is invalid. Contact support.");
                JSONObject claims=new JSONObject(new String(payload,"UTF-8"));
                byte[] keyBytes=MessageDigest.getInstance("SHA-256").digest(key.getBytes("UTF-8"));
                StringBuilder keyHash=new StringBuilder();for(byte v:keyBytes)keyHash.append(String.format(java.util.Locale.US,"%02x",v&255));
                long now=System.currentTimeMillis()/1000L;
                ok=nonce.equals(claims.optString("nonce"))&&device.toString().equals(claims.optString("hwid"))&&"FreeFire".equals(claims.optString("game"))&&keyHash.toString().equals(claims.optString("key_hash"));
                if(!ok)return new Result(false,"License response does not match this request.");
                if(claims.optLong("issued_at")>now+120||claims.optLong("valid_until")<now)return new Result(false,"Verification timed out or device time is incorrect. Enable automatic date/time and retry.");
            }
            String message=json.optString("message","License could not be verified.");
            return new Result(ok,message.length()>200?message.substring(0,200):message);
        } catch(java.net.SocketTimeoutException e){return new Result(false,"Connection timed out. Check your network and retry.");}
          catch(javax.net.ssl.SSLException e){return new Result(false,"Secure connection failed. Check the domain certificate and device date.");}
          catch(Exception e){return new Result(false,"Cannot verify license. Check your connection and panel setup.");}
        finally{if(connection!=null)connection.disconnect();}
    }
}
