/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "LinuxInput.hpp"

#if TK_LINUX

#include "LinuxWindow.hpp"
#include "LinuxWindowSys.hpp"
#include <toolkit/debug/Logger.hpp>
#include <toolkit/debug/Profiler.hpp>
#include <toolkit/input/Gamepad.hpp>
#include <toolkit/input/Keyboard.hpp>
#include <toolkit/input/Mouse.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/platforms/System.hpp>
#include <toolkit/utils/Convertor.hpp>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <cerrno>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>

namespace toolkit
{

    constexpr i32 LinuxInputMaxGamepads = MAX_GAMEPADS;
    constexpr i32 LinuxInputMaxGamepadEventsPerFrame = 256;
    constexpr i32 TriggerThreshold = 25000;
    constexpr X11::Time MouseDoubleClickTime = 500;
    constexpr float MaxDoubleClickDistanceSquared = 10.0f;

    struct LinuxInputDevice
    {
        u32 uid[4]{};
        std::string name;
        std::string handler;
        bool isGamepad = false;
    };

    LinuxInputDevice s_inputDevices[LinuxInputMaxGamepads]{};
    i32 s_foundGamepads = 0;
    float s_lastUpdateTime = -1000.0f;
    X11::Time s_mouseLastButtonPressTime = 0;
    Pos s_oldMouseClickPosition{};

    float normalizeInputAxis(const int axisVal)
    {
        const float norm = axisVal <= 0 ? 32768.0f : 32767.0f;
        return static_cast<float>(axisVal) / norm;
    }

    float normalizeInputTrigger(const int axisVal)
    {
        return TkMath::saturate(static_cast<float>(axisVal) / 255.0f);
    }

    KeyboardKeys mapKeySym(X11::KeySym keySym)
    {
        if (keySym >= XK_a && keySym <= XK_z)
            return static_cast<KeyboardKeys>(static_cast<i32>(KeyboardKeys::kA) + static_cast<i32>(keySym - XK_a));
        if (keySym >= XK_A && keySym <= XK_Z)
            return static_cast<KeyboardKeys>(static_cast<i32>(KeyboardKeys::kA) + static_cast<i32>(keySym - XK_A));
        if (keySym >= XK_0 && keySym <= XK_9)
            return static_cast<KeyboardKeys>(static_cast<i32>(KeyboardKeys::kAlpha0) + static_cast<i32>(keySym - XK_0));
        if (keySym >= XK_KP_0 && keySym <= XK_KP_9)
            return static_cast<KeyboardKeys>(static_cast<i32>(KeyboardKeys::kNumpad0) + static_cast<i32>(keySym - XK_KP_0));
        if (keySym >= XK_F1 && keySym <= XK_F24)
            return static_cast<KeyboardKeys>(static_cast<i32>(KeyboardKeys::kF1) + static_cast<i32>(keySym - XK_F1));

        switch (keySym)
        {
        case XK_BackSpace:
            return KeyboardKeys::kBackspace;
        case XK_Tab:
        case XK_ISO_Left_Tab:
            return KeyboardKeys::kTab;
        case XK_Return:
        case XK_KP_Enter:
            return KeyboardKeys::kReturn;
        case XK_Pause:
            return KeyboardKeys::kPause;
        case XK_Caps_Lock:
            return KeyboardKeys::kCapital;
        case XK_Escape:
            return KeyboardKeys::kEscape;
        case XK_space:
            return KeyboardKeys::kSpacebar;
        case XK_Page_Up:
        case XK_KP_Page_Up:
            return KeyboardKeys::kPageUp;
        case XK_Page_Down:
        case XK_KP_Page_Down:
            return KeyboardKeys::kPageDown;
        case XK_End:
        case XK_KP_End:
            return KeyboardKeys::kEnd;
        case XK_Home:
        case XK_KP_Home:
            return KeyboardKeys::kHome;
        case XK_Left:
        case XK_KP_Left:
            return KeyboardKeys::kArrowLeft;
        case XK_Up:
        case XK_KP_Up:
            return KeyboardKeys::kArrowUp;
        case XK_Right:
        case XK_KP_Right:
            return KeyboardKeys::kArrowRight;
        case XK_Down:
        case XK_KP_Down:
            return KeyboardKeys::kArrowDown;
        case XK_Print:
            return KeyboardKeys::kPrintScreen;
        case XK_Insert:
        case XK_KP_Insert:
            return KeyboardKeys::kInsert;
        case XK_Delete:
        case XK_KP_Delete:
            return KeyboardKeys::kDelete;
        case XK_Shift_L:
        case XK_Shift_R:
            return KeyboardKeys::kShift;
        case XK_Control_L:
        case XK_Control_R:
            return KeyboardKeys::kControl;
        case XK_Alt_L:
        case XK_Alt_R:
        case XK_Meta_L:
        case XK_Meta_R:
            return KeyboardKeys::kAlt;
        case XK_Super_L:
            return KeyboardKeys::kLeftWindows;
        case XK_Super_R:
            return KeyboardKeys::kRightWindows;
        case XK_Menu:
            return KeyboardKeys::kApplications;
        case XK_Num_Lock:
            return KeyboardKeys::kNumlock;
        case XK_Scroll_Lock:
            return KeyboardKeys::kScroll;
        case XK_KP_Multiply:
            return KeyboardKeys::kNumpadMultiply;
        case XK_KP_Add:
            return KeyboardKeys::kNumpadAdd;
        case XK_KP_Separator:
            return KeyboardKeys::kNumpadSeparator;
        case XK_KP_Subtract:
            return KeyboardKeys::kNumpadSubtract;
        case XK_KP_Decimal:
            return KeyboardKeys::kNumpadDecimal;
        case XK_KP_Divide:
            return KeyboardKeys::kNumpadDivide;
        case XK_semicolon:
        case XK_colon:
            return KeyboardKeys::kColon;
        case XK_equal:
        case XK_plus:
            return KeyboardKeys::kPlus;
        case XK_comma:
        case XK_less:
            return KeyboardKeys::kComma;
        case XK_minus:
        case XK_underscore:
            return KeyboardKeys::kMinus;
        case XK_period:
        case XK_greater:
            return KeyboardKeys::kPeriod;
        case XK_slash:
        case XK_question:
            return KeyboardKeys::kSlash;
        case XK_grave:
        case XK_asciitilde:
            return KeyboardKeys::kBackQuote;
        case XK_bracketleft:
        case XK_braceleft:
            return KeyboardKeys::kLeftBracket;
        case XK_backslash:
        case XK_bar:
            return KeyboardKeys::kBackslash;
        case XK_bracketright:
        case XK_braceright:
            return KeyboardKeys::kRightBracket;
        case XK_apostrophe:
        case XK_quotedbl:
            return KeyboardKeys::kQuote;
        default:
            return KeyboardKeys::kNone;
        }
    }

    MouseButton mapMouseButton(unsigned int button)
    {
        switch (button)
        {
        case Button1:
            return MouseButton::kLeft;
        case Button2:
            return MouseButton::kMiddle;
        case Button3:
            return MouseButton::kRight;
        case 8:
            return MouseButton::kExtended2;
        case 9:
            return MouseButton::kExtended1;
        default:
            return MouseButton::kNone;
        }
    }

    void detectGamepads()
    {
        std::string line;
        std::ifstream devs("/proc/bus/input/devices");

        s_foundGamepads = 0;
        LinuxInputDevice current{};
        if (!devs.is_open())
            return;

        while (std::getline(devs, line))
        {
            if (line.empty())
            {
                if (current.isGamepad && !current.handler.empty() && s_foundGamepads < LinuxInputMaxGamepads)
                    s_inputDevices[s_foundGamepads++] = current;
                current = LinuxInputDevice{};
                continue;
            }

            if (line[0] == 'N')
            {
                const auto quoteIndex = line.find('"');
                const auto secondQuoteIndex = quoteIndex != std::string::npos ? line.find('"', quoteIndex + 1) : std::string::npos;
                if (quoteIndex != std::string::npos && secondQuoteIndex != std::string::npos && secondQuoteIndex > quoteIndex)
                    current.name = line.substr(quoteIndex + 1, secondQuoteIndex - quoteIndex - 1);
            }
            else if (line[0] == 'I')
            {
                size_t startIndex = 0;
                for (i32 i = 0; i < 4; i++)
                {
                    const auto equalsIndex = line.find('=', startIndex);
                    if (equalsIndex == std::string::npos)
                        break;
                    std::istringstream part(line.substr(equalsIndex + 1, 4));
                    part >> std::hex >> current.uid[i];
                    startIndex = equalsIndex + 1;
                }
            }
            else if (line[0] == 'H')
            {
                const auto eventIndex = line.find("event");
                if (eventIndex != std::string::npos)
                {
                    auto end = line.find(' ', eventIndex);
                    if (end == std::string::npos)
                        end = line.length();
                    current.handler = "/dev/input/" + line.substr(eventIndex, end - eventIndex);
                }
            }
            else if (line[0] == 'B')
            {
                const auto keyIndex = line.find("KEY=");
                if (keyIndex != std::string::npos)
                {
                    std::istringstream group(line.substr(keyIndex + 4));
                    unsigned long n64 = 0;
                    unsigned long msb = 0;
                    i32 foundGroups = 0;
                    for (i32 i = 0; i < 5; i++)
                    {
                        group >> std::hex >> n64;
                        if (i == 0)
                            msb = n64;
                        foundGroups++;
                        if (group.eof())
                            break;
                    }
                    if (foundGroups < 5)
                        msb = 0;
                    current.isGamepad = (msb & (1lu << 48)) != 0;
                }
            }
        }

        if (current.isGamepad && !current.handler.empty() && s_foundGamepads < LinuxInputMaxGamepads)
            s_inputDevices[s_foundGamepads++] = current;
    }

    class LinuxKeyboard : public Keyboard
    {
    public:
        bool processEvent(TkWindow *window, X11::XEvent &event)
        {
            if (event.type != KeyPress && event.type != KeyRelease)
                return false;

            X11::Display *display = LinuxWindowSys::display();
            if (display == nullptr)
                return false;

            X11::KeySym keySym = X11::XLookupKeysym(&event.xkey, 0);
            const KeyboardKeys key = mapKeySym(keySym);
            if (key != KeyboardKeys::kNone)
            {
                if (event.type == KeyPress)
                    onKeyDown(key, window);
                else
                    onKeyUp(key, window);
            }

            if (event.type == KeyPress)
            {
                char buffer[32];
                X11::KeySym lookupKeySym = 0;
                const int length = X11::XLookupString(&event.xkey, buffer, sizeof(buffer) - 1, &lookupKeySym, nullptr);
                if (length > 0)
                {
                    buffer[length] = 0;
                    wchar chars[8];
                    i32 outLength = 0;
                    TkConvertor::utf8ToUtf16(buffer, chars, length, outLength);
                    if (outLength > 0)
                        onCharInput(chars[0], window);
                }
            }

            return key != KeyboardKeys::kNone;
        }
    };

    class LinuxMouse : public Mouse
    {
    public:
        bool processEvent(TkWindow *window, X11::XEvent &event)
        {
            if (window == nullptr || window->getNativeWindow() == nullptr || !window->getNativeWindow()->hasHandle())
                return false;

            switch (event.type)
            {
            case MotionNotify:
                onMouseMove(Pos(static_cast<float>(event.xmotion.x_root), static_cast<float>(event.xmotion.y_root)), window);
                return true;
            case LeaveNotify:
                onMouseLeave(window);
                return true;
            case ButtonPress:
            {
                const Pos mousePos(static_cast<float>(event.xbutton.x_root), static_cast<float>(event.xbutton.y_root));
                const MouseButton mouseButton = mapMouseButton(event.xbutton.button);
                if (mouseButton == MouseButton::kNone)
                    return false;

                const Pos localPos(static_cast<float>(event.xbutton.x), static_cast<float>(event.xbutton.y));
                const Pos clickDelta = localPos - s_oldMouseClickPosition;
                if (event.xbutton.button == Button1 &&
                    event.xbutton.time < s_mouseLastButtonPressTime + MouseDoubleClickTime &&
                    clickDelta.x * clickDelta.x + clickDelta.y * clickDelta.y < MaxDoubleClickDistanceSquared)
                {
                    onMouseDoubleClick(mousePos, mouseButton, window);
                    s_mouseLastButtonPressTime = 0;
                    s_oldMouseClickPosition = localPos;
                    return true;
                }

                if (event.xbutton.button == Button1)
                {
                    s_mouseLastButtonPressTime = event.xbutton.time;
                    s_oldMouseClickPosition = localPos;
                }

                onMouseDown(mousePos, mouseButton, window);
                return true;
            }
            case ButtonRelease:
            {
                const Pos mousePos(static_cast<float>(event.xbutton.x_root), static_cast<float>(event.xbutton.y_root));
                if (event.xbutton.button == Button4)
                {
                    onMouseWheel(mousePos, 1.0f, window);
                    return true;
                }
                if (event.xbutton.button == Button5)
                {
                    onMouseWheel(mousePos, -1.0f, window);
                    return true;
                }

                const MouseButton mouseButton = mapMouseButton(event.xbutton.button);
                if (mouseButton == MouseButton::kNone)
                    return false;
                onMouseUp(mousePos, mouseButton, window);
                return true;
            }
            default:
                return false;
            }
        }

        void setMousePosition(const Pos &newPosition) final override
        {
            System::setMousePos(newPosition);
            onMouseMoved(newPosition);
        }
    };

    class LinuxGamepad : public Gamepad
    {
    public:
        TkString m_dev;
        int m_fd = -1;

        LinuxGamepad(u32 uid[4], const TkString &name)
            : Gamepad(TkUuid(uid[0], uid[1], uid[2], uid[3]), name)
        {
        }

        ~LinuxGamepad()
        {
            if (m_fd >= 0)
                close(m_fd);
        }

    protected:
        bool updateState() override
        {
            if (m_fd < 0)
                m_fd = open(m_dev.getText(), O_RDONLY | O_NONBLOCK);
            if (m_fd < 0)
                return true;

            input_event event{};
            for (i32 i = 0; i < LinuxInputMaxGamepadEventsPerFrame; i++)
            {
                const ssize_t r = read(m_fd, &event, sizeof(event));
                if (r < 0)
                {
                    if (errno != EAGAIN)
                    {
                        LogWarn("Lost connection to gamepad '%s', errno=%d", getName().getText(), errno);
                        close(m_fd);
                        m_fd = -1;
                        return true;
                    }
                    break;
                }
                if (r == 0)
                    break;
                if (r != sizeof(event))
                {
                    LogWarn("Gamepad '%s' got a shortened package from the kernel %d", getName().getText(), static_cast<i32>(r));
                    break;
                }
                if (event.type > EV_MAX)
                {
                    LogWarn("Gamepad '%s' got an invalid event type from the kernel %d", getName().getText(), event.type);
                    break;
                }

                if (event.type == EV_KEY)
                {
                    switch (event.code)
                    {
                    case BTN_A:
                        m_state.buttons[static_cast<i32>(GamepadButton::kA)] = event.value != 0;
                        break;
                    case BTN_B:
                        m_state.buttons[static_cast<i32>(GamepadButton::kB)] = event.value != 0;
                        break;
                    case BTN_X:
                        m_state.buttons[static_cast<i32>(GamepadButton::kX)] = event.value != 0;
                        break;
                    case BTN_Y:
                        m_state.buttons[static_cast<i32>(GamepadButton::kY)] = event.value != 0;
                        break;
                    case BTN_TL:
                        m_state.buttons[static_cast<i32>(GamepadButton::kLeftShoulder)] = event.value != 0;
                        break;
                    case BTN_TR:
                        m_state.buttons[static_cast<i32>(GamepadButton::kRightShoulder)] = event.value != 0;
                        break;
                    case BTN_BACK:
                        m_state.buttons[static_cast<i32>(GamepadButton::kBack)] = event.value != 0;
                        break;
                    case BTN_START:
                        m_state.buttons[static_cast<i32>(GamepadButton::kStart)] = event.value != 0;
                        break;
                    case BTN_THUMBL:
                        m_state.buttons[static_cast<i32>(GamepadButton::kLeftThumb)] = event.value != 0;
                        break;
                    case BTN_THUMBR:
                        m_state.buttons[static_cast<i32>(GamepadButton::kRightThumb)] = event.value != 0;
                        break;
                    case BTN_DPAD_UP:
                        m_state.buttons[static_cast<i32>(GamepadButton::kDPadUp)] = event.value != 0;
                        break;
                    case BTN_DPAD_DOWN:
                        m_state.buttons[static_cast<i32>(GamepadButton::kDPadDown)] = event.value != 0;
                        break;
                    case BTN_DPAD_LEFT:
                        m_state.buttons[static_cast<i32>(GamepadButton::kDPadLeft)] = event.value != 0;
                        break;
                    case BTN_DPAD_RIGHT:
                        m_state.buttons[static_cast<i32>(GamepadButton::kDPadRight)] = event.value != 0;
                        break;
                    default:
                        break;
                    }
                }
                else if (event.type == EV_ABS)
                {
                    switch (event.code)
                    {
                    case ABS_X:
                        m_state.axis[static_cast<i32>(GamepadAxis::kLeftStickX)] = normalizeInputAxis(event.value);
                        m_state.buttons[static_cast<i32>(GamepadButton::kLeftStickLeft)] = event.value < -TriggerThreshold;
                        m_state.buttons[static_cast<i32>(GamepadButton::kLeftStickRight)] = event.value > TriggerThreshold;
                        break;
                    case ABS_Y:
                        m_state.axis[static_cast<i32>(GamepadAxis::kLeftStickY)] = normalizeInputAxis(event.value);
                        m_state.buttons[static_cast<i32>(GamepadButton::kLeftStickUp)] = event.value < -TriggerThreshold;
                        m_state.buttons[static_cast<i32>(GamepadButton::kLeftStickDown)] = event.value > TriggerThreshold;
                        break;
                    case ABS_Z:
                        m_state.axis[static_cast<i32>(GamepadAxis::kLeftTrigger)] = normalizeInputTrigger(event.value);
                        m_state.buttons[static_cast<i32>(GamepadButton::kLeftTrigger)] = event.value > 2;
                        break;
                    case ABS_RX:
                        m_state.axis[static_cast<i32>(GamepadAxis::kRightStickX)] = normalizeInputAxis(event.value);
                        m_state.buttons[static_cast<i32>(GamepadButton::kRightStickLeft)] = event.value < -TriggerThreshold;
                        m_state.buttons[static_cast<i32>(GamepadButton::kRightStickRight)] = event.value > TriggerThreshold;
                        break;
                    case ABS_RY:
                        m_state.axis[static_cast<i32>(GamepadAxis::kRightStickY)] = normalizeInputAxis(event.value);
                        m_state.buttons[static_cast<i32>(GamepadButton::kRightStickUp)] = event.value < -TriggerThreshold;
                        m_state.buttons[static_cast<i32>(GamepadButton::kRightStickDown)] = event.value > TriggerThreshold;
                        break;
                    case ABS_RZ:
                        m_state.axis[static_cast<i32>(GamepadAxis::kRightTrigger)] = normalizeInputTrigger(event.value);
                        m_state.buttons[static_cast<i32>(GamepadButton::kRightTrigger)] = event.value > 2;
                        break;
                    default:
                        break;
                    }
                }
            }

            return false;
        }
    };

    void LinuxInput::init()
    {
        if (InputSystem::Mouse == nullptr)
            InputSystem::Mouse = TkMemory::newObject<LinuxMouse>();
        if (InputSystem::Keyboard == nullptr)
            InputSystem::Keyboard = TkMemory::newObject<LinuxKeyboard>();
        s_foundGamepads = 0;
        s_lastUpdateTime = -1000.0f;
    }

    void LinuxInput::update()
    {
        const float time = static_cast<float>(System::getTimeSeconds());
        if (time - s_lastUpdateTime < 1.0f)
            return;

        PROFILE_NAMED("Input.ScanGamepads");
        detectGamepads();
        s_lastUpdateTime = time;

        for (i32 i = 0; i < s_foundGamepads; i++)
        {
            auto &inputDevice = s_inputDevices[i];

            bool duplicate = false;
            for (Gamepad *gamepad : InputSystem::Gamepads)
            {
                auto *linuxGamepad = static_cast<LinuxGamepad *>(gamepad);
                if (linuxGamepad != nullptr && linuxGamepad->m_dev == inputDevice.handler.c_str())
                {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate)
                continue;

            auto *linuxGamepad = TkMemory::newObject<LinuxGamepad>(inputDevice.uid, TkString(inputDevice.name.c_str()));
            if (linuxGamepad == nullptr)
                continue;
            linuxGamepad->m_dev = inputDevice.handler.c_str();
            InputSystem::Gamepads.add(linuxGamepad);
            InputSystem::onGamepadsChanged();
            LogInfo("Added gamepad '%s'", linuxGamepad->getName().getText());
        }
    }

    bool LinuxInput::processEvent(TkWindow *window, X11::XEvent &event)
    {
        auto *mouse = static_cast<LinuxMouse *>(InputSystem::Mouse);
        if (mouse != nullptr && mouse->processEvent(window, event))
            return true;

        auto *keyboard = static_cast<LinuxKeyboard *>(InputSystem::Keyboard);
        if (keyboard != nullptr && keyboard->processEvent(window, event))
            return true;

        return false;
    }
}

#endif
