#include "Input.hpp"

#include "Libretro.hpp"
#include "Debug.hpp"

namespace SK
{
static int16_t ProcessJoypadDevice(Libretro* instance, uint32_t port, uint32_t id);
static int16_t ProcessMouseDevice(Libretro* instance, uint32_t port, uint32_t id);
static int16_t ProcessKeyboardDevice(Libretro* instance, uint32_t port, uint32_t id);
static int16_t ProcessLightgunDevice(Libretro* instance, uint32_t port, uint32_t id);
static int16_t ProcessPointerDevice(Libretro* instance, uint32_t port, uint32_t id);
static int16_t ProcessAnalogDevice(Libretro* instance, uint32_t port, uint32_t index, uint32_t id);

void Input::PollCallback()
{
}

int16_t Input::StateCallback(uint32_t port, uint32_t device, uint32_t index, uint32_t id)
{
    auto instance = Libretro::GetInstance();
    if (!instance)
    {
        LogError("StateCallback: Null Instance.");
        return 0;
    }

    device &= RETRO_DEVICE_MASK;

    switch (device)
    {
    case RETRO_DEVICE_JOYPAD:
        return ProcessJoypadDevice(instance, port, id);
    case RETRO_DEVICE_MOUSE:
        return ProcessMouseDevice(instance, port, id);
    case RETRO_DEVICE_KEYBOARD:
        return ProcessKeyboardDevice(instance, port, id);
    case RETRO_DEVICE_LIGHTGUN:
        return ProcessLightgunDevice(instance, port, id);
    case RETRO_DEVICE_POINTER:
        return ProcessPointerDevice(instance, port, id);
    case RETRO_DEVICE_ANALOG:
        return ProcessAnalogDevice(instance, port, index, id);
    default:
        LogError("Unhandled input device: " + std::to_string(device) + " for port: " + std::to_string(port) + " and id: " + std::to_string(id));
        break;
    }

    return 0;
}

bool Input::RumbleInterfaceSetRumbleState(uint32_t port, retro_rumble_effect effect, uint16_t strength)
{
    Log("Port: " + std::to_string(port) +
        " Effect: " + (effect == RETRO_RUMBLE_STRONG ? "strong" : "weak") +
        " Strength: " + std::to_string(strength));
    return true;
}

int16_t ProcessJoypadDevice(Libretro* instance, uint32_t port, uint32_t id)
{
    if (id == RETRO_DEVICE_ID_JOYPAD_MASK)
        return instance->m_input->m_joypad_buttons[port];
    return instance->m_input->m_joypad_buttons[port] & (1 << id);
}

int16_t ProcessMouseDevice(Libretro* instance, uint32_t port, uint32_t id)
{
    switch (id)
    {
    case RETRO_DEVICE_ID_MOUSE_X:
    {
        int16_t dx = instance->m_input->m_mouse_x[port];
        instance->m_input->m_mouse_x[port] = 0;
        return dx;
    }
    break;
    case RETRO_DEVICE_ID_MOUSE_Y:
    {
        int16_t dy = instance->m_input->m_mouse_y[port];
        instance->m_input->m_mouse_y[port] = 0;
        return dy;
    }
    break;
    default:
        return instance->m_input->m_mouse_buttons[port] & (1 << id);
    }
}

static int16_t ProcessKeyboardDevice(Libretro* instance, uint32_t port, uint32_t id)
{
    return instance->m_input->m_keyboard_keys[port] & (1 << id);
}

static int16_t ProcessLightgunDevice(Libretro* instance, uint32_t port, uint32_t id)
{
    switch (id)
    {
        case RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X:
            return instance->m_input->m_lightgun_x[port];
        case RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y:
            return instance->m_input->m_lightgun_y[port];
        case RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN:
            return instance->m_input->m_lightgun_is_offscreen[port];
        default:
            return instance->m_input->m_lightgun_buttons[port] & (1 << id);
    }
}

static int16_t ProcessPointerDevice(Libretro* instance, uint32_t port, uint32_t id)
{
    switch (id)
    {
        case RETRO_DEVICE_ID_POINTER_X:
            return instance->m_input->m_pointer_x[port];
        case RETRO_DEVICE_ID_POINTER_Y:
            return instance->m_input->m_pointer_y[port];
        case RETRO_DEVICE_ID_POINTER_PRESSED:
            return instance->m_input->m_pointer_pressed[port];
        case RETRO_DEVICE_ID_POINTER_COUNT:
            return instance->m_input->m_pointer_count[port];
        default:
            return 0;
    }
}

static int16_t ProcessAnalogDevice(Libretro* instance, uint32_t port, uint32_t index, uint32_t id)
{
    switch (index)
    {
        case RETRO_DEVICE_INDEX_ANALOG_LEFT:
            switch (id)
            {
                case RETRO_DEVICE_ID_ANALOG_X:
                    return instance->m_input->m_analog_left_x[port];
                case RETRO_DEVICE_ID_ANALOG_Y:
                    return instance->m_input->m_analog_left_y[port];
                default:
                    return 0;
            }
        case RETRO_DEVICE_INDEX_ANALOG_RIGHT:
            switch (id)
            {
                case RETRO_DEVICE_ID_ANALOG_X:
                    return instance->m_input->m_analog_right_x[port];
                case RETRO_DEVICE_ID_ANALOG_Y:
                    return instance->m_input->m_analog_right_y[port];
                default:
                    return 0;
            }
        default:
            return 0;
    }
}
}
