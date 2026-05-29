#include <X11/keysym.h>
extern "C" {
    #include "../../lib/tinyPTC/src/tinyptc.h"
}
#include "input.hpp"

namespace ECS {

    static InputState_t g_inputState;

    static void onKeyPress(KeySym ks) {
        switch(ks) {
            case XK_Left:    g_inputState.left    = true; break;
            case XK_Right:   g_inputState.right   = true; break;
            case XK_Up:      g_inputState.up      = true; break;
            case XK_Down:    g_inputState.down    = true; break;
            case XK_space:   g_inputState.space   = true; break;
            case XK_r:
            case XK_R:       g_inputState.restart = true; break;
            case XK_a:       g_inputState.left    = true; break;
            case XK_d:       g_inputState.right   = true; break;
            case XK_w:       g_inputState.up      = true; break;
            case XK_s:       g_inputState.down    = true; break;
            case XK_f:
            case XK_F:       g_inputState.attack  = true; break;
        }
    }

    static void onKeyRelease(KeySym ks) {
        switch(ks) {
            case XK_Left:    g_inputState.left    = false; break;
            case XK_Right:   g_inputState.right   = false; break;
            case XK_Up:      g_inputState.up      = false; break;
            case XK_Down:    g_inputState.down    = false; break;
            case XK_space:   g_inputState.space   = false; break;
            case XK_r:
            case XK_R:       g_inputState.restart = false; break;
            case XK_a:       g_inputState.left    = false; break;
            case XK_d:       g_inputState.right   = false; break;
            case XK_w:       g_inputState.up      = false; break;
            case XK_s:       g_inputState.down    = false; break;
            case XK_f:
            case XK_F:       g_inputState.attack  = false; break;
        }
    }

    void initInput() {
        ptc_set_on_keypress(onKeyPress);
        ptc_set_on_keyrelease(onKeyRelease);
    }

    InputState_t& getInputState() {
        return g_inputState;
    }

}
