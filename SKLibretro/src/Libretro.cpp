#include "Libretro.hpp"

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/input_event_action.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_opengl.h>

#include <filesystem>
#include <fstream>
#include <chrono>

#include "Debug.hpp"

using namespace godot;

namespace SK
{
Libretro* Libretro::s_instance = nullptr;
std::mutex Libretro::s_instance_mutex;

Libretro* Libretro::GetInstance()
{
    std::lock_guard<std::mutex> lock(s_instance_mutex);
    return s_instance;
}

bool Libretro::RegisterInstance(Libretro* instance)
{
    if (!instance)
        return false;

    std::lock_guard<std::mutex> lock(s_instance_mutex);
    s_instance = instance;

    return true;
}

void Libretro::UnregisterInstance()
{
    std::lock_guard<std::mutex> lock(s_instance_mutex);
    s_instance = nullptr;
}

static retro_key GodotToLibretroKeycode(const Ref<InputEventKey>& keyEvent)
{
    switch (keyEvent->get_keycode())
    {
        case KEY_NONE: return RETROK_UNKNOWN;
        case KEY_BACKSPACE: return RETROK_BACKSPACE;
        case KEY_TAB: return RETROK_TAB;
        case KEY_CLEAR: return RETROK_CLEAR;
        case KEY_ENTER: return RETROK_RETURN;
        case KEY_PAUSE: return RETROK_PAUSE;
        case KEY_ESCAPE: return RETROK_ESCAPE;
        case KEY_SPACE: return RETROK_SPACE;
        case KEY_EXCLAM: return RETROK_EXCLAIM;
        case KEY_QUOTEDBL: return RETROK_QUOTEDBL;
        case KEY_NUMBERSIGN: return RETROK_HASH;
        case KEY_DOLLAR: return RETROK_DOLLAR;
        case KEY_AMPERSAND: return RETROK_AMPERSAND;
        case KEY_APOSTROPHE: return RETROK_QUOTE;
        case KEY_PARENLEFT: return RETROK_LEFTPAREN;
        case KEY_PARENRIGHT: return RETROK_RIGHTPAREN;
        case KEY_ASTERISK: return RETROK_ASTERISK;
        case KEY_PLUS: return RETROK_PLUS;
        case KEY_COMMA: return RETROK_COMMA;
        case KEY_MINUS: return RETROK_MINUS;
        case KEY_PERIOD: return RETROK_PERIOD;
        case KEY_SLASH: return RETROK_SLASH;
        case KEY_0: return RETROK_0;
        case KEY_1: return RETROK_1;
        case KEY_2: return RETROK_2;
        case KEY_3: return RETROK_3;
        case KEY_4: return RETROK_4;
        case KEY_5: return RETROK_5;
        case KEY_6: return RETROK_6;
        case KEY_7: return RETROK_7;
        case KEY_8: return RETROK_8;
        case KEY_9: return RETROK_9;
        case KEY_COLON: return RETROK_COLON;
        case KEY_SEMICOLON: return RETROK_SEMICOLON;
        case KEY_LESS: return RETROK_LESS;
        case KEY_EQUAL: return RETROK_EQUALS;
        case KEY_GREATER: return RETROK_GREATER;
        case KEY_QUESTION: return RETROK_QUESTION;
        case KEY_AT: return RETROK_AT;
        case KEY_BRACKETLEFT: return RETROK_LEFTBRACKET;
        case KEY_BACKSLASH: return RETROK_BACKSLASH;
        case KEY_BRACKETRIGHT: return RETROK_RIGHTBRACKET;
        case KEY_ASCIICIRCUM: return RETROK_CARET;
        case KEY_UNDERSCORE: return RETROK_UNDERSCORE;
        case KEY_QUOTELEFT: return RETROK_BACKQUOTE;
        case KEY_A: return RETROK_a;
        case KEY_B: return RETROK_b;
        case KEY_C: return RETROK_c;
        case KEY_D: return RETROK_d;
        case KEY_E: return RETROK_e;
        case KEY_F: return RETROK_f;
        case KEY_G: return RETROK_g;
        case KEY_H: return RETROK_h;
        case KEY_I: return RETROK_i;
        case KEY_J: return RETROK_j;
        case KEY_K: return RETROK_k;
        case KEY_L: return RETROK_l;
        case KEY_M: return RETROK_m;
        case KEY_N: return RETROK_n;
        case KEY_O: return RETROK_o;
        case KEY_P: return RETROK_p;
        case KEY_Q: return RETROK_q;
        case KEY_R: return RETROK_r;
        case KEY_S: return RETROK_s;
        case KEY_T: return RETROK_t;
        case KEY_U: return RETROK_u;
        case KEY_V: return RETROK_v;
        case KEY_W: return RETROK_w;
        case KEY_X: return RETROK_x;
        case KEY_Y: return RETROK_y;
        case KEY_Z: return RETROK_z;
        case KEY_BRACELEFT: return RETROK_LEFTBRACE;
        case KEY_BAR: return RETROK_BAR;
        case KEY_BRACERIGHT: return RETROK_RIGHTBRACE;
        case KEY_ASCIITILDE: return RETROK_TILDE;
        case KEY_DELETE: return RETROK_DELETE;
        case KEY_KP_0: return RETROK_KP0;
        case KEY_KP_1: return RETROK_KP1;
        case KEY_KP_2: return RETROK_KP2;
        case KEY_KP_3: return RETROK_KP3;
        case KEY_KP_4: return RETROK_KP4;
        case KEY_KP_5: return RETROK_KP5;
        case KEY_KP_6: return RETROK_KP6;
        case KEY_KP_7: return RETROK_KP7;
        case KEY_KP_8: return RETROK_KP8;
        case KEY_KP_9: return RETROK_KP9;
        case KEY_KP_PERIOD: return RETROK_KP_PERIOD;
        case KEY_KP_DIVIDE: return RETROK_KP_DIVIDE;
        case KEY_KP_MULTIPLY: return RETROK_KP_MULTIPLY;
        case KEY_KP_SUBTRACT: return RETROK_KP_MINUS;
        case KEY_KP_ADD: return RETROK_KP_PLUS;
        case KEY_KP_ENTER: return RETROK_KP_ENTER;
        // case KEY_KP_EQUALS: return RETROK_KP_EQUALS;
        case KEY_UP: return RETROK_UP;
        case KEY_DOWN: return RETROK_DOWN;
        case KEY_RIGHT: return RETROK_RIGHT;
        case KEY_LEFT: return RETROK_LEFT;
        case KEY_INSERT: return RETROK_INSERT;
        case KEY_HOME: return RETROK_HOME;
        case KEY_END: return RETROK_END;
        case KEY_PAGEUP: return RETROK_PAGEUP;
        case KEY_PAGEDOWN: return RETROK_PAGEDOWN;
        case KEY_F1: return RETROK_F1;
        case KEY_F2: return RETROK_F2;
        case KEY_F3: return RETROK_F3;
        case KEY_F4: return RETROK_F4;
        case KEY_F5: return RETROK_F5;
        case KEY_F6: return RETROK_F6;
        case KEY_F7: return RETROK_F7;
        case KEY_F8: return RETROK_F8;
        case KEY_F9: return RETROK_F9;
        case KEY_F10: return RETROK_F10;
        case KEY_F11: return RETROK_F11;
        case KEY_F12: return RETROK_F12;
        case KEY_F13: return RETROK_F13;
        case KEY_F14: return RETROK_F14;
        case KEY_F15: return RETROK_F15;
        case KEY_NUMLOCK: return RETROK_NUMLOCK;
        case KEY_CAPSLOCK: return RETROK_CAPSLOCK;
        case KEY_SCROLLLOCK: return RETROK_SCROLLOCK;
        case KEY_SHIFT:
        {
            if (keyEvent->get_location() == godot::KEY_LOCATION_LEFT)
                return RETROK_LSHIFT;
            if (keyEvent->get_location() == godot::KEY_LOCATION_RIGHT)
                return RETROK_RSHIFT;
        }
        break;
        case KEY_CTRL:
        {
            if (keyEvent->get_location() == godot::KEY_LOCATION_LEFT)
                return RETROK_LCTRL;
            if (keyEvent->get_location() == godot::KEY_LOCATION_RIGHT)
                return RETROK_RCTRL;
        }
        break;
        case KEY_ALT:
        {
            if (keyEvent->get_location() == godot::KEY_LOCATION_LEFT)
                return RETROK_LALT;
            if (keyEvent->get_location() == godot::KEY_LOCATION_RIGHT)
                return RETROK_RALT;
        }
        break;
        case KEY_META:
        {
            // NOTE: may need to return RETROK_LSUPER/RETK_RSUPER instead for some platforms
            if (keyEvent->get_location() == godot::KEY_LOCATION_LEFT)
                return RETROK_LMETA;
            if (keyEvent->get_location() == godot::KEY_LOCATION_RIGHT)
                return RETROK_RMETA;
        }
        break;
        // case KEY_MODE: return RETROK_MODE;
        // case KEY_COMPOSE: return RETROK_COMPOSE;
        case KEY_HELP: return RETROK_HELP;
        case KEY_PRINT: return RETROK_PRINT;
        case KEY_SYSREQ: return RETROK_SYSREQ;
        // case KEY_BREAK: return RETROK_BREAK;
        case KEY_MENU: return RETROK_MENU;
        // case KEY_POWER: return RETROK_POWER;
        // case KEY_EURO: return RETROK_EURO;
        // case KEY_UNDO: return RETROK_UNDO;
        // case KEY_OEM_102: return RETROK_OEM_102;
        case KEY_BACK: return RETROK_BROWSER_BACK;
        case KEY_FORWARD: return RETROK_BROWSER_FORWARD;
        case KEY_REFRESH: return RETROK_BROWSER_REFRESH;
        case KEY_STOP: return RETROK_BROWSER_STOP;
        case KEY_SEARCH: return RETROK_BROWSER_SEARCH;
        case KEY_FAVORITES: return RETROK_BROWSER_FAVORITES;
        case KEY_HOMEPAGE: return RETROK_BROWSER_HOME;
        case KEY_VOLUMEMUTE: return RETROK_VOLUME_MUTE;
        case KEY_VOLUMEDOWN: return RETROK_VOLUME_DOWN;
        case KEY_VOLUMEUP: return RETROK_VOLUME_UP;
        case KEY_MEDIANEXT: return RETROK_MEDIA_NEXT;
        case KEY_MEDIAPREVIOUS: return RETROK_MEDIA_PREV;
        case KEY_MEDIASTOP: return RETROK_MEDIA_STOP;
        case KEY_MEDIAPLAY: return RETROK_MEDIA_PLAY_PAUSE;
        case KEY_LAUNCHMAIL: return RETROK_LAUNCH_MAIL;
        case KEY_LAUNCHMEDIA: return RETROK_LAUNCH_MEDIA;
        // case KEY_LAUNCH_APP1: return RETROK_LAUNCH_APP1;
        // case KEY_LAUNCH_APP2: return RETROK_LAUNCH_APP2;

        // Unhandled godot keys
        case KEY_SPECIAL: return RETROK_UNKNOWN;
        case KEY_BACKTAB: return RETROK_UNKNOWN;
        case KEY_F16: return RETROK_UNKNOWN;
        case KEY_F17: return RETROK_UNKNOWN;
        case KEY_F18: return RETROK_UNKNOWN;
        case KEY_F19: return RETROK_UNKNOWN;
        case KEY_F20: return RETROK_UNKNOWN;
        case KEY_F21: return RETROK_UNKNOWN;
        case KEY_F22: return RETROK_UNKNOWN;
        case KEY_F23: return RETROK_UNKNOWN;
        case KEY_F24: return RETROK_UNKNOWN;
        case KEY_F25: return RETROK_UNKNOWN;
        case KEY_F26: return RETROK_UNKNOWN;
        case KEY_F27: return RETROK_UNKNOWN;
        case KEY_F28: return RETROK_UNKNOWN;
        case KEY_F29: return RETROK_UNKNOWN;
        case KEY_F30: return RETROK_UNKNOWN;
        case KEY_F31: return RETROK_UNKNOWN;
        case KEY_F32: return RETROK_UNKNOWN;
        case KEY_F33: return RETROK_UNKNOWN;
        case KEY_F34: return RETROK_UNKNOWN;
        case KEY_F35: return RETROK_UNKNOWN;
        case KEY_HYPER: return RETROK_UNKNOWN;
        case KEY_MEDIARECORD: return RETROK_UNKNOWN;
        case KEY_STANDBY: return RETROK_UNKNOWN;
        case KEY_OPENURL: return RETROK_UNKNOWN;
        case KEY_LAUNCH0: return RETROK_UNKNOWN;
        case KEY_LAUNCH1: return RETROK_UNKNOWN;
        case KEY_LAUNCH2: return RETROK_UNKNOWN;
        case KEY_LAUNCH3: return RETROK_UNKNOWN;
        case KEY_LAUNCH4: return RETROK_UNKNOWN;
        case KEY_LAUNCH5: return RETROK_UNKNOWN;
        case KEY_LAUNCH6: return RETROK_UNKNOWN;
        case KEY_LAUNCH7: return RETROK_UNKNOWN;
        case KEY_LAUNCH8: return RETROK_UNKNOWN;
        case KEY_LAUNCH9: return RETROK_UNKNOWN;
        case KEY_LAUNCHA: return RETROK_UNKNOWN;
        case KEY_LAUNCHB: return RETROK_UNKNOWN;
        case KEY_LAUNCHC: return RETROK_UNKNOWN;
        case KEY_LAUNCHD: return RETROK_UNKNOWN;
        case KEY_LAUNCHE: return RETROK_UNKNOWN;
        case KEY_LAUNCHF: return RETROK_UNKNOWN;
        case KEY_GLOBE: return RETROK_UNKNOWN;
        case KEY_KEYBOARD: return RETROK_UNKNOWN;
        case KEY_JIS_EISU: return RETROK_UNKNOWN;
        case KEY_JIS_KANA: return RETROK_UNKNOWN;
        case KEY_UNKNOWN: return RETROK_UNKNOWN;
        case KEY_PERCENT: return RETROK_UNKNOWN;
        case KEY_YEN: return RETROK_UNKNOWN;
        case KEY_SECTION: return RETROK_UNKNOWN;
        default:
            LogError("Unmapped key code from Godot to Libretro: " + std::to_string(keyEvent->get_keycode()));
            break;
    };

    return RETROK_UNKNOWN;
}

static int16_t ToShort(float floatValue, int mul = 1)
{
    return static_cast<int16_t>(Math::clamp(Math::round(floatValue), static_cast<float>(INT16_MIN), static_cast<float>(INT16_MAX)) * mul);
}

void Libretro::StartContent(String root_directory, String core_name, String game_path)
{
    StartEmulationThread(root_directory, core_name, game_path);
}

void Libretro::StopContent()
{
    StopEmulationThread(true);
}

const std::vector<String>& get_action_names()
{
    static std::vector<String> action_names = {
        "RETRO_JOYPAD_B",
        "RETRO_JOYPAD_Y",
        "RETRO_JOYPAD_SELECT",
        "RETRO_JOYPAD_START",
        "RETRO_JOYPAD_UP",
        "RETRO_JOYPAD_DOWN",
        "RETRO_JOYPAD_LEFT",
        "RETRO_JOYPAD_RIGHT",
        "RETRO_JOYPAD_A",
        "RETRO_JOYPAD_X",
        "RETRO_JOYPAD_L",
        "RETRO_JOYPAD_R",
        "RETRO_JOYPAD_L2",
        "RETRO_JOYPAD_R2",
        "RETRO_JOYPAD_L3",
        "RETRO_JOYPAD_R3",
        "RETRO_JOYPAD_ANALOG_LEFT_X_POSITIVE",
        "RETRO_JOYPAD_ANALOG_LEFT_X_NEGATIVE",
        "RETRO_JOYPAD_ANALOG_LEFT_Y_POSITIVE",
        "RETRO_JOYPAD_ANALOG_LEFT_Y_NEGATIVE",
        "RETRO_JOYPAD_ANALOG_RIGHT_X_POSITIVE",
        "RETRO_JOYPAD_ANALOG_RIGHT_X_NEGATIVE",
        "RETRO_JOYPAD_ANALOG_RIGHT_Y_POSITIVE",
        "RETRO_JOYPAD_ANALOG_RIGHT_Y_NEGATIVE"
    };
    return action_names;
}

void Libretro::_ready()
{
    Node* parent = get_parent();
    if (!Object::cast_to<MeshInstance3D>(parent))
    {
        LogError("Must be a child of MeshInstance3D!");
        queue_free();
    }
}

void Libretro::_exit_tree()
{
    StopEmulationThread(false);
}


void Libretro::_input(const Ref<InputEvent>& event)
{
    if (!m_running)
        return;

    Ref<InputEventKey> keyEvent = event;
    if (keyEvent.is_valid())
    {
        auto input_map = InputMap::get_singleton();
        for (const String& action : get_action_names())
        {
            if (input_map->event_is_action(event, action))
            {
                bool pressed = keyEvent->is_pressed();
                if (action == "RETRO_JOYPAD_B")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_B);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_B);
                }
                else if (action == "RETRO_JOYPAD_Y")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_Y);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_Y);
                }
                else if (action == "RETRO_JOYPAD_SELECT")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_SELECT);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_SELECT);
                }
                else if (action == "RETRO_JOYPAD_START")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_START);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_START);
                }
                else if (action == "RETRO_JOYPAD_UP")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_UP);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_UP);
                }
                else if (action == "RETRO_JOYPAD_DOWN")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_DOWN);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_DOWN);
                }
                else if (action == "RETRO_JOYPAD_LEFT")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_LEFT);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_LEFT);
                }
                else if (action == "RETRO_JOYPAD_RIGHT")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_RIGHT);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_RIGHT);
                }
                else if (action == "RETRO_JOYPAD_A")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_A);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_A);
                }
                else if (action == "RETRO_JOYPAD_X")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_X);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_X);
                }
                else if (action == "RETRO_JOYPAD_L")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_L);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_L);
                }
                else if (action == "RETRO_JOYPAD_R")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_R);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_R);
                }
                else if (action == "RETRO_JOYPAD_L2")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_L2);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_L2);
                }
                else if (action == "RETRO_JOYPAD_R2")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_R2);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_R2);
                }
                else if (action == "RETRO_JOYPAD_L3")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_L3);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_L3);
                }
                else if (action == "RETRO_JOYPAD_R3")
                {
                    if (pressed)
                        m_input->m_joypad_buttons[0] |= (1 << RETRO_DEVICE_ID_JOYPAD_R3);
                    else
                        m_input->m_joypad_buttons[0] &= ~(1 << RETRO_DEVICE_ID_JOYPAD_R3);
                }
            }
        }
    }

    Ref<InputEventMouseMotion> mouseMotion = event;
    if (mouseMotion.is_valid())
    {
        auto mouseMotionValue = mouseMotion->get_relative();
        m_input->m_mouse_x[0] += ToShort(mouseMotionValue.x * 1.8f);
        m_input->m_mouse_y[0] += ToShort(mouseMotionValue.y * 1.8f);
    }

    Ref<InputEventMouseButton> mouseButton = event;
    if (mouseButton.is_valid())
    {
        int button_index = mouseButton->get_button_index();
        bool pressed     = mouseButton->is_pressed();

        switch (button_index)
        {
            case MouseButton::MOUSE_BUTTON_LEFT:
                if (pressed)
                    m_input->m_mouse_buttons[0] |= (1 << RETRO_DEVICE_ID_MOUSE_LEFT);
                else
                    m_input->m_mouse_buttons[0] &= ~(1 << RETRO_DEVICE_ID_MOUSE_LEFT);
                break;
            case MouseButton::MOUSE_BUTTON_RIGHT:
                if (pressed)
                    m_input->m_mouse_buttons[0] |= (1 << RETRO_DEVICE_ID_MOUSE_RIGHT);
                else
                    m_input->m_mouse_buttons[0] &= ~(1 << RETRO_DEVICE_ID_MOUSE_RIGHT);
                break;
            case MouseButton::MOUSE_BUTTON_MIDDLE:
                if (pressed)
                    m_input->m_mouse_buttons[0] |= (1 << RETRO_DEVICE_ID_MOUSE_MIDDLE);
                else
                    m_input->m_mouse_buttons[0] &= ~(1 << RETRO_DEVICE_ID_MOUSE_MIDDLE);
                break;
            case MouseButton::MOUSE_BUTTON_WHEEL_UP:
                if (pressed)
                    m_input->m_mouse_buttons[0] |= (1 << RETRO_DEVICE_ID_MOUSE_WHEELUP);
                else
                    m_input->m_mouse_buttons[0] &= ~(1 << RETRO_DEVICE_ID_MOUSE_WHEELUP);
                break;
            case MouseButton::MOUSE_BUTTON_WHEEL_DOWN:
                if (pressed)
                    m_input->m_mouse_buttons[0] |= (1 << RETRO_DEVICE_ID_MOUSE_WHEELDOWN);
                else
                    m_input->m_mouse_buttons[0] &= ~(1 << RETRO_DEVICE_ID_MOUSE_WHEELDOWN);
                break;
            case MouseButton::MOUSE_BUTTON_WHEEL_LEFT:
                if (pressed)
                    m_input->m_mouse_buttons[0] |= (1 << RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP);
                else
                    m_input->m_mouse_buttons[0] &= ~(1 << RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP);
                break;
            case MouseButton::MOUSE_BUTTON_WHEEL_RIGHT:
                if (pressed)
                    m_input->m_mouse_buttons[0] |= (1 << RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN);
                else
                    m_input->m_mouse_buttons[0] &= ~(1 << RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN);
                break;
            case MouseButton::MOUSE_BUTTON_XBUTTON1:
                if (pressed)
                    m_input->m_mouse_buttons[0] |= (1 << RETRO_DEVICE_ID_MOUSE_BUTTON_4);
                else
                    m_input->m_mouse_buttons[0] &= ~(1 << RETRO_DEVICE_ID_MOUSE_BUTTON_4);
                break;
            case MouseButton::MOUSE_BUTTON_XBUTTON2:
                if (pressed)
                    m_input->m_mouse_buttons[0] |= (1 << RETRO_DEVICE_ID_MOUSE_BUTTON_5);
                else
                    m_input->m_mouse_buttons[0] &= ~(1 << RETRO_DEVICE_ID_MOUSE_BUTTON_5);
                break;
            default:
                print_line_rich("[color=red][SK.Libretro][/color] Unhandled mouse button: " + String::num_int64(button_index));
                break;
        }
    }

    if (keyEvent.is_valid())
    {
        bool down             = keyEvent->is_pressed();
        uint32_t keycode      = GodotToLibretroKeycode(keyEvent);
        uint32_t character    = keyEvent->get_unicode();
        uint16_t keyModifiers = 0;
        auto mods             = keyEvent->get_modifiers_mask();
        if (mods & KeyModifierMask::KEY_MASK_SHIFT)
            keyModifiers |= RETROKMOD_SHIFT;
        if (mods & KeyModifierMask::KEY_MASK_CTRL)
            keyModifiers |= RETROKMOD_CTRL;
        if (mods & KeyModifierMask::KEY_MASK_ALT)
            keyModifiers |= RETROKMOD_ALT;
        if (mods & KeyModifierMask::KEY_MASK_META)
            keyModifiers |= RETROKMOD_META;

        if (down)
            m_input->m_keyboard_keys[0] |= (1 << keycode);
        else
            m_input->m_keyboard_keys[0] &= ~(1 << keycode);

        if (m_input->m_keyboard_event)
            m_input->m_keyboard_event(down, keycode, character, keyModifiers);
    }
}

void Libretro::_process(double delta)
{
    if (!m_running)
        return;

    std::unique_ptr<ThreadCommand> command;
    while (m_main_thread_commands_queue.try_dequeue(command))
        command->Execute();

    Vector2 left_vec;
    Vector2 right_vec;

    auto input_map = godot::Input::get_singleton();

    if (input_map->is_action_pressed("RETRO_JOYPAD_ANALOG_LEFT_X_NEGATIVE"))
        left_vec.x -= 1.0f;
    if (input_map->is_action_pressed("RETRO_JOYPAD_ANALOG_LEFT_X_POSITIVE"))
        left_vec.x += 1.0f;
    if (input_map->is_action_pressed("RETRO_JOYPAD_ANALOG_LEFT_Y_POSITIVE"))
        left_vec.y -= 1.0f;
    if (input_map->is_action_pressed("RETRO_JOYPAD_ANALOG_LEFT_Y_NEGATIVE"))
        left_vec.y += 1.0f;

    if (left_vec.length() > 1.0f)
        left_vec = left_vec.normalized();

    m_input->m_analog_left_x[0] = static_cast<int16_t>(left_vec.x * 0x7fff);
    m_input->m_analog_left_y[0] = static_cast<int16_t>(left_vec.y * 0x7fff);

    if (input_map->is_action_pressed("RETRO_JOYPAD_ANALOG_RIGHT_X_NEGATIVE"))
        right_vec.x -= 1.0f;
    if (input_map->is_action_pressed("RETRO_JOYPAD_ANALOG_RIGHT_X_POSITIVE"))
        right_vec.x += 1.0f;
    if (input_map->is_action_pressed("RETRO_JOYPAD_ANALOG_RIGHT_Y_POSITIVE"))
        right_vec.y -= 1.0f;
    if (input_map->is_action_pressed("RETRO_JOYPAD_ANALOG_RIGHT_Y_NEGATIVE"))
        right_vec.y += 1.0f;

    if (right_vec.length() > 1.0f)
        right_vec = right_vec.normalized();

    m_input->m_analog_right_x[0] = static_cast<int16_t>(right_vec.x * 0x7fff);
    m_input->m_analog_right_y[0] = static_cast<int16_t>(right_vec.y * 0x7fff);
}

void Libretro::DeInitGraphics()
{
    Node* parent = get_parent();
    MeshInstance3D* mesh = Object::cast_to<MeshInstance3D>(parent);
    if (mesh)
        mesh->set_surface_override_material(0, m_original_surface_material_override);

    if (m_new_material.is_valid())
        m_new_material.unref();

    if (m_video->m_texture.is_valid())
        m_video->m_texture.unref();

    if (m_video->m_image.is_valid())
        m_video->m_image.unref();

    if (m_video->m_sdl_gl_context)
    {
        SDL_GL_DestroyContext(m_video->m_sdl_gl_context);
        m_video->m_sdl_gl_context = nullptr;
    }

    if (m_video->m_sdl_window)
    {
        SDL_DestroyWindow(m_video->m_sdl_window);
        m_video->m_sdl_window = nullptr;
    }
}

void Libretro::InitAudio(float bufferLengthSec, double sampleRate)
{
    m_main_thread_commands_queue.enqueue(std::make_unique<ThreadCommandInitAudio>(this, bufferLengthSec, sampleRate));
}

void Libretro::DeinitAudio()
{
    if (m_audio->m_audio_stream_player)
    {
        m_audio->m_audio_stream_player->stop();
        m_audio->m_audio_stream_player = nullptr;
    }

    if (m_audio->m_audio_stream_generator_playback.is_valid())
    {
        m_audio->m_audio_stream_generator_playback->stop();
        m_audio->m_audio_stream_generator_playback.unref();
    }

    if (m_audio->m_audio_stream_generator.is_valid())
        m_audio->m_audio_stream_generator.unref();
}

void Libretro::StartEmulationThread(String root_directory, String core_name, String game_path)
{
    Node* parent = get_parent();
    if (!parent)
        return;
        
    MeshInstance3D* mesh = Object::cast_to<MeshInstance3D>(parent);
    if (!mesh)
        return;

    if (m_core)
    {
        LogWarning("Emulation thread is already running.");
        return;
    }

    String core_path = root_directory.path_join("cores").path_join(core_name + "_libretro.dll");

    m_core = std::make_unique<Core>(this, core_path.utf8().get_data());
    m_environment = std::make_unique<Environment>();
    m_video = std::make_unique<Video>();
    m_audio = std::make_unique<Audio>();
    m_input = std::make_unique<Input>();

    if (m_new_material.is_valid())
        m_new_material.unref();

    m_original_surface_material_override = mesh->get_surface_override_material(0);

    m_new_material.instantiate();
    mesh->set_surface_override_material(0, m_new_material);

    m_new_material->set_albedo(Color(0, 0, 0, 1));
    m_new_material->set_feature(StandardMaterial3D::FEATURE_EMISSION, true);

    m_root_directory = root_directory.utf8().get_data();
    m_temp_directory = root_directory.path_join("temp").utf8().get_data();
    m_game_path = game_path.utf8().get_data();
    m_environment->m_system_directory = root_directory.path_join("system").path_join(core_name).utf8().get_data();
    m_environment->m_save_directory = root_directory.path_join("save").path_join(core_name).utf8().get_data();
    m_environment->m_core_assets_directory = root_directory.path_join("core_assets").path_join(core_name).utf8().get_data();

    if (!std::filesystem::exists(m_temp_directory))
        std::filesystem::create_directories(m_temp_directory);

    m_thread = std::thread(&Libretro::EmulationThreadLoop, this);
}

void Libretro::StopEmulationThread(bool log)
{
    if (!m_core)
    {
        if (log)
            LogWarning("Emulation thread is not running.");
        return;
    }

    m_running = false;
    m_thread.join();

    DeInitGraphics();
    DeinitAudio();

    UnregisterInstance();

    m_core = nullptr;
    m_environment = nullptr;
    m_video = nullptr;
    m_audio = nullptr;
    m_input = nullptr;
}

void Libretro::EmulationThreadLoop()
{
    Log("Libretro Thread starting...");

    if (!RegisterInstance(this))
        return;

    if (!m_core->Load())
        return;

    if (m_game_path.empty())
    {
        if (!m_core->GetSupportsNoGame())
        {
            LogError("Game not set and this core does not support no game mode.");
            return;
        }

        retro_game_info game_info = {};

        if (!m_core->retro_load_game_func(&game_info))
        {
            LogError("Failed to load game");
            return;
        }
    }
    else
    {
        if (!std::filesystem::is_regular_file(m_game_path))
        {
            LogError("Game not found: " + m_game_path);
            return;
        }

        std::ifstream file(m_game_path, std::ios::binary | std::ios::ate);
        if (!file)
        {
            LogError("Failed to open game file: " + m_game_path);
            return;
        }

        size_t game_size = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);
        
        m_game_buffer.resize(game_size);
        if (!file.read(reinterpret_cast<char*>(m_game_buffer.data()), game_size))
        {
            LogError("Failed to read game file: " + m_game_path);
            return;
        }

        retro_game_info game_info = {};
        game_info.path = m_game_path.c_str();
        game_info.data = reinterpret_cast<const void*>(m_game_buffer.data());
        game_info.size = game_size;
        game_info.meta = nullptr;

        if (!m_core->retro_load_game_func(&game_info))
        {
            LogError("Failed to load game");
            return;
        }
    }

    retro_system_av_info systemAvInfo = {};
    m_core->retro_get_system_av_info_func(&systemAvInfo);

    Log("FPS: " + std::to_string(systemAvInfo.timing.fps) + " Sample Rate: " + std::to_string(systemAvInfo.timing.sample_rate));

    if (m_video->m_context_reset)
    {
        Log("Creating OpenGL context...");

        if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
        {
            LogError("Failed to initialize SDL video subsystem: " + std::string(SDL_GetError()));
            return;
        }

        m_video->m_sdl_window = SDL_CreateWindow("Libretro OpenGL Window", systemAvInfo.geometry.base_width, systemAvInfo.geometry.base_height, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if (!m_video->m_sdl_window)
        {
            LogError("Failed to create SDL window: " + std::string(SDL_GetError()));
            return;
        }

        m_video->m_sdl_gl_context = SDL_GL_CreateContext(m_video->m_sdl_window);
        if (!m_video->m_sdl_gl_context)
        {
            LogError("Failed to create SDL OpenGL context: " + std::string(SDL_GetError()));
            SDL_DestroyWindow(m_video->m_sdl_window);
            return;
        }

        LogOK("OpenGL context created successfully.");

        SDL_GL_MakeCurrent(m_video->m_sdl_window, m_video->m_sdl_gl_context);
        m_video->m_context_reset();
    }

    m_running = true;

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_mutex_done = false;
        InitAudio(0.1f, systemAvInfo.timing.sample_rate);
        m_condition_variable.wait(lock, [&]{ return m_mutex_done; });
    }

    double frameDurationMs = 1000.0 / systemAvInfo.timing.fps;
    auto lastTime = std::chrono::steady_clock::now();
    double accumulator = 0.0;

    double frame_duration_ms = 1000.0 / systemAvInfo.timing.fps;
    auto last_time = std::chrono::steady_clock::now();

    while (m_running)
    {
        if (!m_running)
            break;

        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - last_time).count();

        if (elapsed < frame_duration_ms)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(frame_duration_ms - elapsed)));
            continue;
        }

        last_time = now;

        if (m_audio->m_audio_buffer_status_callback)
            m_audio->m_audio_buffer_status_callback(true, m_audio->m_audio_buffer_occupancy, m_audio->m_audio_buffer_occupancy <= 2);

        m_core->retro_run_func();
    }
    
    m_core->retro_unload_game_func();
    m_core->retro_deinit_func();

    m_running = false;
    Log("Libretro thread stopped.");
}

void Libretro::CreateTexture(Image::Format image_format, PackedByteArray pixel_data, int32_t width, int32_t height, bool flip_y)
{
    m_video->m_image_format = image_format;
    m_main_thread_commands_queue.enqueue(std::make_unique<ThreadCommandCreateTexture>(this, image_format, pixel_data, width, height, flip_y));
}

void Libretro::UpdateTexture(PackedByteArray pixel_data, bool flip_y)
{
    m_main_thread_commands_queue.enqueue(std::make_unique<ThreadCommandUpdateTexture>(this, pixel_data, flip_y));
}

void Libretro::LogInterfaceLog(retro_log_level level, const char* fmt, ...)
{
    auto instance = GetInstance();
    if (!instance)
    {
        print_line_rich("[color=red][SK.Libretro][/color] LogInterfaceLog: Null Instance.");
        return;
    }

    if (level < instance->m_log_level)
        return;

    va_list args;
    va_start(args, fmt);
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    switch (level)
    {
    case RETRO_LOG_DEBUG:
        print_line_rich("[color=white][Core][Debug]:[/color] " + String(buffer));
        break;
    case RETRO_LOG_INFO:
        print_line_rich("[color=white][Core][Info]:[/color] " + String(buffer));
        break;
    case RETRO_LOG_WARN:
        print_line_rich("[color=orange][Core][Warning]:[/color] " + String(buffer));
        break;
    case RETRO_LOG_ERROR:
        print_line_rich("[color=red][Core][Error]:[/color] " + String(buffer));
        break;
    default:
        break;
    }

    va_end(args);
}

void Libretro::LedInterfaceSetLedState(int32_t led, int32_t state)
{
    print_line_rich("[color=cyan][LedInterfaceSetLedState][/color] LED " + String::num_int64(led) + " set to " + (state ? "on" : "off"));
}

void Libretro::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("start_content", "rootDirectory", "coreName", "gamePath"), &Libretro::StartContent);
    ClassDB::bind_method(D_METHOD("stop_content"), &Libretro::StopContent);
}

Libretro::ThreadCommand::ThreadCommand(Libretro* instance)
: m_instance(instance)
{
}

Libretro::ThreadCommandInitAudio::ThreadCommandInitAudio(Libretro* instance, float bufferCapacitySec, double sampleRate)
: ThreadCommand(instance)
, m_bufferCapacitySec(bufferCapacitySec)
, m_sampleRate(sampleRate)
{
}

void Libretro::ThreadCommandInitAudio::Execute()
{
    std::unique_lock<std::mutex> lock(m_instance->m_mutex);

    m_instance->m_audio->m_audio_buffer_capacity_sec = m_bufferCapacitySec;
    m_instance->m_audio->m_audio_sample_rate = m_sampleRate;

    m_instance->m_audio->m_audio_stream_generator.instantiate();
    m_instance->m_audio->m_audio_stream_generator->set_mix_rate(m_sampleRate);
    m_instance->m_audio->m_audio_stream_generator->set_buffer_length(m_bufferCapacitySec);

    m_instance->m_audio->m_audio_stream_player = m_instance->get_node<AudioStreamPlayer>("AudioStreamPlayer");
    m_instance->m_audio->m_audio_stream_player->set_stream(m_instance->m_audio->m_audio_stream_generator);
    m_instance->m_audio->m_audio_stream_player->play();

    m_instance->m_audio->m_audio_stream_generator_playback = m_instance->m_audio->m_audio_stream_player->get_stream_playback();

    m_instance->m_mutex_done = true;
    m_instance->m_condition_variable.notify_one();
}

Libretro::ThreadCommandCreateTexture::ThreadCommandCreateTexture(Libretro* instance, Image::Format image_format, PackedByteArray pixel_data, int32_t width, int32_t height, bool flip_y)
: ThreadCommand(instance)
, m_imageFormat(image_format)
, m_pixelData(pixel_data)
, m_width(width)
, m_height(height)
, m_flipY(flip_y)
{
}

void Libretro::ThreadCommandCreateTexture::Execute()
{
    m_instance->m_video->m_image.instantiate();
    m_instance->m_video->m_image->create(m_width, m_height, false, m_imageFormat);
    m_instance->m_video->m_image->set_data(m_width, m_height, false, m_imageFormat, m_pixelData);
    if (m_flipY)
        m_instance->m_video->m_image->flip_y();

    m_instance->m_video->m_texture = ImageTexture::create_from_image(m_instance->m_video->m_image);

    Node* parent = m_instance->get_parent();
    MeshInstance3D* mesh = Object::cast_to<MeshInstance3D>(parent);
    mesh->set_surface_override_material(0, m_instance->m_new_material);
    m_instance->m_new_material->set_texture(StandardMaterial3D::TEXTURE_EMISSION, m_instance->m_video->m_texture);
}

Libretro::ThreadCommandUpdateTexture::ThreadCommandUpdateTexture(Libretro* instance, godot::PackedByteArray pixelData, bool flipY)
: ThreadCommand(instance)
, m_pixelData(pixelData)
, m_flipY(flipY)
{
}

void Libretro::ThreadCommandUpdateTexture::Execute()
{
    if (m_instance->m_video->m_image.is_null())
        return;

    m_instance->m_video->m_image->set_data(m_instance->m_video->m_last_width, m_instance->m_video->m_last_height, false, m_instance->m_video->m_image_format, m_pixelData);
    if (m_flipY)
        m_instance->m_video->m_image->flip_y();

    if (!m_instance->m_video->m_image->is_empty() && m_instance->m_video->m_texture.is_valid())
        m_instance->m_video->m_texture->update(m_instance->m_video->m_image);
}
}
