#include <src/Includes/http.h>
#include "Category.h"
#include "Switch.h"
#include "SeekBar.h"
#include "Images.h"

class Widget {

private:
    JNIEnv* env;
    Category category;
    Switch aSwitch;
    SeekBar seekBar;

public:
    Widget(JNIEnv* globEnv) {
        env = globEnv;
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


};
