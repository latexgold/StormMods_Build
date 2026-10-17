package com.microsoft.xboxone.smartglass;
import android.content.Context;
import android.graphics.*;
import android.graphics.drawable.Drawable;
final class StormBackdrop extends Drawable {
 private final Paint p=new Paint(Paint.ANTI_ALIAS_FLAG);
 @Override public void draw(Canvas c){
  float w=getBounds().width(),h=getBounds().height();c.drawColor(0xFF080D1B);
  p.setShader(new RadialGradient(w*.95f,h*.1f,Math.max(1,w*.8f),new int[]{0x554F40A2,0x00080D1B},null,Shader.TileMode.CLAMP));c.drawRect(0,0,w,h,p);
  p.setShader(new RadialGradient(0,h*.75f,Math.max(1,w*.75f),new int[]{0x3324B9E3,0x00080D1B},null,Shader.TileMode.CLAMP));c.drawRect(0,0,w,h,p);p.setShader(null);
  p.setColor(0x122D4265);p.setStrokeWidth(1);for(float x=0;x<w;x+=48)c.drawLine(x,0,x,h,p);for(float y=0;y<h;y+=48)c.drawLine(0,y,w,y,p);
 }
 @Override public void setAlpha(int a){} @Override public void setColorFilter(ColorFilter f){} @Override public int getOpacity(){return PixelFormat.OPAQUE;}
}
