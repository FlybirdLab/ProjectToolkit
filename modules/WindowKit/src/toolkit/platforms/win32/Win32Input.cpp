/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Win32Input.hpp"

#if TK_WINDOWS

#include "toolkit/input/Keyboard.hpp"
#include "toolkit/input/Mouse.hpp"
#include "toolkit/input/Gamepad.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/platforms/System.hpp"
#include <toolkit/types/StringView.hpp>

#define DIRECTINPUT_VERSION 0x0800
#include <xinput.h>

#include <toolkit/windowing/Window.hpp>
#include <toolkit/platforms/win32/Win32Window.hpp>

#define WINDOWS_GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define WINDOWS_GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

namespace toolkit {
    
    // Implementation of the keyboard device for Windows platform.
    class Win32Keyboard : public Keyboard
    {
    public:
        explicit Win32Keyboard()
            : Keyboard()
        {
        }

    public:
        bool wndProc(TkWindow *window, UINT msg, WPARAM wParam, LPARAM lParam);
    };

    // Implementation of the mouse device for Windows platform.
    class Win32Mouse : public Mouse
    {
    public:
        explicit Win32Mouse()
            : Mouse()
        {
        }

    public:
        bool wndProc(TkWindow *window, UINT msg, WPARAM wParam, LPARAM lParam);

    public:
        void setMousePosition(const Pos &newPosition) final override
        {
            ::SetCursorPos(static_cast<i32>(newPosition.x), static_cast<i32>(newPosition.y));

            onMouseMoved(newPosition);
        }
    };

    // Implementation of the gamepad device for Windows platform.
    class Win32Gamepad : public Gamepad
    {
    private:
        DWORD m_userIndex;

    public:
        explicit Win32Gamepad(DWORD userIndex);
        ~Win32Gamepad();

    public:
        // [Gamepad]
        void setVibration(const GamepadVibrationState &state) override;

    protected:
        // [Gamepad]
        bool updateState() override;
    };

    float XInputLastUpdateTime = 0;
    bool XInputGamepads[XUSER_MAX_COUNT] = {false};

    void Win32Input::init()
    {
        InputSystem::Mouse = TkMemory::newObject<Win32Mouse>();
        InputSystem::Keyboard = TkMemory::newObject<Win32Keyboard>();
    }

    void Win32Input::update()
    {
        // Update every second
        const float time = (float)System::getTimeSeconds();
        if (time - XInputLastUpdateTime < 1.0f)
            return;
        XInputLastUpdateTime = time;

        PROFILE_NAMED("Input.ScanGamepads");

        XINPUT_STATE state;
        for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
        {
            if (!XInputGamepads[i])
            {
                const DWORD result = XInputGetState(i, &state);
                if (result == ERROR_SUCCESS)
                {
                    auto *gamepad = TkMemory::newObject<Win32Gamepad>(TkMemory::getDefault(), i);
                    if (gamepad != nullptr)
                    {
                        XInputGamepads[i] = true;
                        InputSystem::Gamepads.add(gamepad);
                        InputSystem::onGamepadsChanged();
                    }
                }
            }
        }
    }

    bool Win32Input::wndProc(TkWindow *window, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        auto *mouse = static_cast<Win32Mouse *>(InputSystem::Mouse);
        if (mouse && mouse->wndProc(window, msg, wParam, lParam))
            return true;
        auto *keyboard = static_cast<Win32Keyboard *>(InputSystem::Keyboard);
        if (keyboard && keyboard->wndProc(window, msg, wParam, lParam))
            return true;
        return false;
    }

    bool Win32Keyboard::wndProc(TkWindow *window, const UINT msg, WPARAM wParam, LPARAM lParam)
    {
        bool result = false;

        // Handle keyboard related messages
        switch (msg)
        {
        case WM_CHAR:
        {
            // The current input event model carries one UTF-16 wchar. BMP characters are preserved;
            // surrogate-pair assembly for supplementary codepoints belongs in a future text layer.
            onCharInput(static_cast<wchar>(wParam), window);
            result = true;
            break;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            onKeyDown(static_cast<KeyboardKeys>(wParam), window);
            result = true;
            break;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            onKeyUp(static_cast<KeyboardKeys>(wParam), window);
            result = true;
            break;
        }
        }

        return result;
    }

    bool Win32Mouse::wndProc(TkWindow *window, const UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (window == nullptr || window->getNativeWindow() == nullptr || !window->getNativeWindow()->hasHWND())
            return false;

        bool result = false;

        // Get mouse position in screen coordinates
        POINT p;
        p.x = static_cast<LONG>(WINDOWS_GET_X_LPARAM(lParam));
        p.y = static_cast<LONG>(WINDOWS_GET_Y_LPARAM(lParam));
        ::ClientToScreen(window->getNativeWindow()->getHWND(), &p);
        const Pos mousePos(static_cast<float>(p.x), static_cast<float>(p.y));

        // Handle mouse related messages
        switch (msg)
        {
        case WM_MOUSEMOVE:
        {
            onMouseMove(mousePos, window);
            result = true;
            break;
        }
        case WM_NCMOUSEMOVE:
        {
            onMouseMove(mousePos, window);
            result = true;
            break;
        }
        case WM_MOUSELEAVE:
        {
            onMouseLeave(window);
            result = true;
            break;
        }
        case WM_LBUTTONDOWN:
        {
            onMouseDown(mousePos, MouseButton::kLeft, window);
            result = true;
            break;
        }
        case WM_RBUTTONDOWN:
        {
            onMouseDown(mousePos, MouseButton::kRight, window);
            result = true;
            break;
        }
        case WM_MBUTTONDOWN:
        {
            onMouseDown(mousePos, MouseButton::kMiddle, window);
            result = true;
            break;
        }
        case WM_XBUTTONDOWN:
        {
            const auto button = (HIWORD(wParam) & XBUTTON1) ? MouseButton::kExtended1 : MouseButton::kExtended2;
            onMouseDown(mousePos, button, window);
            result = true;
            break;
        }
        case WM_LBUTTONUP:
        {
            onMouseUp(mousePos, MouseButton::kLeft, window);
            result = true;
            break;
        }
        case WM_RBUTTONUP:
        {
            onMouseUp(mousePos, MouseButton::kRight, window);
            result = true;
            break;
        }
        case WM_MBUTTONUP:
        {
            onMouseUp(mousePos, MouseButton::kMiddle, window);
            result = true;
            break;
        }
        case WM_XBUTTONUP:
        {
            const auto button = (HIWORD(wParam) & XBUTTON1) ? MouseButton::kExtended1 : MouseButton::kExtended2;
            onMouseUp(mousePos, button, window);
            result = true;
            break;
        }
        case WM_LBUTTONDBLCLK:
        {
            onMouseDoubleClick(mousePos, MouseButton::kLeft, window);
            result = true;
            break;
        }
        case WM_RBUTTONDBLCLK:
        {
            onMouseDoubleClick(mousePos, MouseButton::kRight, window);
            result = true;
            break;
        }
        case WM_MBUTTONDBLCLK:
        {
            onMouseDoubleClick(mousePos, MouseButton::kMiddle, window);
            result = true;
            break;
        }
        case WM_MOUSEWHEEL:
        {
            const short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (delta != 0)
            {
                const float deltaNormalized = static_cast<float>(delta) / WHEEL_DELTA;
                POINT wheelPoint;
                wheelPoint.x = WINDOWS_GET_X_LPARAM(lParam);
                wheelPoint.y = WINDOWS_GET_Y_LPARAM(lParam);
                onMouseWheel(Pos(static_cast<float>(wheelPoint.x), static_cast<float>(wheelPoint.y)), deltaNormalized, window);
            }
            result = true;
            break;
        }
        }

        return result;
    }

    float normalizeXInputAxis(const i16 axisVal)
    {
        // Normalize [-32768..32767] -> [-1..1]
        const float norm = axisVal <= 0 ? 32768.0f : 32767.0f;
        return float(axisVal) / norm;
    }

    Win32Gamepad::Win32Gamepad(DWORD userIndex)
        : Gamepad(TkUuid(0, 0, 0, userIndex), "XInput Gamepad"), m_userIndex(userIndex)
    {
        XInputGamepads[m_userIndex] = true;
    }

    Win32Gamepad::~Win32Gamepad()
    {
        XInputGamepads[m_userIndex] = false;
    }

    void Win32Gamepad::setVibration(const GamepadVibrationState &state)
    {
        Gamepad::setVibration(state);

        XINPUT_VIBRATION xInputVibration;
        const float leftMotor = TkMath::saturate((state.leftLarge + state.leftSmall) * 0.5f);
        const float rightMotor = TkMath::saturate((state.rightLarge + state.rightSmall) * 0.5f);
        xInputVibration.wLeftMotorSpeed = static_cast<WORD>(leftMotor * 65535.0f);
        xInputVibration.wRightMotorSpeed = static_cast<WORD>(rightMotor * 65535.0f);
        XInputSetState(m_userIndex, &xInputVibration);
    }

    bool Win32Gamepad::updateState()
    {
        // Gather XInput device state
        XINPUT_STATE inputState;
        if (XInputGetState(m_userIndex, &inputState) != ERROR_SUCCESS)
        {
            return true;
        }

        // Process buttons state
        m_state.buttons[(i32)GamepadButton::kA] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_A);
        m_state.buttons[(i32)GamepadButton::kB] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_B);
        m_state.buttons[(i32)GamepadButton::kX] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_X);
        m_state.buttons[(i32)GamepadButton::kY] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
        m_state.buttons[(i32)GamepadButton::kLeftShoulder] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
        m_state.buttons[(i32)GamepadButton::kRightShoulder] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
        m_state.buttons[(i32)GamepadButton::kBack] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
        m_state.buttons[(i32)GamepadButton::kStart] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_START);
        m_state.buttons[(i32)GamepadButton::kLeftThumb] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
        m_state.buttons[(i32)GamepadButton::kRightThumb] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
        m_state.buttons[(i32)GamepadButton::kLeftTrigger] = inputState.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
        m_state.buttons[(i32)GamepadButton::kRightTrigger] = inputState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
        m_state.buttons[(i32)GamepadButton::kDPadUp] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
        m_state.buttons[(i32)GamepadButton::kDPadDown] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
        m_state.buttons[(i32)GamepadButton::kDPadLeft] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
        m_state.buttons[(i32)GamepadButton::kDPadRight] = !!(inputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
        m_state.buttons[(i32)GamepadButton::kLeftStickUp] = inputState.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
        m_state.buttons[(i32)GamepadButton::kLeftStickDown] = inputState.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
        m_state.buttons[(i32)GamepadButton::kLeftStickLeft] = inputState.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
        m_state.buttons[(i32)GamepadButton::kLeftStickRight] = inputState.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
        m_state.buttons[(i32)GamepadButton::kRightStickUp] = inputState.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
        m_state.buttons[(i32)GamepadButton::kRightStickDown] = inputState.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
        m_state.buttons[(i32)GamepadButton::kRightStickLeft] = inputState.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
        m_state.buttons[(i32)GamepadButton::kRightStickRight] = inputState.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

        // Process axes state
        m_state.axis[(i32)GamepadAxis::kLeftStickX] = normalizeXInputAxis(inputState.Gamepad.sThumbLX);
        m_state.axis[(i32)GamepadAxis::kLeftStickY] = normalizeXInputAxis(inputState.Gamepad.sThumbLY);
        m_state.axis[(i32)GamepadAxis::kRightStickX] = normalizeXInputAxis(inputState.Gamepad.sThumbRX);
        m_state.axis[(i32)GamepadAxis::kRightStickY] = normalizeXInputAxis(inputState.Gamepad.sThumbRY);
        m_state.axis[(i32)GamepadAxis::kLeftTrigger] = inputState.Gamepad.bLeftTrigger / 255.0f;
        m_state.axis[(i32)GamepadAxis::kRightTrigger] = inputState.Gamepad.bRightTrigger / 255.0f;

        return false;
    }

}
#endif
