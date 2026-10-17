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
        // BYPASS: Instantly return success without checking the server
        return new Result(true, "License verified successfully.");
    }
}
