package com.microsoft.xboxone.smartglass;

import android.app.*;
import android.content.*;
import android.graphics.Typeface;
import android.net.Uri;
import android.os.*;
import android.provider.Settings;
import android.text.InputType;
import android.view.*;
import android.view.inputmethod.InputMethodManager;
import android.widget.*;
import java.util.concurrent.*;

public class LoginActivity {
    static {System.loadLibrary("cnative");}
    public static native String getObfuscatedString(int id);
    public static native int getObfuscatedColor(int id);
    public static native void setGameVersionNative(String version);
    public static native void setKernelDriverMode(boolean enable);
    private static native void setLicenseSession(boolean verified);
    private final MainActivity activity;
    private final Handler main=new Handler(Looper.getMainLooper());
    private final ExecutorService worker=Executors.newSingleThreadExecutor();
    private LinearLayout content;
    private EditText keyInput;
    private TextView status;
    private Button verifyButton,launchButton;
    private boolean verified,disposed,launching;
    private int generation;
    private final SharedPreferences prefs;
    private static final String[] ROOTS={"Magisk / Delta / Alpha","KernelSU / APatch","Emulator / Cloud","Kernel driver"};
    private static final String[] VERSIONS={"v2.132.x","v2.133.x"};
    public LoginActivity(Context context){activity=(MainActivity)context;prefs=activity.getSharedPreferences("BKL_PREFS",0);reset();}
    public void reset(){generation++;verified=false;launching=false;setLicenseSession(false);showLicense();}
    void cancelPending(){generation++;if(verifyButton!=null){verifyButton.setEnabled(true);verifyButton.setText("Verify license  →");}}
    void dispose(){disposed=true;generation++;worker.shutdownNow();main.removeCallbacksAndMessages(null);}
    private int dp(int n){return StormTheme.dp(activity,n);}
    private TextView text(String s,int size,int color,boolean bold){return StormTheme.text(activity,s,size,color,bold);}
    private void space(int n){View v=new View(activity);content.addView(v,new LinearLayout.LayoutParams(1,dp(n)));}
    private void page(String step,String title,String subtitle){
        ScrollView scroll=new ScrollView(activity);scroll.setFillViewport(true);scroll.setBackground(new StormBackdrop());scroll.setClipToPadding(false);
        LinearLayout outer=new LinearLayout(activity);outer.setOrientation(LinearLayout.VERTICAL);outer.setGravity(Gravity.CENTER);outer.setPadding(dp(20),dp(24),dp(20),dp(24));
        scroll.addView(outer,new ScrollView.LayoutParams(-1,-1));
        LinearLayout hero=new LinearLayout(activity);hero.setOrientation(LinearLayout.VERTICAL);hero.setGravity(Gravity.CENTER);
        ImageView logo=new ImageView(activity);logo.setImageResource(R.drawable.storm_logo);logo.setAdjustViewBounds(true);logo.setScaleType(ImageView.ScaleType.FIT_CENTER);logo.setContentDescription("STORM MODS logo");
        hero.addView(logo,new LinearLayout.LayoutParams(dp(142),dp(142)));
        TextView eyebrow=text("STORM MODS / ROOT INJECTOR",10,StormTheme.MUTED,true);eyebrow.setLetterSpacing(.15f);hero.addView(eyebrow);
        LinearLayout.LayoutParams hp=new LinearLayout.LayoutParams(-1,-2);hp.bottomMargin=dp(22);outer.addView(hero,hp);
        content=new LinearLayout(activity);content.setOrientation(LinearLayout.VERTICAL);content.setPadding(dp(22),dp(24),dp(22),dp(24));
        content.setBackground(StormTheme.shape(activity,StormTheme.CARD,24));content.setElevation(dp(8));
        int width=Math.min(dp(470),activity.getResources().getDisplayMetrics().widthPixels-dp(40));outer.addView(content,new LinearLayout.LayoutParams(width,-2));
        LinearLayout progress=new LinearLayout(activity);progress.setOrientation(LinearLayout.HORIZONTAL);
        for(int i=0;i<2;i++){View v=new View(activity);v.setBackground(StormTheme.shape(activity,(i==0||verified)?StormTheme.ACCENT:StormTheme.BORDER,3));LinearLayout.LayoutParams vp=new LinearLayout.LayoutParams(0,dp(3),1);vp.rightMargin=dp(6);progress.addView(v,vp);}content.addView(progress);space(16);
        TextView badge=text(step,10,StormTheme.ACCENT,true);badge.setLetterSpacing(.14f);content.addView(badge);space(12);
        content.addView(text(title,27,StormTheme.TEXT,true));space(8);content.addView(text(subtitle,13,StormTheme.MUTED,false));space(22);
        activity.setContentView(scroll);StormTheme.enter(content);
    }
    private void action(Button b){LinearLayout.LayoutParams p=new LinearLayout.LayoutParams(-1,dp(50));p.topMargin=dp(12);content.addView(b,p);}
    private void support(){space(20);TextView help=text("Support TG:  ↗   @stormxp999",12,StormTheme.MUTED,false);help.setGravity(Gravity.CENTER);help.setPadding(0,dp(12),0,dp(12));help.setOnClickListener(v->open("https://www.instagram.com/tejendra.xyz/"));content.addView(help);}
    private void open(String url){try{activity.startActivity(new Intent(Intent.ACTION_VIEW,Uri.parse(url)));}catch(ActivityNotFoundException e){Toast.makeText(activity,"No browser available",Toast.LENGTH_SHORT).show();}}
    private void showLicense(){
        page("01   ACCESS","Welcome to the storm mods.","Enter you Active License here .");
        content.addView(text("LICENSE KEY",10,StormTheme.MUTED,true));space(8);
        keyInput=new EditText(activity);keyInput.setSingleLine(true);keyInput.setTextSize(16);keyInput.setTextColor(StormTheme.TEXT);keyInput.setHintTextColor(StormTheme.MUTED);
        keyInput.setHint("Enter license key");keyInput.setInputType(InputType.TYPE_CLASS_TEXT|InputType.TYPE_TEXT_VARIATION_PASSWORD);
        keyInput.setPadding(dp(14),0,dp(14),0);keyInput.setBackground(StormTheme.shape(activity,StormTheme.BG,12));
        keyInput.setSaveEnabled(false);content.addView(keyInput,new LinearLayout.LayoutParams(-1,dp(54)));
        CheckBox show=new CheckBox(activity);show.setText("Show license");show.setTextColor(StormTheme.MUTED);show.setTextSize(12);
        show.setOnCheckedChangeListener((b,on)->{keyInput.setTransformationMethod(on?null:android.text.method.PasswordTransformationMethod.getInstance());keyInput.setSelection(keyInput.length());});content.addView(show);
        status=text("Encrypted connection.",12,StormTheme.MUTED,false);content.addView(status);
        verifyButton=StormTheme.button(activity,"Verify license ",true);action(verifyButton);verifyButton.setOnClickListener(v->verify());
        keyInput.setImeOptions(android.view.inputmethod.EditorInfo.IME_ACTION_DONE);
        keyInput.setOnEditorActionListener((v,id,event)->{if(id==android.view.inputmethod.EditorInfo.IME_ACTION_DONE){if(verifyButton.isEnabled())verify();return true;}return false;});support();
    }
    private void verify(){
        final String key=keyInput.getText().toString().trim();
        if(key.length()==0){keyInput.setError("Enter your license");return;}
        if(key.length()>90){keyInput.setError("License is too long");return;}
        final int request=++generation;
        verifyButton.setEnabled(false);verifyButton.setText("Verifying…");status.setText("Checking your license securely…");status.setTextColor(StormTheme.MUTED);
        worker.execute(()->{final LicenseClient.Result result=LicenseClient.verify(activity.getApplicationContext(),key);main.post(()->{
            if(disposed||request!=generation||activity.isFinishing())return;
            verifyButton.setEnabled(true);verifyButton.setText("Verify license  →");
            if(result.ok){verified=true;setLicenseSession(true);keyInput.setText("");
                InputMethodManager imm=(InputMethodManager)activity.getSystemService(Context.INPUT_METHOD_SERVICE);
                if(imm!=null)imm.hideSoftInputFromWindow(keyInput.getWindowToken(),0);showSetup();
            }else{setLicenseSession(false);status.setText(result.message);status.setTextColor(0xFFFF929B);}
        });});
    }
    private void showSetup(){
        page("02   CONFIGURATION","Make it your session.","License verified. Choose your existing root method and game version, then continue.");
        TextView chip=text("●  SESSION VERIFIED",11,StormTheme.ACCENT,true);content.addView(chip);space(18);
        selection("ROOT METHOD",ROOTS,"ROOT_METHOD_INDEX");space(14);selection("GAME VERSION",VERSIONS,"VERSION_INDEX");space(14);
        status=text("",12,StormTheme.MUTED,false);content.addView(status);
        launchButton=StormTheme.button(activity,Floater.hasActiveWindow()?"Open floating menu  →":"Continue  →",true);action(launchButton);launchButton.setOnClickListener(v->launch());
        Button signout=StormTheme.button(activity,"Lock session",false);action(signout);signout.setOnClickListener(v->reset());refreshPermission();support();
    }
    private void selection(String title,String[] options,String pref){
        content.addView(text(title,10,StormTheme.MUTED,true));space(6);
        int selected=Math.max(0,Math.min(options.length-1,prefs.getInt(pref,0)));
        Button b=StormTheme.button(activity,options[selected]+"  ▾",false);content.addView(b,new LinearLayout.LayoutParams(-1,dp(50)));
        b.setOnClickListener(v->new AlertDialog.Builder(activity).setTitle(title).setSingleChoiceItems(options,Math.max(0,Math.min(options.length-1,prefs.getInt(pref,0))),(dialog,index)->{prefs.edit().putInt(pref,index).apply();b.setText(options[index]+"  ▾");dialog.dismiss();}).setNegativeButton("Cancel",null).show());
    }
    void refreshPermission(){if(verified&&status!=null)status.setText(Build.VERSION.SDK_INT>=23&&!Settings.canDrawOverlays(activity)?"Next: allow the floating overlay permission.":"Overlay ready. Your license will not be requested in the floating menu.");}
    private void launch(){
        if(!verified||launching)return;
        if(Build.VERSION.SDK_INT>=23&&!Settings.canDrawOverlays(activity)){
            activity.permissionRoundTrip=true;
            try{activity.startActivityForResult(new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION,Uri.parse("package:"+activity.getPackageName())),2002);}catch(Exception e){activity.permissionRoundTrip=false;status.setText("Open Android Settings and allow display over other apps.");}return;
        }
        if(Floater.hasActiveWindow()){Floater.reopen();activity.finish();return;}
        int version=Math.max(0,Math.min(1,prefs.getInt("VERSION_INDEX",0)));
        setGameVersionNative(VERSIONS[version]);
        setKernelDriverMode(prefs.getInt("ROOT_METHOD_INDEX",0)==3);
        // Existing daemon launcher is intentionally unchanged.
        launching=true;launchButton.setEnabled(false);launchButton.setText("Starting…");
        new Initialize(activity);activity.finish();
    }
}
