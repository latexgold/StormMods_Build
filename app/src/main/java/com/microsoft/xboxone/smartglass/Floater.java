package com.microsoft.xboxone.smartglass;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.animation.ValueAnimator;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.PixelFormat;
import android.graphics.Typeface;
import android.graphics.drawable.ClipDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.LayerDrawable;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.text.Editable;
import android.text.Html;
import android.text.TextWatcher;
import android.util.Base64;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.Interpolator;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.ScrollView;
import android.widget.HorizontalScrollView;
import android.widget.SeekBar;
import android.widget.TextView;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

public class Floater {
    private static Floater activeWindow;
    public static boolean hasActiveWindow(){return activeWindow!=null&&activeWindow.rootFrame!=null&&activeWindow.rootFrame.isAttachedToWindow();}
    public static void reopen(){if(hasActiveWindow())activeWindow.expandMenu();}

    public static native String getObfuscatedString(int id);
    public static int getObfuscatedColor(int id){
        switch(id){case 200:return 0xFF5DE2FF;case 201:return 0xFF0C1425;case 202:return 0xFF293B5E;case 203:return 0xFF9BAAC2;case 204:return 0xFF243858;case 205:return 0xFF142239;case 206:return 0xFFB5C1D6;case 207:return 0xFF19283F;case 208:return 0xFF22324B;case 209:return 0xFFBBC8DE;case 210:return 0xFF355DC0;case 211:return 0xFF5DE2FF;case 212:return 0xFF6DE6BF;case 213:return 0xFFFF8EAB;case 214:return 0xFFF3F7FF;case 215:return 0x00000000;case 216:return 0xFF19243A;default:return 0xFFF3F7FF;}
    }
    private static MSGConnection conn;
    private static Messenger remoteMessenger;
    private static final Messenger myMessenger = new Messenger(new Handler(Looper.getMainLooper(), null));
    public static native void Functions();
    public static native void ChangesID(int ID, int Value);
    public static native void ChangesString(int ID, String text);
    public static native void ftKZXivSr(Context context);
    public static native String getBase64Icon();

    public static boolean serviceTestQueued;
    private java.util.Timer timer;
    public static int SCREEN_WIDTH = 0;
    public static int SCREEN_HEIGHT = 0;

    public static int Width() {
        return SCREEN_WIDTH;
    }

    public static int Height() {
        return SCREEN_HEIGHT;
    }
    private int buttonClick = 0;
    public static int PrimaryColor;
    public static Context context;
    private static Utils utils;
    private FrameLayout rootFrame;
    private RelativeLayout rootRelative;
    private RelativeLayout collapsedRelative;
    private LinearLayout menuLayout;
    private ImageView logoImagem;
    private LinearLayout tabCima;
    private RelativeLayout tabCimaRelative;
    private TextView tituloText;
    private static TextView loginLogText;
    public static FrameLayout pagesContainer;
    public static LinearLayout tabContainer;
    public static List<LinearLayout> tabPages = new ArrayList<>();
    public static List<LinearLayout> tabButtons = new ArrayList<>();

    private static int currentTabIndex = 0;
    private LinearLayout tabBaixo;
    private RelativeLayout tabBaixoRelative;
    private Button closeButton;
    private Interpolator fastOutSlowIn = new AccelerateDecelerateInterpolator();
    private boolean isMenuExpanded = false;
    private AnimatorSet panelAnimation;

    public Bitmap getBitmapFromAssets(Context context, String fileName) {
        AssetManager assetManager = context.getAssets();
        InputStream istr;
        Bitmap bitmap = null;
        try {
            istr = assetManager.open(fileName);
            bitmap = BitmapFactory.decodeStream(istr);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return bitmap;
    }

    private Bitmap base64ToBitmap(String base64Str) {
        try {
            byte[] decodedBytes = Base64.decode(base64Str, Base64.DEFAULT);
            return BitmapFactory.decodeByteArray(decodedBytes, 0, decodedBytes.length);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    private WindowManager windowManager;
    private WindowManager.LayoutParams windowManagerParams;

    ESPView drawView;
    WindowManager.LayoutParams windowManagerDrawViewParams;

    public static native void PxbftKZXivSr(ESPView espView, Canvas canvas, int width, int height);

    public static void toggleKeyboardFocus(boolean focusable, EditText editText) {
        try {
            if (container_features == null || context == null) return;

            WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            View rootView = container_features.getRootView();
            WindowManager.LayoutParams lp = (WindowManager.LayoutParams) rootView.getLayoutParams();

            if (focusable) {
                lp.flags &= ~WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
                lp.flags |= WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL;
                wm.updateViewLayout(rootView, lp);

                if (editText != null) {
                    editText.requestFocus();
                    InputMethodManager imm = (InputMethodManager) context.getSystemService(Context.INPUT_METHOD_SERVICE);
                    if (imm != null) {
                        imm.showSoftInput(editText, InputMethodManager.SHOW_IMPLICIT);
                    }
                }
            } else {
                lp.flags |= WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
                lp.flags &= ~WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL;
                wm.updateViewLayout(rootView, lp);

                InputMethodManager imm = (InputMethodManager) context.getSystemService(Context.INPUT_METHOD_SERVICE);
                if (imm != null) {
                    imm.hideSoftInputFromWindow(container_features.getWindowToken(), 0);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void DrawCanvas() {
        int LAYOUT_FLAG;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            LAYOUT_FLAG = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
        } else {
            LAYOUT_FLAG = WindowManager.LayoutParams.TYPE_PHONE;
        }

        drawView = new ESPView(context);

        windowManagerDrawViewParams = new WindowManager.LayoutParams(
                -1, -1,
                this.getLayoutType(),
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE |
                        WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE,
                PixelFormat.TRANSLUCENT);

        windowManagerDrawViewParams.gravity = Gravity.TOP | Gravity.START;
        windowManagerDrawViewParams.x = 0;
        windowManagerDrawViewParams.y = 0;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            windowManagerDrawViewParams.layoutInDisplayCutoutMode =
                    WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        windowManager.addView(drawView, windowManagerDrawViewParams);
    }

    static LinearLayout container_features;

    private int getLayoutType() {
        if (Build.VERSION.SDK_INT >= 26) { return 2038; }
        if (Build.VERSION.SDK_INT >= 24) { return 2002; }
        if (Build.VERSION.SDK_INT >= 23) { return 2005; }
        return 2003;
    }

    private int convertDP(int value) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, value, context.getResources().getDisplayMetrics());
    }

    private static int convertDPStatic(int value) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, value, context.getResources().getDisplayMetrics());
    }

    public Floater(Context globContext) {
        PrimaryColor = getObfuscatedColor(200);
        context = globContext.getApplicationContext();
        activeWindow = this;
        utils = new Utils(context);
        onCreate();
    }

    public void onCreate() {
        onCreateSystemWindow();
        onCreateTemplate();
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    public void onCreateTemplate() {
        tabPages.clear();
        tabButtons.clear();
        currentTabIndex = 0;

        rootFrame = new FrameLayout(context);
        rootFrame.setLayoutParams(new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));

        rootRelative = new RelativeLayout(context);
        rootRelative.setLayoutParams(new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));

        collapsedRelative = new RelativeLayout(context);
        collapsedRelative.setLayoutParams(new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));
        collapsedRelative.setAlpha(0.9f);
        collapsedRelative.setVisibility(View.VISIBLE);

        logoImagem = new ImageView(context);
        logoImagem.setLayoutParams(new LinearLayout.LayoutParams(convertDP(58),convertDP(58)));
        logoImagem.setImageResource(com.microsoft.xboxone.smartglass.R.drawable.storm_logo);
        logoImagem.setContentDescription("Open STORM MODS floating menu");

        menuLayout = new LinearLayout(context);
        menuLayout.setLayoutParams(new LinearLayout.LayoutParams(
                Math.min(convertDP(350), context.getResources().getDisplayMetrics().widthPixels-convertDP(16)),
                ViewGroup.LayoutParams.WRAP_CONTENT));
        menuLayout.setVisibility(View.GONE);
        menuLayout.setAlpha(0.95f);
        menuLayout.setOrientation(LinearLayout.VERTICAL);
        menuLayout.setScaleX(0f);
        menuLayout.setScaleY(0f);
        menuLayout.setAlpha(0f);

        GradientDrawable menuBackground = new GradientDrawable();
        menuBackground.setShape(GradientDrawable.RECTANGLE);
        menuBackground.setColor(getObfuscatedColor(201));
        menuBackground.setCornerRadius(convertDP(18));
        menuBackground.setStroke(convertDP(1), 0xFF28364D);
        menuLayout.setPadding(convertDP(8),convertDP(6),convertDP(8),convertDP(6));
        menuLayout.setBackground(menuBackground);

        tabCima = new LinearLayout(context);
        tabCima.setLayoutParams(new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, convertDP(40)));

        tabCimaRelative = new RelativeLayout(context);
        tabCimaRelative.setLayoutParams(new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, convertDP(40)));
        tabCimaRelative.setGravity(Gravity.LEFT);

        tituloText = new TextView(context);
        tituloText.setLayoutParams(new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, convertDP(40)));
        tituloText.setText(getObfuscatedString(100));
        tituloText.setTextColor(PrimaryColor);
        tituloText.setTypeface(null, Typeface.BOLD);
        tituloText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 18);
        tituloText.setGravity(Gravity.CENTER);
        tituloText.setLetterSpacing(0.07f);
        tituloText.setOnTouchListener(createOnTouchListener());

        LinearLayout middleBody = new LinearLayout(context);
        middleBody.setLayoutParams(new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, Math.min(convertDP(256), Math.max(convertDP(120), context.getResources().getDisplayMetrics().heightPixels-convertDP(150)))));
        middleBody.setOrientation(LinearLayout.VERTICAL);

        HorizontalScrollView tabScroll = new HorizontalScrollView(context);
        tabScroll.setHorizontalScrollBarEnabled(false);
        tabScroll.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, convertDP(48)));

        tabContainer = new LinearLayout(context);
        tabContainer.setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        tabContainer.setOrientation(LinearLayout.HORIZONTAL);
        tabContainer.setPadding(convertDP(5), convertDP(5), convertDP(5), convertDP(5));
        tabScroll.addView(tabContainer);

        View divider = new View(context);
        divider.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,convertDP(1)));
        divider.setBackgroundColor(getObfuscatedColor(202));

        ScrollView contentScroll = new ScrollView(context);
        contentScroll.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,0,1f));
        contentScroll.setVerticalScrollBarEnabled(false);

        pagesContainer = new FrameLayout(context);
        pagesContainer.setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        contentScroll.addView(pagesContainer);

        middleBody.addView(tabScroll);
        middleBody.addView(divider);
        middleBody.addView(contentScroll);
        tabBaixo = new LinearLayout(context);
        tabBaixo.setLayoutParams(new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, convertDP(40)));

        tabBaixoRelative = new RelativeLayout(context);
        tabBaixoRelative.setLayoutParams(new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, convertDP(40)));
        tabBaixoRelative.setGravity(Gravity.CENTER);

        GradientDrawable closeButtonBackground = new GradientDrawable();
        closeButtonBackground.setShape(GradientDrawable.RECTANGLE);
        closeButtonBackground.setColor(getObfuscatedColor(216));
        closeButtonBackground.setStroke(convertDP(1), PrimaryColor);
        closeButtonBackground.setCornerRadius(convertDP(6));

        closeButton = new Button(context);
        closeButton.setLayoutParams(new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, convertDP(40)));
        closeButton.setText(getObfuscatedString(101));
        closeButton.setTextColor(getObfuscatedColor(214));
        closeButton.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12);
        closeButton.setAllCaps(false);
        closeButton.setTypeface(null, Typeface.BOLD);
        closeButton.setPadding(0, 0, 0, 0);
        closeButton.setBackground(closeButtonBackground);
        closeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                collapseMenu();
            }
        });

        tabBaixoRelative.addView(closeButton);
        tabBaixo.addView(tabBaixoRelative);
        rootFrame.addView(rootRelative);
        rootRelative.addView(collapsedRelative);
        collapsedRelative.addView(logoImagem);
        rootRelative.addView(menuLayout);

        menuLayout.addView(tabCima);
        tabCima.addView(tabCimaRelative);
        tabCimaRelative.addView(tituloText);

        menuLayout.addView(middleBody);

        menuLayout.addView(tabBaixo);

        windowManager.addView(rootFrame, windowManagerParams);

        // Removed rootFrame.setOnTouchListener to prevent full-screen touch swallowing
        logoImagem.setOnTouchListener(createOnTouchListener());
        logoImagem.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                expandMenu();
            }
        });

        Functions();
        addDisplaySettings();
        Handler handler = new Handler(Looper.getMainLooper());
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                ftKZXivSr(context);
            }
        }, 5000);
    }
    private void addDisplaySettings(){
        addTab("Display");
        LinearLayout page=container_features;
        TextView hint=StormTheme.text(context,"Your floating workspace",14,0xFFF3F7FF,true);hint.setPadding(convertDP(10),convertDP(10),0,convertDP(8));page.addView(hint);
        TextView opacity=StormTheme.text(context,"Panel opacity",12,0xFF9BAAC2,false);opacity.setPadding(convertDP(10),0,0,0);page.addView(opacity);
        SeekBar alpha=new SeekBar(context);alpha.setMax(30);alpha.setProgress(context.getSharedPreferences("STORM_UI",0).getInt("opacity",25));alpha.setProgressTintList(android.content.res.ColorStateList.valueOf(PrimaryColor));
        page.addView(alpha,new LinearLayout.LayoutParams(-1,convertDP(44)));
        alpha.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener(){public void onProgressChanged(SeekBar b,int v,boolean user){if(user){context.getSharedPreferences("STORM_UI",0).edit().putInt("opacity",v).apply();menuLayout.setAlpha((70+v)/100f);}}public void onStartTrackingTouch(SeekBar b){}public void onStopTrackingTouch(SeekBar b){}});
        Button reset=StormTheme.button(context,"Reset panel position",false);page.addView(reset,new LinearLayout.LayoutParams(-1,convertDP(44)));reset.setOnClickListener(v->{windowManagerParams.x=0;windowManagerParams.y=convertDP(60);windowManager.updateViewLayout(rootFrame,windowManagerParams);});
        TextView foot=StormTheme.text(context,"Drag the title to move · Minimize to return to the logo",11,0xFF9BAAC2,false);foot.setPadding(convertDP(10),convertDP(12),convertDP(10),convertDP(12));page.addView(foot);
    }
    private float panelOpacity(){return (70+context.getSharedPreferences("STORM_UI",0).getInt("opacity",25))/100f;}
    private void expandMenu() {
        if (isMenuExpanded) return;
        if(panelAnimation!=null){panelAnimation.removeAllListeners();panelAnimation.cancel();}
        isMenuExpanded = true;
        menuLayout.setVisibility(View.VISIBLE);
        menuLayout.bringToFront();

        AnimatorSet set = new AnimatorSet();
        panelAnimation = set;
        set.playTogether(
                ObjectAnimator.ofFloat(menuLayout, View.SCALE_X, 0.92f, 1f),
                ObjectAnimator.ofFloat(menuLayout, View.SCALE_Y, 0.92f, 1f),
                ObjectAnimator.ofFloat(menuLayout, View.ALPHA, 0f, panelOpacity())
        );
        set.setDuration(180);
        set.setInterpolator(fastOutSlowIn);
        set.start();

        ObjectAnimator.ofFloat(collapsedRelative, View.ALPHA, 0.9f, 0.3f).setDuration(200).start();
    }

    private void collapseMenu() {
        if (!isMenuExpanded) return;
        if(panelAnimation!=null){panelAnimation.removeAllListeners();panelAnimation.cancel();}
        isMenuExpanded = false;

        AnimatorSet set = new AnimatorSet();
        panelAnimation = set;
        set.playTogether(
                ObjectAnimator.ofFloat(menuLayout, View.SCALE_X, 1f, 0.92f),
                ObjectAnimator.ofFloat(menuLayout, View.SCALE_Y, 1f, 0.92f),
                ObjectAnimator.ofFloat(menuLayout, View.ALPHA, menuLayout.getAlpha(), 0f)
        );
        set.setDuration(180);
        set.setInterpolator(fastOutSlowIn);
        set.addListener(new AnimatorListenerAdapter() {
            @Override
            public void onAnimationEnd(Animator animation) {
                menuLayout.setVisibility(View.GONE);
            }
        });
        set.start();

        ObjectAnimator.ofFloat(collapsedRelative, View.ALPHA, 0.3f, 0.9f).setDuration(200).start();
    }

    public static void addTab(final String name) {
        final LinearLayout page = new LinearLayout(context);
        page.setOrientation(LinearLayout.VERTICAL);
        page.setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        page.setPadding(0, convertDPStatic(5), 0, convertDPStatic(5));

        boolean isFirst = tabPages.isEmpty();
        page.setVisibility(isFirst ? View.VISIBLE : View.GONE);
        page.setAlpha(isFirst ? 1f : 0f);
        page.setTranslationY(isFirst ? 0f : 80f);

        container_features = page;
        tabPages.add(page);
        if (pagesContainer != null) pagesContainer.addView(page);

        final LinearLayout tabBtn = new LinearLayout(context);
        tabBtn.setOrientation(LinearLayout.HORIZONTAL);

        LinearLayout.LayoutParams btnParams = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, convertDPStatic(36));
        btnParams.setMargins(0,0,convertDPStatic(6),0);
        tabBtn.setLayoutParams(btnParams);
        tabBtn.setGravity(Gravity.CENTER_VERTICAL);
        tabBtn.setPadding(convertDPStatic(12),0,convertDPStatic(12),0);

        final TextView tabTxt = new TextView(context);
        tabTxt.setText(name);
        tabTxt.setTextColor(isFirst ? getObfuscatedColor(214) : getObfuscatedColor(203));
        tabTxt.setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f);
        tabTxt.setTypeface(null, Typeface.BOLD);
        LinearLayout.LayoutParams txtParams = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        tabTxt.setLayoutParams(txtParams);

        final View rightLine = new View(context);
        LinearLayout.LayoutParams lineParams = new LinearLayout.LayoutParams(convertDPStatic(3), convertDPStatic(18));
        rightLine.setLayoutParams(lineParams);
        GradientDrawable lineBg = new GradientDrawable();
        lineBg.setShape(GradientDrawable.RECTANGLE);
        lineBg.setCornerRadius(convertDPStatic(2));
        lineBg.setColor(getObfuscatedColor(214));
        rightLine.setBackground(lineBg);
        rightLine.setVisibility(isFirst ? View.VISIBLE : View.INVISIBLE);

        if (isFirst) {
            GradientDrawable bg = new GradientDrawable();
            bg.setShape(GradientDrawable.RECTANGLE);
            bg.setCornerRadius(convertDPStatic(5));
            bg.setColor(getObfuscatedColor(204));
            tabBtn.setBackground(bg);
        } else {
            tabBtn.setBackground(null);
        }

        tabBtn.addView(tabTxt);
        tabBtn.addView(rightLine);

        final int tabIndex = tabPages.size() - 1;

        tabBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                switchTab(tabIndex);
            }
        });

        tabButtons.add(tabBtn);
        if (tabContainer != null) tabContainer.addView(tabBtn);

        if (!isFirst) {
            page.setAlpha(0f);
            page.setTranslationY(80f);
            page.animate()
                    .alpha(1f)
                    .translationY(0f)
                    .setDuration(180)
                    .setInterpolator(new DecelerateInterpolator())
                    .start();
        }
    }
    private static void switchTab(int newIndex) {
        if(newIndex<0||newIndex>=tabPages.size())return;
        currentTabIndex=newIndex;
        for(int i=0;i<tabPages.size();i++){
            View page=tabPages.get(i);page.animate().cancel();
            page.setVisibility(i==newIndex?View.VISIBLE:View.GONE);
            page.setAlpha(1f);page.setTranslationY(0f);
        }
        StormTheme.enter(tabPages.get(newIndex));

        for (int i = 0; i < tabPages.size(); i++) {
            boolean isActive = (i == newIndex);
            LinearLayout btn = tabButtons.get(i);
            TextView txt = (TextView) btn.getChildAt(0);
            View line = btn.getChildAt(1);

            txt.setTextColor(isActive ? getObfuscatedColor(214) : getObfuscatedColor(203));
            line.setVisibility(isActive ? View.VISIBLE : View.INVISIBLE);

            if (isActive) {
                GradientDrawable bg = new GradientDrawable();
                bg.setShape(GradientDrawable.RECTANGLE);
                bg.setCornerRadius(convertDPStatic(5));
                bg.setColor(getObfuscatedColor(204));
                btn.setScaleX(0.9f);
                btn.setScaleY(0.9f);
                btn.setBackground(bg);
                btn.animate()
                        .scaleX(1f)
                        .scaleY(1f)
                        .setDuration(200)
                        .setInterpolator(new DecelerateInterpolator())
                        .start();
            } else {
                btn.setBackground(null);
                btn.setScaleX(1f);
                btn.setScaleY(1f);
            }
        }
    }
    @SuppressLint("ClickableViewAccessibility")
    public void onCreateSystemWindow() {
        int LAYOUT_FLAG;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            LAYOUT_FLAG = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
        } else {
            LAYOUT_FLAG = WindowManager.LayoutParams.TYPE_PHONE;
        }

        windowManagerParams = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.WRAP_CONTENT,
                LAYOUT_FLAG,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL,
                PixelFormat.TRANSLUCENT);
        windowManagerParams.gravity = Gravity.TOP | Gravity.LEFT;
        windowManagerParams.x = 0;
        windowManagerParams.y = 100;

        windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        DrawCanvas();
    }

    private View.OnTouchListener createOnTouchListener() {
        return new View.OnTouchListener() {
            private int initialX;
            private int initialY;
            private float initialTouchX;
            private float initialTouchY;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        initialX = windowManagerParams.x;
                        initialY = windowManagerParams.y;
                        initialTouchX = event.getRawX();
                        initialTouchY = event.getRawY();
                        return true;

                    case MotionEvent.ACTION_MOVE:
                        windowManagerParams.x = Math.max(0, Math.min(context.getResources().getDisplayMetrics().widthPixels-rootFrame.getWidth(), initialX + (int) (event.getRawX() - initialTouchX)));
                        windowManagerParams.y = Math.max(0, Math.min(context.getResources().getDisplayMetrics().heightPixels-rootFrame.getHeight(), initialY + (int) (event.getRawY() - initialTouchY)));
                        windowManager.updateViewLayout(rootFrame, windowManagerParams);
                        return true;

                    case MotionEvent.ACTION_UP:
                        int xDiff = (int) (event.getRawX() - initialTouchX);
                        int yDiff = (int) (event.getRawY() - initialTouchY);

                        if (Math.abs(xDiff) < 10 && Math.abs(yDiff) < 10) {
                            if (v == logoImagem) {
                                expandMenu();
                            }
                        }
                        return true;
                }
                return false;
            }
        };
    }

    public static void addCategory(String name) { addCategory(name, 22); }
    public static void addCategory(String name, int heightDp) {
        CustomFade textView = new CustomFade(context);
        textView.setText(name);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, convertDPStatic(heightDp));
        params.setMargins(convertDPStatic(5), 0, convertDPStatic(5), 0);
        textView.setLayoutParams(params);
        if(container_features != null) container_features.addView(textView);
    }

    public static void addSwitch(final String name, final int ID) { addSwitch(name, ID, 32); }
    public static void addSwitch(final String name, final int ID, int rowHeightDp) {
        int compactRowHeight = 42;

        LinearLayout row = new LinearLayout(context);
        row.setLayoutParams(new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, convertDPStatic(compactRowHeight)));
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(convertDPStatic(5), 0, convertDPStatic(5), 0);

        TextView label = new TextView(context);
        LinearLayout.LayoutParams textParams = new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f);
        label.setLayoutParams(textParams);
        label.setText(name);
        label.setTextColor(getObfuscatedColor(214));
        label.setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f);
        label.setTypeface(null, Typeface.BOLD);

        CustomCheckbox checkbox = new CustomCheckbox(context);
        LinearLayout.LayoutParams boxParams = new LinearLayout.LayoutParams(convertDPStatic(32), convertDPStatic(16));
        checkbox.setLayoutParams(boxParams);

        checkbox.setOnCheckedChangeListener((view, isChecked) -> { ChangesID(ID, 0); });

        row.addView(label);
        row.addView(checkbox);
        if (container_features != null) container_features.addView(row);
    }

    public static void addButton(final String name, final int ID) {
        Button btn = new Button(context);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, convertDPStatic(35));
        params.setMargins(convertDPStatic(5), convertDPStatic(2), convertDPStatic(5), convertDPStatic(2));
        btn.setLayoutParams(params);
        btn.setText(name);
        btn.setTextColor(getObfuscatedColor(214));
        btn.setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f);
        btn.setTypeface(null, Typeface.BOLD);
        btn.setAllCaps(false);

        GradientDrawable bgShape = new GradientDrawable();
        bgShape.setShape(GradientDrawable.RECTANGLE);
        bgShape.setColor(getObfuscatedColor(205));
        bgShape.setStroke(convertDPStatic(1), PrimaryColor);
        bgShape.setCornerRadius(convertDPStatic(4));
        btn.setBackground(bgShape);

        btn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        v.animate().scaleX(0.95f).scaleY(0.95f).setDuration(100).start();
                        break;
                    case MotionEvent.ACTION_UP:
                    case MotionEvent.ACTION_CANCEL:
                        v.animate().scaleX(1f).scaleY(1f).setDuration(100).start();
                        break;
                }
                return false;
            }
        });

        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (ID == 1000) {
                    if (loginLogText != null) {
                        loginLogText.setText(getObfuscatedString(102));
                        loginLogText.setTextColor(getObfuscatedColor(214));
                    }
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            ChangesID(ID, 1);
                        }
                    }).start();
                } else {
                    ChangesID(ID, 1);
                }
            }
        });

        if (container_features != null) {
            btn.setAlpha(0f);
            btn.setScaleX(0.8f);
            btn.setScaleY(0.8f);
            container_features.addView(btn);
            btn.animate()
                    .alpha(1f)
                    .scaleX(1f)
                    .scaleY(1f)
                    .setDuration(250)
                    .setInterpolator(new DecelerateInterpolator())
                    .start();

            if (ID == 1000) {
                loginLogText = new TextView(context);
                LinearLayout.LayoutParams paramsTxt = new LinearLayout.LayoutParams(
                        LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
                paramsTxt.setMargins(convertDPStatic(5), convertDPStatic(5), convertDPStatic(5), convertDPStatic(5));
                loginLogText.setLayoutParams(paramsTxt);

                loginLogText.setText("");
                loginLogText.setTextColor(getObfuscatedColor(214));
                loginLogText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f);
                loginLogText.setTypeface(null, Typeface.BOLD);
                loginLogText.setGravity(Gravity.CENTER);

                container_features.addView(loginLogText);
                loginLogText.setAlpha(0f);
                loginLogText.animate().alpha(1f).setDuration(180).start();
            }
        }
    }

    public static void addTextBox(final String hint, final int ID) {
        final EditText editText = new EditText(context);
        LinearLayout.LayoutParams paramsTxt = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, convertDPStatic(35));
        paramsTxt.setMargins(convertDPStatic(5), convertDPStatic(5), convertDPStatic(5), convertDPStatic(5));
        editText.setLayoutParams(paramsTxt);
        editText.setHint(hint);
        editText.setHintTextColor(getObfuscatedColor(206));
        editText.setTextColor(getObfuscatedColor(214));
        editText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 12f);
        editText.setSingleLine(true);
        editText.setPadding(convertDPStatic(10), 0, convertDPStatic(10), 0);

        GradientDrawable bgShape = new GradientDrawable();
        bgShape.setShape(GradientDrawable.RECTANGLE);
        bgShape.setColor(getObfuscatedColor(207));
        bgShape.setStroke(convertDPStatic(1), getObfuscatedColor(208));
        bgShape.setCornerRadius(convertDPStatic(4));
        editText.setBackground(bgShape);

        editText.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event.getAction() == MotionEvent.ACTION_UP) {
                    toggleKeyboardFocus(true, editText);
                }
                return false;
            }
        });

        editText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                ChangesString(ID, s.toString());
            }

            @Override
            public void afterTextChanged(Editable s) {}
        });

        if (container_features != null) {
            container_features.addView(editText);
            editText.setAlpha(0f);
            editText.animate().alpha(1f).setDuration(180).start();
        }
    }
    public static void clearMenuLayout() {
        toggleKeyboardFocus(false, null);
        if (container_features != null) {
            container_features.removeAllViews();
        }

        tabPages.clear();
        tabButtons.clear();
        currentTabIndex = 0;
        if (tabContainer != null) {
            tabContainer.removeAllViews();
        }
        if (pagesContainer != null) {
            pagesContainer.removeAllViews();
        }
    }
    public static void addSeekBar(final String name, int value, int max, final String type, final int ID) {
        LinearLayout container = new LinearLayout(context);
        container.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        container.setPadding(convertDPStatic(5), convertDPStatic(4), convertDPStatic(5), convertDPStatic(4));
        container.setOrientation(LinearLayout.VERTICAL);

        RelativeLayout textLayout = new RelativeLayout(context);
        textLayout.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        TextView titleText = new TextView(context);
        titleText.setText(name);
        titleText.setTextColor(getObfuscatedColor(214));
        titleText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f);
        titleText.setTypeface(null, Typeface.BOLD);

        TextView valueText = new TextView(context);
        valueText.setTextColor(getObfuscatedColor(209));
        valueText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 10f);
        valueText.setTypeface(null, Typeface.BOLD);
        RelativeLayout.LayoutParams valParams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        valParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        valueText.setLayoutParams(valParams);
        updateSliderText(valueText, value, type);

        textLayout.addView(titleText);
        textLayout.addView(valueText);

        SeekBar seekBar = new SeekBar(context);
        LinearLayout.LayoutParams sliderParams = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, convertDPStatic(36));
        sliderParams.setMargins(0, convertDPStatic(5), 0, 0);
        seekBar.setLayoutParams(sliderParams);
        seekBar.setPadding(0, 0, 0, 0);
        seekBar.setThumbTintList(android.content.res.ColorStateList.valueOf(PrimaryColor));

        GradientDrawable bgShape = new GradientDrawable();
        bgShape.setShape(GradientDrawable.RECTANGLE);
        bgShape.setColor(getObfuscatedColor(207));
        bgShape.setCornerRadius(convertDPStatic(3));

        GradientDrawable progressShape = new GradientDrawable(
                GradientDrawable.Orientation.LEFT_RIGHT,
                new int[]{getObfuscatedColor(210), getObfuscatedColor(211)}
        );
        progressShape.setCornerRadius(convertDPStatic(3));

        ClipDrawable progressClip = new ClipDrawable(progressShape, Gravity.LEFT, ClipDrawable.HORIZONTAL);
        Drawable[] layersList = { bgShape, progressClip };
        LayerDrawable layers = new LayerDrawable(layersList);
        layers.setId(0, android.R.id.background);
        layers.setId(1, android.R.id.progress);

        seekBar.setProgressDrawable(layers);
        seekBar.setMax(max);
        if (type.equals("Color")) seekBar.setMax(9);
        else if (type.equals("BoxType") || type.equals("LineType")) seekBar.setMax(2);
        seekBar.setProgress(value);

        // Scalare doar pe slider (nu și pe text)
        seekBar.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        v.animate().scaleY(1.3f).setDuration(150).start();
                        break;
                    case MotionEvent.ACTION_UP:
                    case MotionEvent.ACTION_CANCEL:
                        v.animate().scaleY(1f).setDuration(150).start();
                        break;
                }
                return false;
            }
        });

        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    updateSliderText(valueText, progress, type);
                    ChangesID(ID, progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        container.addView(textLayout);
        container.addView(seekBar);

        if (container_features != null) {
            container_features.addView(container);
            container.setAlpha(0f);
            container.animate().alpha(1f).setDuration(180).start();
        }
    }

    private static void updateSliderText(TextView tv, int i, String type) {
        if (type.equals("Color")) {
            String colorName = getObfuscatedString(120 + i);
            String hexCode = getObfuscatedString(130 + i);
            tv.setText(Html.fromHtml("<font color='" + hexCode + "'>" + colorName + "</font>"));
        } else if (type.equals("BoxType")) {
            tv.setText(getObfuscatedString(140 + i));
        } else if (type.equals("LineType")) {
            tv.setText(getObfuscatedString(150 + i));
        } else {
            tv.setText(String.valueOf(i) + type);
        }
    }

    public static void Funçoes(String pkg, String lib, String offset, String hex) {
        Message message = Message.obtain(null, RootServices.MSG_GETINFO);
        message.getData().putString("pkg", pkg);
        message.getData().putString("fileSo", lib);
        message.getData().putInt("offset", Integer.decode(offset).intValue());
        message.getData().putString("hexNumber", hex);
        message.replyTo = myMessenger;
        try {
            remoteMessenger.send(message);
        } catch (RemoteException e) {}
    }

    public static class MSGConnection implements ServiceConnection {
        public void onServiceConnected(ComponentName name, IBinder service) {
            setRemoteMessenger(new Messenger(service));
            if (serviceTestQueued) serviceTestQueued = false;
        }
        public void onServiceDisconnected(ComponentName name) {
            setRemoteMessenger((Messenger) null);
        }
    }

    public static final void setRemoteMessenger(Messenger messenger) {
        remoteMessenger = messenger;
    }

    public static void addLogMsg(final String msg) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                if (loginLogText != null) {
                    loginLogText.setText(msg);
                }
            }
        });
    }

    private static java.util.HashMap<Integer, View> shortcutButtons = new java.util.HashMap<>();

    public static void manageShortcutButton(final int id, final String name, final int state) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);

                if (state == 0) {
                    if (shortcutButtons.containsKey(id)) {
                        View container = shortcutButtons.get(id);
                        try {
                            wm.removeView(container);
                        } catch (Exception e) {}
                        shortcutButtons.remove(id);
                    }
                    return;
                }
                if (shortcutButtons.containsKey(id)) {
                    LinearLayout container = (LinearLayout) shortcutButtons.get(id);
                    CustomCheckbox checkbox = (CustomCheckbox) container.getChildAt(1);
                    if (checkbox != null) {
                        checkbox.setChecked(state == 1);
                    }
                    return;
                }
                final LinearLayout container = new LinearLayout(context);
                container.setOrientation(LinearLayout.VERTICAL);
                container.setGravity(Gravity.CENTER);
                container.setPadding(convertDPStatic(8), convertDPStatic(6), convertDPStatic(8), convertDPStatic(6));
                container.setAlpha(0.95f);
                GradientDrawable gd = new GradientDrawable();
                gd.setShape(GradientDrawable.RECTANGLE);
                gd.setColor(getObfuscatedColor(201));
                gd.setCornerRadius(convertDPStatic(6));
                gd.setStroke(convertDPStatic(1), getObfuscatedColor(211));
                container.setBackground(gd);

                TextView label = new TextView(context);
                LinearLayout.LayoutParams textParams = new LinearLayout.LayoutParams(
                        LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
                textParams.bottomMargin = convertDPStatic(4);
                label.setLayoutParams(textParams);
                label.setText(name);
                label.setTextColor(getObfuscatedColor(214));
                label.setTextSize(TypedValue.COMPLEX_UNIT_SP, 11f);
                label.setTypeface(null, Typeface.BOLD);
                label.setGravity(Gravity.CENTER);
                container.addView(label);

                CustomCheckbox checkbox = new CustomCheckbox(context);
                LinearLayout.LayoutParams boxParams = new LinearLayout.LayoutParams(convertDPStatic(32), convertDPStatic(16));
                checkbox.setLayoutParams(boxParams);
                checkbox.setChecked(state == 1);
                container.addView(checkbox);
                int LAYOUT_FLAG = (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) ?
                        WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY : WindowManager.LayoutParams.TYPE_PHONE;

                final WindowManager.LayoutParams params = new WindowManager.LayoutParams(
                        convertDPStatic(85),
                        convertDPStatic(52),
                        LAYOUT_FLAG,
                        WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL,
                        PixelFormat.TRANSLUCENT);

                params.gravity = Gravity.TOP | Gravity.LEFT;
                params.x = 100;
                params.y = 200;
                View.OnTouchListener widgetTouchListener = new View.OnTouchListener() {
                    private int initialX;
                    private int initialY;
                    private float initialTouchX;
                    private float initialTouchY;
                    private boolean isClick = true;

                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        switch (event.getAction()) {
                            case MotionEvent.ACTION_DOWN:
                                initialX = params.x;
                                initialY = params.y;
                                initialTouchX = event.getRawX();
                                initialTouchY = event.getRawY();
                                isClick = true;
                                return true;

                            case MotionEvent.ACTION_MOVE:
                                int dX = (int) (event.getRawX() - initialTouchX);
                                int dY = (int) (event.getRawY() - initialTouchY);
                                if (Math.abs(dX) > 10 || Math.abs(dY) > 10) {
                                    isClick = false;
                                }
                                params.x = initialX + dX;
                                params.y = initialY + dY;
                                wm.updateViewLayout(container, params);
                                return true;

                            case MotionEvent.ACTION_UP:
                                if (isClick) {
                                    ChangesID(id, 0);
                                }
                                return true;
                        }
                        return false;
                    }
                };
                container.setOnTouchListener(widgetTouchListener);
                checkbox.setOnTouchListener(widgetTouchListener);
                wm.addView(container, params);
                shortcutButtons.put(id, container);

                container.setScaleX(0f);
                container.setScaleY(0f);
                container.setAlpha(0f);
                container.animate()
                        .scaleX(1f)
                        .scaleY(1f)
                        .alpha(1f)
                        .setDuration(180)
                        .setInterpolator(new DecelerateInterpolator())
                        .start();
            }
        });
    }

    public static void loginSuccessFromNative() {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                if (loginLogText != null) {
                    loginLogText.setText(getObfuscatedString(103));
                    loginLogText.setTextColor(getObfuscatedColor(212));
                }

                new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        clearMenuLayout();
                        Functions();
                    }
                }, 600);
            }
        });
    }

    public static void loginFailedFromNative(final String msg) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                if (loginLogText != null) {
                    loginLogText.setText(msg);
                    loginLogText.setTextColor(getObfuscatedColor(213));
                }
            }
        });
    }
}
