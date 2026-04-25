/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include "toolkit/base/Math.hpp"
#include "toolkit/types/String.hpp"
#include <toolkit/memory/Memory.hpp>
#include "toolkit/collection/Array.hpp"

namespace toolkit
{
    class TkWindow;

    enum class KeyboardKeys;
    enum class MouseButton;
    enum class InputEventType
    {
        kChar,
        kKeyDown,
        kKeyUp,
        kMouseDown,
        kMouseUp,
        kMouseDoubleClick,
        kMouseWheel,
        kMouseMove,
        kMouseLeave,
        kTouchDown,
        kTouchMove,
        kTouchUp,
    };

    struct InputEvent
    {
        InputEventType Type;
        TkWindow *Target;

        union
        {
            struct
            {
                wchar c;
            } charData;

            struct
            {
                KeyboardKeys key;
            } keyData;

            struct
            {
                MouseButton button;
                Pos pos;
            } mouseData;

            struct
            {
                float wheelDelta;
                Pos pos;
            } mouseWheelData;

            struct
            {
                Pos pos;
                i32 pointerId;
            } touchData;
        };

        InputEvent()
            : Type(InputEventType::kChar), Target(nullptr), charData{0}
        {
        }
        InputEvent(const InputEvent &e)
        {
            TkMemory::memoryCopy(this, &e, sizeof(InputEvent));
        }
    };
    typedef TkArray<InputEvent> EventQueue;

    // Base class for all input device objects.
    class TK_API InputDevice
    {
    public:
    protected:
        TkString m_name;
        EventQueue m_queue;

        explicit InputDevice(const TkStringView &name)
            : m_name(name)
        {
        }

    public:
        // Gets the name.
        TK_FORCE_INLINE const TkString &getName() const
        {
            return m_name;
        }

        // Resets the input device state. Called when application looses focus.
        virtual void resetState()
        {
            m_queue.clear();
        }

        // Captures the input since the last call and triggers the input events.
        virtual bool update(EventQueue &queue)
        {
            if (updateState())
                return true;
            queue.add(m_queue);
            m_queue.clear();
            return false;
        }

        // Updates only the current state of the device.
        virtual bool updateState()
        {
            return false;
        }
    };

    enum class KeyboardKeys
    {
        kNone = 0,
        kBackspace = 0x08,
        kTab = 0x09,
        kClear = 0x0C,
        kReturn = 0x0D,
        kShift = 0x10,
        kControl = 0x11,
        kAlt = 0x12,
        kPause = 0x13,
        kCapital = 0x14,
        kKana = 0x15,
        kHangul = 0x15,
        kJunja = 0x17,
        kFinal = 0x18,
        kHanja = 0x19,
        kKanji = 0x19,
        kEscape = 0x1B,
        kConvert = 0x1C,
        kNonconvert = 0x1D,
        kAccept = 0x1E,
        kModechange = 0x1F,
        kSpacebar = 0x20,
        kPageUp = 0x21,
        kPageDown = 0x22,
        kEnd = 0x23,
        kHome = 0x24,
        kArrowLeft = 0x25,
        kArrowUp = 0x26,
        kArrowRight = 0x27,
        kArrowDown = 0x28,
        kSelect = 0x29,
        kPrint = 0x2A,
        kExecute = 0x2B,
        kPrintScreen = 0x2C,
        kInsert = 0x2D,
        kDelete = 0x2E,
        kHelp = 0x2F,
        kAlpha0 = 0x30,
        kAlpha1 = 0x31,
        kAlpha2 = 0x32,
        kAlpha3 = 0x33,
        kAlpha4 = 0x34,
        kAlpha5 = 0x35,
        kAlpha6 = 0x36,
        kAlpha7 = 0x37,
        kAlpha8 = 0x38,
        kAlpha9 = 0x39,
        kA = 0x41,
        kB = 0x42,
        kC = 0x43,
        kD = 0x44,
        kE = 0x45,
        kF = 0x46,
        kG = 0x47,
        kH = 0x48,
        kI = 0x49,
        kJ = 0x4A,
        kK = 0x4B,
        kL = 0x4C,
        kM = 0x4D,
        kN = 0x4E,
        kO = 0x4F,
        kP = 0x50,
        kQ = 0x51,
        kR = 0x52,
        kS = 0x53,
        kT = 0x54,
        kU = 0x55,
        kV = 0x56,
        kW = 0x57,
        kX = 0x58,
        kY = 0x59,
        kZ = 0x5A,
        kLeftWindows = 0x5B,
        kRightWindows = 0x5C,
        kApplications = 0x5D,
        kSleep = 0x5F,
        kNumpad0 = 0x60,
        kNumpad1 = 0x61,
        kNumpad2 = 0x62,
        kNumpad3 = 0x63,
        kNumpad4 = 0x64,
        kNumpad5 = 0x65,
        kNumpad6 = 0x66,
        kNumpad7 = 0x67,
        kNumpad8 = 0x68,
        kNumpad9 = 0x69,
        kNumpadMultiply = 0x6A,
        kNumpadAdd = 0x6B,
        kNumpadSeparator = 0x6C,
        kNumpadSubtract = 0x6D,
        kNumpadDecimal = 0x6E,
        kNumpadDivide = 0x6F,
        kF1 = 0x70,
        kF2 = 0x71,
        kF3 = 0x72,
        kF4 = 0x73,
        kF5 = 0x74,
        kF6 = 0x75,
        kF7 = 0x76,
        kF8 = 0x77,
        kF9 = 0x78,
        kF10 = 0x79,
        kF11 = 0x7A,
        kF12 = 0x7B,
        kF13 = 0x7C,
        kF14 = 0x7D,
        kF15 = 0x7E,
        kF16 = 0x7F,
        kF17 = 0x80,
        kF18 = 0x81,
        kF19 = 0x82,
        kF20 = 0x83,
        kF21 = 0x84,
        kF22 = 0x85,
        kF23 = 0x86,
        kF24 = 0x87,
        kNumlock = 0x90,
        kScroll = 0x91,
        kLeftMenu = 0xA4,
        kRightMenu = 0xA5,
        kBrowserBack = 0xA6,
        kBrowserForward = 0xA7,
        kBrowserRefresh = 0xA8,
        kBrowserStop = 0xA9,
        kBrowserSearch = 0xAA,
        kBrowserFavorites = 0xAB,
        kBrowserHome = 0xAC,
        kVolumeMute = 0xAD,
        kVolumeDown = 0xAE,
        kVolumeUp = 0xAF,
        kMediaNextTrack = 0xB0,
        kMediaPrevTrack = 0xB1,
        kMediaStop = 0xB2,
        kMediaPlayPause = 0xB3,
        kLaunchMail = 0xB4,
        kLaunchMediaSelect = 0xB5,
        kLaunchApp1 = 0xB6,
        kLaunchApp2 = 0xB7,
        kColon = 0xBA,
        kPlus = 0xBB,
        kComma = 0xBC,
        kMinus = 0xBD,
        kPeriod = 0xBE,
        kSlash = 0xBF,
        kBackQuote = 0xC0,
        kLeftBracket = 0xDB,
        kBackslash = 0xDC,
        kRightBracket = 0xDD,
        kQuote = 0xDE,
        kOem8 = 0xDF,
        kOem102 = 0xE2,
        kProcesskey = 0xE5,
        kPacket = 0xE7,
        kAttn = 0xF6,
        kCrsel = 0xF7,
        kExsel = 0xF8,
        kEreof = 0xF9,
        kPlay = 0xFA,
        kZoom = 0xFB,
        kPa1 = 0xFD,
        kOemClear = 0xFE,
        kMax
    };

    enum class CursorLockMode
    {
        kNone = 0,
        kLocked = 1,
        kClipped = 2,
    };

    enum class MouseButton
    {
        kNone = 0,
        kLeft = 1,
        kMiddle = 2,
        kRight = 3,
        kExtended1 = 4,
        kExtended2 = 5,
        kMax
    };

    TK_FORCE_INLINE bool isValidKey(const KeyboardKeys key)
    {
        const i32 value = static_cast<i32>(key);
        return value > static_cast<i32>(KeyboardKeys::kNone) && value < static_cast<i32>(KeyboardKeys::kMax);
    }

    TK_FORCE_INLINE bool isValidMouseButton(const MouseButton button)
    {
        const i32 value = static_cast<i32>(button);
        return value > static_cast<i32>(MouseButton::kNone) && value < static_cast<i32>(MouseButton::kMax);
    }

// The maximum amount of supported gamepads connected at once.
#define MAX_GAMEPADS 8

    // Axis for gamepad.
    enum class GamepadAxis
    {
        kNone = 0,
        kLeftStickX = 1,
        kLeftStickY = 2,
        kRightStickX = 3,
        kRightStickY = 4,
        kLeftTrigger = 5,
        kRightTrigger = 6,
        kMax
    };

    // Buttons for gamepad.
    enum class GamepadButton
    {
        kNone = 0,
        kDPadUp = 1,
        kDPadDown = 2,
        kDPadLeft = 3,
        kDPadRight = 4,
        kStart = 5,
        kBack = 6,
        kLeftThumb = 7,
        kRightThumb = 8,
        kLeftShoulder = 9,
        kRightShoulder = 10,
        kLeftTrigger = 11,
        kRightTrigger = 12,
        kA = 13,
        kB = 14,
        kX = 15,
        kY = 16,
        kLeftStickUp = 17,
        kLeftStickDown = 18,
        kLeftStickLeft = 19,
        kLeftStickRight = 20,
        kRightStickUp = 21,
        kRightStickDown = 22,
        kRightStickLeft = 23,
        kRightStickRight = 24,
        kMax
    };

    TK_FORCE_INLINE bool isValidGamepadAxis(const GamepadAxis axis)
    {
        const i32 value = static_cast<i32>(axis);
        return value > static_cast<i32>(GamepadAxis::kNone) && value < static_cast<i32>(GamepadAxis::kMax);
    }

    TK_FORCE_INLINE bool isValidGamepadButton(const GamepadButton button)
    {
        const i32 value = static_cast<i32>(button);
        return value > static_cast<i32>(GamepadButton::kNone) && value < static_cast<i32>(GamepadButton::kMax);
    }

    // The input action event trigger modes.
    enum class InputActionMode
    {
        kPressing = 0,
        kPress = 1,
        kRelease = 2,
    };

    // The input action event phases.
    enum class InputActionState
    {
        kNone = 0,
        kWaiting = 1,
        kPressing = 2,
        kPress = 3,
        kRelease = 4,
    };

    // The input gamepad index.
    enum class InputGamepadIndex
    {
        kAll = -1,
        kGamepad0 = 0,
        kGamepad1 = 1,
        kGamepad2 = 2,
        kGamepad3 = 3,
        kGamepad4 = 4,
        kGamepad5 = 5,
    };

    // The input axes types.
    enum class InputAxisType
    {
        kMouseX = 0,
        kMouseY = 1,
        kMouseWheel = 2,
        kGamepadLeftStickX = 3,
        kGamepadLeftStickY = 4,
        kGamepadRightStickX = 5,
        kGamepadRightStickY = 6,
        kGamepadLeftTrigger = 7,
        kGamepadRightTrigger = 8,
        kKeyboardOnly = 9,
        kGamepadDPadX = 10,
        kGamepadDPadY = 11,
    };

}
