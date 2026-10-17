package com.microsoft.xboxone.smartglass;

import android.content.Context;
import android.content.res.ColorStateList;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.RippleDrawable;
import android.view.View;
import android.widget.*;

final class StormTheme {
    static final int BG=0xFF080D1B, CARD=0xFF111B30, BORDER=0xFF293B5E;
    static final int TEXT=0xFFF3F7FF, MUTED=0xFF9BAAC2, ACCENT=0xFF5DE2FF;
    static int dp(Context c,int n){ return Math.round(n*c.getResources().getDisplayMetrics().density); }
    static GradientDrawable shape(Context c,int color,int radius){
        GradientDrawable d=new GradientDrawable(); d.setColor(color); d.setCornerRadius(dp(c,radius));
        d.setStroke(dp(c,1),BORDER); return d;
    }
    static TextView text(Context c,String value,int size,int color,boolean bold){
        TextView t=new TextView(c); t.setText(value); t.setTextSize(size); t.setTextColor(color);
        t.setFontFeatureSettings("kern"); if(bold)t.setTypeface(Typeface.create("sans-serif-medium",Typeface.NORMAL));
        return t;
    }
    static Button button(Context c,String label,boolean primary){
        Button b=new Button(c); b.setText(label); b.setTextSize(14); b.setAllCaps(false);
        b.setTypeface(Typeface.create("sans-serif-medium",Typeface.NORMAL));
        b.setTextColor(primary?BG:TEXT); b.setMinHeight(dp(c,48));
        GradientDrawable base=shape(c,primary?ACCENT:CARD,16);
        if(primary){base.setColors(new int[]{0xFF5DE2FF,0xFFB0A1FF});base.setOrientation(GradientDrawable.Orientation.LEFT_RIGHT);}
        b.setBackground(new RippleDrawable(ColorStateList.valueOf(0x337FFFFF),base,null));
        return b;
    }
    static void enter(View v){v.setAlpha(0f);v.setTranslationY(12);v.animate().alpha(1f).translationY(0).setDuration(180).start();}
}
