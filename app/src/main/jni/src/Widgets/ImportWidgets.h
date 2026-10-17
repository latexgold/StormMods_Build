#include <src/Includes/http.h>
#include "Category.h"
#include "Switch.h"
#include "SeekBar.h"
#include "Images.h"
#include "Tab.h"
#include "Button.h"
#include "TextBox.h"

class Widget {

private:
    JNIEnv* env;
    Category category;
    Switch aSwitch;
    SeekBar seekBar;
    Tab tab;
    Button button;
    TextBox textBox;

public:
    Widget(JNIEnv* globEnv) {
        env = globEnv;
    }

    void AddTab(const char* name) {
        tab.create(env, name);
    }

    void Category(const char* name) {
        category.create(env, name);
    }

    void Switch(const char* name, jint ID) {
        aSwitch.create(env, name, ID);
    }

    void SeekBar(const char* name, jint value, jint max, const char* type, jint ID) {
        seekBar.create(env, name, value, max, type, ID);
    }

    void ButtonWidget(const char* name, jint ID) {
        button.create(env, name, ID);
    }

    void TextBoxWidget(const char* hint, jint ID) {
        textBox.create(env, hint, ID);
    }


};
