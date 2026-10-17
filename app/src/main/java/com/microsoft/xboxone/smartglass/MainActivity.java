package com.microsoft.xboxone.smartglass;

import android.app.Activity;
import android.os.Bundle;
import android.content.Intent;

public class MainActivity extends Activity {
    private LoginActivity screen;
    private boolean background;
    boolean permissionRoundTrip;
    @Override public void onCreate(Bundle state){
        super.onCreate(state);
        getWindow().setStatusBarColor(StormTheme.BG);
        getWindow().setNavigationBarColor(StormTheme.BG);
        screen=new LoginActivity(this);
    }
    @Override protected void onStart(){
        super.onStart();
        if(background&&!permissionRoundTrip&&screen!=null)screen.reset();
        background=false;
    }
    @Override protected void onStop(){super.onStop();background=true;if(screen!=null)screen.cancelPending();}
    @Override protected void onActivityResult(int request,int result,Intent data){
        super.onActivityResult(request,result,data);
        if(request==2002){permissionRoundTrip=false;background=false;if(screen!=null)screen.refreshPermission();}
    }
    @Override protected void onDestroy(){if(screen!=null)screen.dispose();super.onDestroy();}
}
