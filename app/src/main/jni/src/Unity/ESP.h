#ifndef ESP_H
#define ESP_H

#include <jni.h>
#include <algorithm>  // Added for std::min
#include "Color.hpp"
#include "Rect.hpp"

class ESP {
private:
    JNIEnv *_env;
    jobject _cvsView;
    jobject _cvs;

public:
    ESP() {
        _env = nullptr;
        _cvsView = nullptr;
        _cvs = nullptr;
    }

    ESP(JNIEnv *env, jobject cvsView, jobject cvs) {
        this->_env = env;
        this->_cvsView = cvsView;
        this->_cvs = cvs;
    }

    bool isValid() const {
        return (_env != nullptr && _cvsView != nullptr && _cvs != nullptr);
    }

    int getWidth() const {
        if (isValid()) {
            jclass canvas = _env->GetObjectClass(_cvs);
            jmethodID width = _env->GetMethodID(canvas, ("getWidth"), ("()I"));
            return _env->CallIntMethod(_cvs, width);
        }
        return 0;
    }

    int getHeight() const {
        if (isValid()) {
            jclass canvas = _env->GetObjectClass(_cvs);
            jmethodID width = _env->GetMethodID(canvas, ("getHeight"), ("()I"));
            return _env->CallIntMethod(_cvs, width);
        }
        return 0;
    }

    void DrawLine(Color color, float thickness, Vector3 start, Vector3 end) {
        if (isValid()) {
            jclass canvasView = _env->GetObjectClass(_cvsView);
            jmethodID drawline = _env->GetMethodID(canvasView, ("DrawLine"),
                                                   ("(Landroid/graphics/Canvas;IIIIFFFFF)V"));
            _env->CallVoidMethod(_cvsView, drawline, _cvs, (int) color.a, (int) color.r,
                                 (int) color.g, (int) color.b,
                                 thickness,
                                 start.X, start.Y, end.X, end.Y);
        }
    }

    void DrawText(Color color, const char *txt, Vector2 pos, float size) {
        if (isValid()) {
            jclass canvasView = _env->GetObjectClass(_cvsView);
            jmethodID drawtext = _env->GetMethodID(canvasView, ("DrawText"),
                                                   ("(Landroid/graphics/Canvas;IIIILjava/lang/String;FFF)V"));
            _env->CallVoidMethod(_cvsView, drawtext, _cvs, (int) color.a, (int) color.r,
                                 (int) color.g, (int) color.b,
                                 _env->NewStringUTF(txt), pos.X, pos.Y, size);
        }
    }

    void DrawText2(Color color, float stroke, const char *txt, Vector2 pos, float size) {
        if (isValid()) {
            jclass canvasView = _env->GetObjectClass(_cvsView);
            jmethodID drawtext = _env->GetMethodID(canvasView, ("DrawText"),
                                                   ("(Landroid/graphics/Canvas;IIIILjava/lang/String;FFF)V"));
            _env->CallVoidMethod(_cvsView, drawtext, _cvs, (int) color.a, (int) color.r,
                                 (int) color.g, (int) color.b,
                                 _env->NewStringUTF(txt), pos.X, pos.Y, size);
        }
    }

    void DrawName(Color color, const char *txt, Vector3 pos, float size) {
        if (isValid()) {
            jclass canvasView = _env->GetObjectClass(_cvsView);
            jmethodID drawtext = _env->GetMethodID(canvasView, ("DrawName"),
                                                   ("(Landroid/graphics/Canvas;IIIILjava/lang/String;IFFF)V"));
            jstring s=_env->NewStringUTF(txt);
            _env->CallVoidMethod(_cvsView, drawtext, _cvs, (int) color.a, (int) color.r,
                                 (int) color.g, (int) color.b,
                                 s, pos.X, pos.Y, size);
            _env->DeleteLocalRef(s);
        }
    }

    void DrawFilledCircle(Color color, Vector2 pos, float radius) {
        if (isValid()) {
            jclass canvasView = _env->GetObjectClass(_cvsView);
            jmethodID drawfilledcircle = _env->GetMethodID(canvasView, ("DrawFilledCircle"),
                                                           ("(Landroid/graphics/Canvas;IIIIFFF)V"));
            _env->CallVoidMethod(_cvsView, drawfilledcircle, _cvs, (int) color.a, (int) color.r,
                                 (int) color.g, (int) color.b, pos.X, pos.Y, radius);
        }
    }

    void DrawCircle(Color color, float stroke, Vector2 pos, float radius) {
        if (isValid()) {
            jclass canvasView = _env->GetObjectClass(_cvsView);
            jmethodID drawcircle = _env->GetMethodID(canvasView, ("DrawCircle"),
                                                     ("(Landroid/graphics/Canvas;IIIIFFFF)V"));
            _env->CallVoidMethod(_cvsView, drawcircle, _cvs, (int) color.a, (int) color.r,
                                 (int) color.g, (int) color.b,stroke, pos.X, pos.Y, radius);
        }
    }

    void DrawFilledRect(Color color, Vector3 pos, float width, float height) {
        if (isValid()) {
            jclass canvasView = _env->GetObjectClass(_cvsView);
            jmethodID drawfilledrect = _env->GetMethodID(canvasView, "DrawFilledRect",
                                                         "(Landroid/graphics/Canvas;IIIIFFFF)V");
            _env->CallVoidMethod(_cvsView, drawfilledrect, _cvs, (int) color.a, (int) color.r,
                                 (int) color.g, (int) color.b, pos.X, pos.Y, width, height);
        }
    }

    void DrawFilledRect(Rect rect, Color color) {
        DrawFilledRect(color, Vector3(rect.x, rect.y, 0), rect.width, rect.height);
    }

    void DrawSkeleton(Color color, float thickness, float boxWidth, float boxHeight, int screenWidth, int screenHeight, Vector3 headP, Vector3 NeckP, Vector3 bodyP, Vector3 pernaEP, Vector3 pernaDP, Vector3 ombroD, Vector3 ombroE, Vector3 DedoS, Vector3 maoDP) {
        auto DrawCircleAt = [&](Vector3 p, float r) { DrawCircle(color, thickness, Vector2(p.X, screenHeight - p.Y), r); };
        auto DrawLineBetween = [&](Vector3 p1, Vector3 p2) { DrawLine(color, thickness, Vector3(screenWidth - (screenWidth - p1.X), screenHeight - p1.Y), Vector3(screenWidth - (screenWidth - p2.X), screenHeight - p2.Y)); };

        DrawCircleAt(headP, 5.0f);
        DrawLineBetween(bodyP, NeckP);
        DrawLineBetween(ombroD, NeckP);
        DrawLineBetween(ombroE, NeckP);
        DrawLineBetween(pernaEP, bodyP);
        DrawLineBetween(pernaDP, bodyP);
        DrawLineBetween(DedoS, ombroE);
        DrawLineBetween(maoDP, ombroD);
    }

    void DrawBox(Color color, float stroke, Rect rect) {
        Vector3 v1 = Vector3(rect.x, rect.y);
        Vector3 v2 = Vector3(rect.x + rect.width, rect.y);
        Vector3 v3 = Vector3(rect.x + rect.width, rect.y + rect.height);
        Vector3 v4 = Vector3(rect.x, rect.y + rect.height);

        DrawLine(color, stroke, v1, v2); // LINE UP
        DrawLine(color, stroke, v2, v3); // LINE RIGHT
        DrawLine(color, stroke, v3, v4); // LINE DOWN
        DrawLine(color, stroke, v4, v1); // LINE LEFT
    }

    void DrawBox4Line(int x, int y, int w, int h, Color color, float thickness) {
        int iw = w / 4;
        int ih = h / 4;

        DrawLine(color,thickness,Vector3(x, y),Vector3(x + iw, y));
        DrawLine(color,thickness,Vector3(x + w - iw, y),Vector3(x + w, y));
        DrawLine(color,thickness,Vector3(x, y),Vector3(x, y + ih));
        DrawLine(color,thickness,Vector3(x + w - 1, y),Vector3(x + w - 1, y + ih));

        DrawLine(color,thickness,Vector3(x, y + h),Vector3(x + iw, y + h));
        DrawLine(color,thickness,Vector3(x + w - iw, y + h),Vector3(x + w, y + h));
        DrawLine(color,thickness,Vector3(x, y + h - ih), Vector3(x, y + h));
        DrawLine(color,thickness,Vector3(x + w - 1, y + h - ih), Vector3(x + w - 1, y + h));
    }

    void DrawBox4Line(Rect rect, Color color, float thickness) {
        DrawBox4Line(rect.x, rect.y, rect.width, rect.height, color, thickness);
    }

    void DrawVerticalHealthBar(Vector2 screenPos, float height, float maxHealth, float currentHealth, float width = 8.0f, bool showText = false) {
        // Calculate health percentage and bar height
        float healthPercentage = std::min(1.0f, std::max(0.0f, currentHealth / maxHealth));
        float healthHeight = height * healthPercentage;
        
        // Determine color based on health percentage
        Color healthColor;
        if (healthPercentage > 0.6f) {
            healthColor = Color(0, 255, 0, 200);    // Green for high health
        } else if (healthPercentage > 0.3f) {
            healthColor = Color(255, 255, 0, 200);  // Yellow for medium health
        } else {
            healthColor = Color(255, 0, 0, 200);    // Red for low health
        }
        
        // Draw filled health bar (from bottom to top) - NO BORDER
        float healthY = screenPos.Y + (height - healthHeight);
        DrawFilledRect(healthColor, Vector3(screenPos.X, healthY, 0), width, healthHeight);
        
        // Optional: Draw health text next to the bar (only if showText is true)
        if (showText && height > 30) {
            char healthText[16];
            snprintf(healthText, sizeof(healthText), "%.0f", currentHealth);
            
            // Position text to the left of the health bar
            DrawText(Color(255, 255, 255, 200), 
                    healthText, 
                    Vector2(screenPos.X - 25, screenPos.Y + (height / 2) - 6), 
                    10.0f);
        }
    }

    void DrawHorizontalHealthBar(Vector2 screenPos, float width, float maxHealth, float currentHealth) {
        screenPos -= Vector2(0.0f, 5.0f);
        DrawBox(Color(0, 0, 0, 255), 1, Rect(screenPos.X, screenPos.Y, width, 5.0f));
        screenPos += Vector2(1.0f, 1.0f);
        Color clr = Color(0, 255, 0, 255);
        float hpWidth = (currentHealth * (width - 2)) / maxHealth;

        hpWidth = std::min(hpWidth, width);

        if (currentHealth <= (maxHealth * 0.6)) {
            clr = Color(255, 255, 0, 255);
        }
        if (currentHealth < (maxHealth * 0.4)) {
            clr = Color(255, 0, 0, 255);
        }
        DrawBox(clr, 1, Rect(screenPos.X, screenPos.Y, hpWidth, 4.0f));
    }

    // Helper method to create alpha-blended color - FIXED VERSION
    Color AlphaBlend(Color base, Color overlay) {
        float alpha = overlay.a / 255.0f;
        float invAlpha = 1.0f - alpha;
        
        int r = (int)(base.r * invAlpha + overlay.r * alpha);
        int g = (int)(base.g * invAlpha + overlay.g * alpha);
        int b = (int)(base.b * invAlpha + overlay.b * alpha);
        int a = (int)(base.a + overlay.a);
        
        // Clamp values between 0-255
        r = r < 0 ? 0 : (r > 255 ? 255 : r);
        g = g < 0 ? 0 : (g > 255 ? 255 : g);
        b = b < 0 ? 0 : (b > 255 ? 255 : b);
        a = a < 0 ? 0 : (a > 255 ? 255 : a);
        
        return Color(r, g, b, a);
    }
};

#endif