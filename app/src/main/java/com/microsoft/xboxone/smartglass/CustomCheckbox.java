package com.microsoft.xboxone.smartglass;

import android.animation.ArgbEvaluator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.*;
import android.view.View;
import android.view.MotionEvent;

public class CustomCheckbox extends View {
    private boolean isChecked = false;
    private float animPos = 0f; // 0.0 (OFF) la 1.0 (ON)
    private float scale = 1.0f;
    private boolean pressed = false;

    private Paint bgPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private Paint dotPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private ArgbEvaluator colorEval = new ArgbEvaluator();

    private int colorOffBg = Color.parseColor("#090F1D");
    private int colorOnBg = Color.parseColor("#0050FF");
    private int colorOffDot = Color.parseColor("#000A0F");
    private int colorOnDot = Color.WHITE;

    private OnCheckedChangeListener listener;

    public interface OnCheckedChangeListener {
        void onCheckedChanged(View view, boolean isChecked);
    }

    public CustomCheckbox(Context context) {
        super(context);
    }

    public void setOnCheckedChangeListener(OnCheckedChangeListener listener) {
        this.listener = listener;
    }

    private float convertDPStatic(int dp) {
        return dp * getContext().getResources().getDisplayMetrics().density;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        float width = getWidth();
        float height = getHeight();

        int currentBgColor = (int) colorEval.evaluate(animPos, colorOffBg, colorOnBg);
        int currentDotColor = (int) colorEval.evaluate(animPos, colorOffDot, colorOnDot);

        bgPaint.setColor(currentBgColor);
        RectF rectF = new RectF(0, 0, width, height);
        canvas.drawRoundRect(rectF, height / 2, height / 2, bgPaint);
        float edgePadding = convertDPStatic(2);

        float dotRadius = ((height - (edgePadding * 2)) / 2f) * scale;
        float startX = edgePadding + dotRadius;
        float endX = width - edgePadding - dotRadius;
        float currentDotX = startX + (endX - startX) * animPos;

        dotPaint.setColor(currentDotColor);
        canvas.drawCircle(currentDotX, height / 2, dotRadius, dotPaint);

        if (pressed || (isChecked && animPos < 1f) || (!isChecked && animPos > 0f)) {
            invalidate();
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                pressed = true;
                animateScale(0.85f);
                return true;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                pressed = false;
                animateScale(1.0f);
                if (event.getAction() == MotionEvent.ACTION_UP) {
                    isChecked = !isChecked;
                    animateToggle(isChecked);
                    if (listener != null) listener.onCheckedChanged(this, isChecked);
                }
                return true;
        }
        return super.onTouchEvent(event);
    }

    private void animateToggle(boolean target) {
        ValueAnimator anim = ValueAnimator.ofFloat(animPos, target ? 1f : 0f);
        anim.setDuration(150);
        anim.addUpdateListener(animation -> {
            animPos = (float) animation.getAnimatedValue();
            invalidate();
        });
        anim.start();
    }

    private void animateScale(float targetScale) {
        ValueAnimator anim = ValueAnimator.ofFloat(scale, targetScale);
        anim.setDuration(100);
        anim.addUpdateListener(animation -> {
            scale = (float) animation.getAnimatedValue();
            invalidate();
        });
        anim.start();
    }

    public void setChecked(boolean state) {
        if (this.isChecked != state) {
            this.isChecked = state;
            this.animPos = state ? 1f : 0f;
            invalidate();
        }
    }
    public boolean isChecked() {
        return isChecked;
    }
}

