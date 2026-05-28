/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "LinuxInput.hpp"

#if TK_LINUX

#include "LinuxWindow.hpp"
#include "LinuxWindowSys.hpp"
#include <toolkit/input/Gamepad.hpp>
#include <toolkit/input/Keyboard.hpp>
#include <toolkit/input/Mouse.hpp>
#include <toolkit/platforms/System.hpp>

#include <errno.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace toolkit
{
    class LinuxKeyboard final : public Keyboard
    {
    public:
        bool handleEvent(TkWindow *window, X11::XEvent &event);
    };

    class LinuxMouse final : public Mouse
    {
    public:
        void setMousePosition(const Pos &newPosition) override
        {
            X11::Display *display = LinuxWindowSys::getDisplay();
            if (!display)
                return;
            X11::XWarpPointer(display, 0, X11::XDefaultRootWindow(display), 0, 0, 0, 0, static_cast<i32>(newPosition.x), static_cast<i32>(newPosition.y));
            X11::XFlush(display);
            onMouseMoved(newPosition);
        }

        bool handleEvent(TkWindow *window, X11::XEvent &event);
    };

    class LinuxGamepad final : public Gamepad
    {
    public:
        LinuxGamepad(i32 index, i32 fd, const TkString &name)
            : Gamepad(TkUuid(0x4c696e75, 0x784a7300u | static_cast<u32>(index), 0, static_cast<u32>(index)), name),
              m_index(index),
              m_fd(fd)
        {
        }

        ~LinuxGamepad()
        {
            if (m_fd >= 0)
                close(m_fd);
        }

        i32 getIndex() const
        {
            return m_index;
        }

    protected:
        bool updateState() override;

    private:
        void setButton(u8 index, bool pressed);
        void setAxis(u8 index, i16 value);

    private:
        i32 m_index = -1;
        i32 m_fd = -1;
    };

    static float s_lastGamepadScanTime = -1.0f;

    static KeyboardKeys mapKeySym(X11::KeySym key)
    {
        if (key >= XK_A && key <= XK_Z)
            return static_cast<KeyboardKeys>(static_cast<i32>(KeyboardKeys::kA) + static_cast<i32>(key - XK_A));
        if (key >= XK_a && key <= XK_z)
            return static_cast<KeyboardKeys>(static_cast<i32>(KeyboardKeys::kA) + static_cast<i32>(key - XK_a));
        if (key >= XK_0 && key <= XK_9)
            return static_cast<KeyboardKeys>(static_cast<i32>(KeyboardKeys::kAlpha0) + static_cast<i32>(key - XK_0));
        if (key >= XK_KP_0 && key <= XK_KP_9)
            return static_cast<KeyboardKeys>(static_cast<i32>(KeyboardKeys::kNumpad0) + static_cast<i32>(key - XK_KP_0));
        if (key >= XK_F1 && key <= XK_F24)
            return static_cast<KeyboardKeys>(static_cast<i32>(KeyboardKeys::kF1) + static_cast<i32>(key - XK_F1));

        switch (key)
        {
        case XK_BackSpace:
            return KeyboardKeys::kBackspace;
        case XK_Tab:
            return KeyboardKeys::kTab;
        case XK_Return:
        case XK_KP_Enter:
            return KeyboardKeys::kReturn;
        case XK_Escape:
            return KeyboardKeys::kEscape;
        case XK_space:
            return KeyboardKeys::kSpacebar;
        case XK_Page_Up:
            return KeyboardKeys::kPageUp;
        case XK_Page_Down:
            return KeyboardKeys::kPageDown;
        case XK_End:
            return KeyboardKeys::kEnd;
        case XK_Home:
            return KeyboardKeys::kHome;
        case XK_Left:
            return KeyboardKeys::kArrowLeft;
        case XK_Up:
            return KeyboardKeys::kArrowUp;
        case XK_Right:
            return KeyboardKeys::kArrowRight;
        case XK_Down:
            return KeyboardKeys::kArrowDown;
        case XK_Insert:
            return KeyboardKeys::kInsert;
        case XK_Delete:
            return KeyboardKeys::kDelete;
        case XK_Shift_L:
        case XK_Shift_R:
            return KeyboardKeys::kShift;
        case XK_Control_L:
        case XK_Control_R:
            return KeyboardKeys::kControl;
        case XK_Alt_L:
        case XK_Alt_R:
            return KeyboardKeys::kAlt;
        case XK_Num_Lock:
            return KeyboardKeys::kNumlock;
        case XK_Scroll_Lock:
            return KeyboardKeys::kScroll;
        case XK_Caps_Lock:
            return KeyboardKeys::kCapital;
        case XK_KP_Multiply:
            return KeyboardKeys::kNumpadMultiply;
        case XK_KP_Add:
            return KeyboardKeys::kNumpadAdd;
        case XK_KP_Subtract:
            return KeyboardKeys::kNumpadSubtract;
        case XK_KP_Decimal:
            return KeyboardKeys::kNumpadDecimal;
        case XK_KP_Divide:
            return KeyboardKeys::kNumpadDivide;
        case XK_semicolon:
            return KeyboardKeys::kColon;
        case XK_equal:
            return KeyboardKeys::kPlus;
        case XK_comma:
            return KeyboardKeys::kComma;
        case XK_minus:
            return KeyboardKeys::kMinus;
        case XK_period:
            return KeyboardKeys::kPeriod;
        case XK_slash:
            return KeyboardKeys::kSlash;
        case XK_grave:
            return KeyboardKeys::kBackQuote;
        case XK_bracketleft:
            return KeyboardKeys::kLeftBracket;
        case XK_backslash:
            return KeyboardKeys::kBackslash;
        case XK_bracketright:
            return KeyboardKeys::kRightBracket;
        case XK_apostrophe:
            return KeyboardKeys::kQuote;
        default:
            return KeyboardKeys::kNone;
        }
    }

    static Pos getScreenMousePos(X11::Display *display, X11::Window window)
    {
        X11::Window root = 0;
        X11::Window child = 0;
        i32 rootX = 0;
        i32 rootY = 0;
        i32 winX = 0;
        i32 winY = 0;
        u32 mask = 0;
        if (!display || !X11::XQueryPointer(display, window, &root, &child, &rootX, &rootY, &winX, &winY, &mask))
            return Pos::Minimum;
        return Pos(static_cast<float>(rootX), static_cast<float>(rootY));
    }

    static f32 normalizeAxis(i16 value)
    {
        const f32 norm = value < 0 ? 32768.0f : 32767.0f;
        return static_cast<f32>(value) / norm;
    }

    static bool hasGamepadIndex(i32 index)
    {
        for (Gamepad *gamepad : InputSystem::Gamepads)
        {
            auto *linuxGamepad = static_cast<LinuxGamepad *>(gamepad);
            if (linuxGamepad && linuxGamepad->getIndex() == index)
                return true;
        }
        return false;
    }

    static void detectGamepads()
    {
        const float now = static_cast<float>(System::getTimeSeconds());
        if (s_lastGamepadScanTime >= 0.0f && now - s_lastGamepadScanTime < 1.0f)
            return;
        s_lastGamepadScanTime = now;

        for (i32 i = 0; i < MAX_GAMEPADS; ++i)
        {
            if (hasGamepadIndex(i))
                continue;

            const TkString path = TkString::format("/dev/input/js%i", i);
            const i32 fd = open(path.getText(), O_RDONLY | O_NONBLOCK);
            if (fd < 0)
                continue;

            char name[128] {};
            if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0 || name[0] == '\0')
                snprintf(name, sizeof(name), "Linux Gamepad %i", i);

            auto *gamepad = TkMemory::newObject<LinuxGamepad>(TkMemory::getDefault(), i, fd, TkString(name));
            if (gamepad)
            {
                InputSystem::Gamepads.add(gamepad);
                InputSystem::onGamepadsChanged();
            }
            else
                close(fd);
        }
    }

    void LinuxInput::init()
    {
        if (!InputSystem::Mouse)
            InputSystem::Mouse = TkMemory::newObject<LinuxMouse>();
        if (!InputSystem::Keyboard)
            InputSystem::Keyboard = TkMemory::newObject<LinuxKeyboard>();
    }

    void LinuxInput::update()
    {
        detectGamepads();
    }

    bool LinuxInput::handleEvent(TkWindow *window, X11::XEvent &event)
    {
        bool handled = false;
        auto *mouse = static_cast<LinuxMouse *>(InputSystem::Mouse);
        if (mouse)
            handled = mouse->handleEvent(window, event) || handled;
        auto *keyboard = static_cast<LinuxKeyboard *>(InputSystem::Keyboard);
        if (keyboard)
            handled = keyboard->handleEvent(window, event) || handled;
        return handled;
    }

    bool LinuxKeyboard::handleEvent(TkWindow *window, X11::XEvent &event)
    {
        if (event.type != KeyPress && event.type != KeyRelease)
            return false;

        X11::KeySym keySym = 0;
        char text[8] = {};
        const i32 textLength = X11::XLookupString(&event.xkey, text, sizeof(text), &keySym, nullptr);
        const KeyboardKeys key = mapKeySym(keySym);

        if (event.type == KeyPress)
        {
            if (isValidKey(key))
                onKeyDown(key, window);
            if (textLength == 1 && static_cast<unsigned char>(text[0]) >= 32)
                onCharInput(static_cast<wchar>(text[0]), window);
        }
        else
        {
            if (isValidKey(key))
                onKeyUp(key, window);
        }
        return true;
    }

    bool LinuxMouse::handleEvent(TkWindow *window, X11::XEvent &event)
    {
        if (!window || !window->getNativeWindow())
            return false;

        X11::Display *display = LinuxWindowSys::getDisplay();
        const X11::Window handle = window->getNativeWindow()->getXWindow();
        const Pos pos = event.type == MotionNotify
                            ? Pos(static_cast<float>(event.xmotion.x_root), static_cast<float>(event.xmotion.y_root))
                            : getScreenMousePos(display, handle);

        switch (event.type)
        {
        case MotionNotify:
            onMouseMove(pos, window);
            return true;
        case LeaveNotify:
            onMouseLeave(window);
            return true;
        case ButtonPress:
            if (event.xbutton.button == Button4 || event.xbutton.button == Button5)
            {
                onMouseWheel(pos, event.xbutton.button == Button4 ? 1.0f : -1.0f, window);
                return true;
            }
            if (event.xbutton.button == Button1)
                onMouseDown(pos, MouseButton::kLeft, window);
            else if (event.xbutton.button == Button2)
                onMouseDown(pos, MouseButton::kMiddle, window);
            else if (event.xbutton.button == Button3)
                onMouseDown(pos, MouseButton::kRight, window);
            else if (event.xbutton.button == 8)
                onMouseDown(pos, MouseButton::kExtended1, window);
            else if (event.xbutton.button == 9)
                onMouseDown(pos, MouseButton::kExtended2, window);
            return true;
        case ButtonRelease:
            if (event.xbutton.button == Button4 || event.xbutton.button == Button5)
                return true;
            if (event.xbutton.button == Button1)
                onMouseUp(pos, MouseButton::kLeft, window);
            else if (event.xbutton.button == Button2)
                onMouseUp(pos, MouseButton::kMiddle, window);
            else if (event.xbutton.button == Button3)
                onMouseUp(pos, MouseButton::kRight, window);
            else if (event.xbutton.button == 8)
                onMouseUp(pos, MouseButton::kExtended1, window);
            else if (event.xbutton.button == 9)
                onMouseUp(pos, MouseButton::kExtended2, window);
            return true;
        default:
            return false;
        }
    }

    void LinuxGamepad::setButton(u8 index, bool pressed)
    {
        switch (index)
        {
        case 0:
            m_state.buttons[static_cast<i32>(GamepadButton::kA)] = pressed;
            break;
        case 1:
            m_state.buttons[static_cast<i32>(GamepadButton::kB)] = pressed;
            break;
        case 2:
            m_state.buttons[static_cast<i32>(GamepadButton::kX)] = pressed;
            break;
        case 3:
            m_state.buttons[static_cast<i32>(GamepadButton::kY)] = pressed;
            break;
        case 4:
            m_state.buttons[static_cast<i32>(GamepadButton::kLeftShoulder)] = pressed;
            break;
        case 5:
            m_state.buttons[static_cast<i32>(GamepadButton::kRightShoulder)] = pressed;
            break;
        case 6:
            m_state.buttons[static_cast<i32>(GamepadButton::kBack)] = pressed;
            break;
        case 7:
            m_state.buttons[static_cast<i32>(GamepadButton::kStart)] = pressed;
            break;
        case 8:
            m_state.buttons[static_cast<i32>(GamepadButton::kLeftThumb)] = pressed;
            break;
        case 9:
            m_state.buttons[static_cast<i32>(GamepadButton::kRightThumb)] = pressed;
            break;
        default:
            break;
        }
    }

    void LinuxGamepad::setAxis(u8 index, i16 value)
    {
        const f32 normalized = normalizeAxis(value);
        switch (index)
        {
        case 0:
            m_state.axis[static_cast<i32>(GamepadAxis::kLeftStickX)] = normalized;
            m_state.buttons[static_cast<i32>(GamepadButton::kLeftStickLeft)] = normalized < -0.5f;
            m_state.buttons[static_cast<i32>(GamepadButton::kLeftStickRight)] = normalized > 0.5f;
            break;
        case 1:
            m_state.axis[static_cast<i32>(GamepadAxis::kLeftStickY)] = -normalized;
            m_state.buttons[static_cast<i32>(GamepadButton::kLeftStickUp)] = normalized < -0.5f;
            m_state.buttons[static_cast<i32>(GamepadButton::kLeftStickDown)] = normalized > 0.5f;
            break;
        case 2:
            m_state.axis[static_cast<i32>(GamepadAxis::kLeftTrigger)] = (normalized + 1.0f) * 0.5f;
            m_state.buttons[static_cast<i32>(GamepadButton::kLeftTrigger)] = m_state.axis[static_cast<i32>(GamepadAxis::kLeftTrigger)] > 0.5f;
            break;
        case 3:
            m_state.axis[static_cast<i32>(GamepadAxis::kRightStickX)] = normalized;
            m_state.buttons[static_cast<i32>(GamepadButton::kRightStickLeft)] = normalized < -0.5f;
            m_state.buttons[static_cast<i32>(GamepadButton::kRightStickRight)] = normalized > 0.5f;
            break;
        case 4:
            m_state.axis[static_cast<i32>(GamepadAxis::kRightStickY)] = -normalized;
            m_state.buttons[static_cast<i32>(GamepadButton::kRightStickUp)] = normalized < -0.5f;
            m_state.buttons[static_cast<i32>(GamepadButton::kRightStickDown)] = normalized > 0.5f;
            break;
        case 5:
            m_state.axis[static_cast<i32>(GamepadAxis::kRightTrigger)] = (normalized + 1.0f) * 0.5f;
            m_state.buttons[static_cast<i32>(GamepadButton::kRightTrigger)] = m_state.axis[static_cast<i32>(GamepadAxis::kRightTrigger)] > 0.5f;
            break;
        case 6:
            m_state.buttons[static_cast<i32>(GamepadButton::kDPadLeft)] = normalized < -0.5f;
            m_state.buttons[static_cast<i32>(GamepadButton::kDPadRight)] = normalized > 0.5f;
            break;
        case 7:
            m_state.buttons[static_cast<i32>(GamepadButton::kDPadUp)] = normalized < -0.5f;
            m_state.buttons[static_cast<i32>(GamepadButton::kDPadDown)] = normalized > 0.5f;
            break;
        default:
            break;
        }
    }

    bool LinuxGamepad::updateState()
    {
        if (m_fd < 0)
            return true;

        js_event event {};
        ssize_t bytesRead = 0;
        while ((bytesRead = read(m_fd, &event, sizeof(event))) == sizeof(event))
        {
            const u8 type = event.type & ~JS_EVENT_INIT;
            if (type == JS_EVENT_BUTTON)
                setButton(event.number, event.value != 0);
            else if (type == JS_EVENT_AXIS)
                setAxis(event.number, event.value);
        }

        if (bytesRead < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
            return false;
        return true;
    }
}

#endif
