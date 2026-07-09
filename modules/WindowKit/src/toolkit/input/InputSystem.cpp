/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "InputSystem.hpp"

#include "Gamepad.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"
#include "toolkit/collection/HashMap.hpp"
#include "toolkit/base/Function.hpp"
#include "toolkit/collection/Array.hpp"
#include "toolkit/debug/Profiler.hpp"
#include "toolkit/types/StringView.hpp"
#include "toolkit/windowing/WindowSystem.hpp"
#include "toolkit/windowing/Window.hpp"
#include "toolkit/platforms/Screen.hpp"

namespace toolkit
{
    struct AxisEvaluation
    {
        float rawValue;
        float value;
        float prevKeyValue;
        bool used;
    };

    struct ActionData
    {
        bool active = false;
        u64 frameIndex = 0;
        InputActionState state = InputActionState::kWaiting;
    };

    struct AxisData
    {
        float value = 0.0f;
        float valueRaw = 0.0f;
        float prevValue = 0.0f;
        float prevKeyValue = 0.0f;
        u64 frameIndex = 0;
    };

    static TkHashMap<TkString, ActionData> Actions;
    static TkHashMap<TkString, AxisData> Axes;
    static bool s_GamepadsChanged = true;
    static u64 s_FrameIndex = 0;
    static TkArray<AxisEvaluation> AxesValues;
    static EventQueue InputEvents;

    static i32 getActionStatePriority(const InputActionState state)
    {
        switch (state)
        {
        case InputActionState::kRelease:
            return 4;
        case InputActionState::kPress:
            return 3;
        case InputActionState::kPressing:
            return 2;
        case InputActionState::kWaiting:
            return 1;
        case InputActionState::kNone:
        default:
            return 0;
        }
    }

    static TkArray<TkWindow *> getInputWindowsSnapshot()
    {
        return WindowSystem::getWindowsSnapshot();
    }

    static TkWindow *findPrimaryInputWindow(const TkArray<TkWindow *> &windows)
    {
        for (auto *window : windows)
        {
            if (window && window->getParams().allowInput && window->isFocused())
                return window;
        }

        for (auto *window : windows)
        {
            if (window && window->getParams().allowInput && !window->isClosed())
                return window;
        }

        return nullptr;
    }

    static bool hasInputFocus()
    {
        const auto windows = getInputWindowsSnapshot();
        return findPrimaryInputWindow(windows) != nullptr;
    }

    static Pos screenToInputPosition(const Pos &screenPos)
    {
        const auto windows = getInputWindowsSnapshot();
        if (auto *window = findPrimaryInputWindow(windows))
            return window->screenToClient(screenPos);

        return Pos::Minimum;
    }

    static Pos inputToScreenPosition(const Pos &inputPos)
    {
        const auto windows = getInputWindowsSnapshot();
        if (auto *window = findPrimaryInputWindow(windows))
            return window->clientToScreen(inputPos);

        return Pos::Minimum;
    }

    Mouse *InputSystem::Mouse = nullptr;
    Keyboard *InputSystem::Keyboard = nullptr;
    TkArray<Gamepad *> InputSystem::Gamepads;
    TkAction<> InputSystem::GamepadsChanged;
    TkArray<InputDevice *> InputSystem::CustomDevices;

    TkAction<wchar> InputSystem::CharInput;
    TkAction<KeyboardKeys> InputSystem::KeyDown;
    TkAction<KeyboardKeys> InputSystem::KeyUp;
    TkAction<const Pos &, MouseButton> InputSystem::MouseDown;
    TkAction<const Pos &, MouseButton> InputSystem::MouseUp;
    TkAction<const Pos &, MouseButton> InputSystem::MouseDoubleClick;
    TkAction<const Pos &, float> InputSystem::MouseWheel;
    TkAction<const Pos &> InputSystem::MouseMove;
    TkAction<> InputSystem::MouseLeave;
    TkAction<const Pos &, i32> InputSystem::TouchDown;
    TkAction<const Pos &, i32> InputSystem::TouchMove;
    TkAction<const Pos &, i32> InputSystem::TouchUp;
    TkAction<TkStringView, InputActionState> InputSystem::ActionTriggered;
    TkAction<TkStringView> InputSystem::AxisValueChanged;
    TkArray<ActionConfig> InputSystem::ActionMappings;
    TkArray<AxisConfig> InputSystem::AxisMappings;

    // void InputSettings::Apply()
    // {
    //     InputSystem::ActionMappings = ActionMappings;
    //     InputSystem::AxisMappings = AxisMappings;
    // }

    // void InputSettings::Deserialize(DeserializeStream &stream, ISerializeModifier *modifier)
    // {
    //     const auto actionMappings = stream.FindMember("ActionMappings");
    //     if (actionMappings != stream.MemberEnd())
    //     {
    //         auto &actionMappingsArray = actionMappings->value;
    //         if (actionMappingsArray.IsArray())
    //         {
    //             ActionMappings.resize(actionMappingsArray.size(), false);
    //             for (ui32 i = 0; i < actionMappingsArray.size(); i++)
    //             {
    //                 auto &v = actionMappingsArray[i];
    //                 if (!v.IsObject())
    //                     continue;

    //                 ActionConfig &config = ActionMappings[i];
    //                 config.name = JsonTools::GetString(v, "Name");
    //                 config.Mode = JsonTools::GetEnum(v, "Mode", InputActionMode::kPressing);
    //                 config.key = JsonTools::GetEnum(v, "Key", KeyboardKeys::None);
    //                 config.mouseButton = JsonTools::GetEnum(v, "MouseButton", MouseButton::None);
    //                 config.gamepadButton = JsonTools::GetEnum(v, "GamepadButton", GamepadButton::kNone);
    //                 config.gamepad = JsonTools::GetEnum(v, "Gamepad", InputGamepadIndex::kAll);
    //             }
    //         }
    //         else
    //         {
    //             ActionMappings.resize(0, false);
    //         }
    //     }

    //     const auto axisMappings = stream.FindMember("AxisMappings");
    //     if (axisMappings != stream.MemberEnd())
    //     {
    //         auto &axisMappingsArray = axisMappings->value;
    //         if (axisMappingsArray.IsArray())
    //         {
    //             AxisMappings.resize(axisMappingsArray.size(), false);
    //             for (ui32 i = 0; i < axisMappingsArray.size(); i++)
    //             {
    //                 auto &v = axisMappingsArray[i];
    //                 if (!v.IsObject())
    //                     continue;

    //                 AxisConfig &config = AxisMappings[i];
    //                 config.name = JsonTools::GetString(v, "Name");
    //                 config.axis = JsonTools::GetEnum(v, "Axis", InputAxisType::kMouseX);
    //                 config.gamepad = JsonTools::GetEnum(v, "Gamepad", InputGamepadIndex::kAll);
    //                 config.positiveButton = JsonTools::GetEnum(v, "PositiveButton", KeyboardKeys::None);
    //                 config.negativeButton = JsonTools::GetEnum(v, "NegativeButton", KeyboardKeys::None);
    //                 config.gamepadPositiveButton = JsonTools::GetEnum(v, "GamepadPositiveButton", GamepadButton::kNone);
    //                 config.gamepadNegativeButton = JsonTools::GetEnum(v, "GamepadNegativeButton", GamepadButton::kNone);
    //                 config.deadZone = JsonTools::GetFloat(v, "DeadZone", 0.1f);
    //                 config.sensitivity = JsonTools::GetFloat(v, "Sensitivity", 0.4f);
    //                 config.gravity = JsonTools::GetFloat(v, "Gravity", 1.0f);
    //                 config.scale = JsonTools::GetFloat(v, "Scale", 1.0f);
    //                 config.snap = JsonTools::GetBool(v, "Snap", false);
    //             }
    //         }
    //         else
    //         {
    //             AxisMappings.resize(0, false);
    //         }
    //     }
    // }

    i32 InputSystem::getGamepadsCount()
    {
        return Gamepads.count();
    }

    Gamepad *InputSystem::getGamepad(i32 index)
    {
        if (index >= 0 && index < Gamepads.count())
            return Gamepads[index];
        return nullptr;
    }

    void InputSystem::onGamepadsChanged()
    {
        s_GamepadsChanged = true;
    }

    TkStringView InputSystem::getInputText()
    {
        return Keyboard ? Keyboard->getInputText() : TkStringView::Empty;
    }

    bool InputSystem::getKey(const KeyboardKeys key)
    {
        return Keyboard ? Keyboard->getKey(key) : false;
    }

    bool InputSystem::getKeyDown(const KeyboardKeys key)
    {
        return Keyboard ? Keyboard->getKeyDown(key) : false;
    }

    bool InputSystem::getKeyUp(const KeyboardKeys key)
    {
        return Keyboard ? Keyboard->getKeyUp(key) : false;
    }

    Pos InputSystem::getMousePosition()
    {
        return Mouse ? screenToInputPosition(Mouse->getPos()) : Pos::Minimum;
    }

    void InputSystem::setMousePosition(const Pos &position)
    {
        if (Mouse && hasInputFocus())
        {
            const auto pos = inputToScreenPosition(position);
            if (pos > Pos::Minimum)
                Mouse->setMousePosition(pos);
        }
    }

    Pos InputSystem::getMouseScreenPosition()
    {
        return Mouse ? Mouse->getPos() : Pos::Minimum;
    }

    void InputSystem::setMouseScreenPosition(const Pos &position)
    {
        if (Mouse && hasInputFocus())
        {
            Mouse->setMousePosition(position);
        }
    }

    Pos InputSystem::getMousePositionDelta()
    {
        return Mouse ? Mouse->getPosDelta() : Pos{0, 0};
    }

    float InputSystem::getMouseScrollDelta()
    {
        return Mouse ? Mouse->getScrollDelta() : 0.0f;
    }

    bool InputSystem::getMouseButton(const MouseButton button)
    {
        return Mouse ? Mouse->getButton(button) : false;
    }

    bool InputSystem::getMouseButtonDown(const MouseButton button)
    {
        return Mouse ? Mouse->getButtonDown(button) : false;
    }

    bool InputSystem::getMouseButtonUp(const MouseButton button)
    {
        return Mouse ? Mouse->getButtonUp(button) : false;
    }

    float InputSystem::getGamepadAxis(i32 gamepadIndex, GamepadAxis axis)
    {
        if (gamepadIndex >= 0 && gamepadIndex < Gamepads.count())
            return Gamepads[gamepadIndex]->getAxis(axis);
        return 0.0f;
    }

    bool InputSystem::getGamepadButton(i32 gamepadIndex, GamepadButton button)
    {
        if (gamepadIndex >= 0 && gamepadIndex < Gamepads.count())
            return Gamepads[gamepadIndex]->getButton(button);
        return false;
    }

    bool InputSystem::getGamepadButtonDown(i32 gamepadIndex, GamepadButton button)
    {
        if (gamepadIndex >= 0 && gamepadIndex < Gamepads.count())
            return Gamepads[gamepadIndex]->getButtonDown(button);
        return false;
    }

    bool InputSystem::getGamepadButtonUp(i32 gamepadIndex, GamepadButton button)
    {
        if (gamepadIndex >= 0 && gamepadIndex < Gamepads.count())
            return Gamepads[gamepadIndex]->getButtonUp(button);
        return false;
    }

    float InputSystem::getGamepadAxis(InputGamepadIndex gamepad, GamepadAxis axis)
    {
        if (gamepad == InputGamepadIndex::kAll)
        {
            float result = 0.0f;
            for (auto g : Gamepads)
            {
                float v = g->getAxis(axis);
                if (TkMath::abs(v) > TkMath::abs(result))
                    result = v;
            }
            return result;
        }
        else
        {
            const auto index = static_cast<i32>(gamepad);
            if (index >= 0 && index < Gamepads.count())
                return Gamepads[index]->getAxis(axis);
        }
        return 0.0f;
    }

    bool InputSystem::getGamepadButton(InputGamepadIndex gamepad, GamepadButton button)
    {
        if (gamepad == InputGamepadIndex::kAll)
        {
            for (auto g : Gamepads)
            {
                if (g->getButton(button))
                    return true;
            }
        }
        else
        {
            const auto index = static_cast<i32>(gamepad);
            if (index >= 0 && index < Gamepads.count())
                return Gamepads[index]->getButton(button);
        }
        return false;
    }

    bool InputSystem::getGamepadButtonDown(InputGamepadIndex gamepad, GamepadButton button)
    {
        if (gamepad == InputGamepadIndex::kAll)
        {
            for (auto g : Gamepads)
            {
                if (g->getButtonDown(button))
                    return true;
            }
        }
        else
        {
            const auto index = static_cast<i32>(gamepad);
            if (index >= 0 && index < Gamepads.count())
                return Gamepads[index]->getButtonDown(button);
        }
        return false;
    }

    bool InputSystem::getGamepadButtonUp(InputGamepadIndex gamepad, GamepadButton button)
    {
        if (gamepad == InputGamepadIndex::kAll)
        {
            for (auto g : Gamepads)
            {
                if (g->getButtonUp(button))
                    return true;
            }
        }
        else
        {
            const auto index = static_cast<i32>(gamepad);
            if (index >= 0 && index < Gamepads.count())
                return Gamepads[index]->getButtonUp(button);
        }
        return false;
    }

    bool InputSystem::getAction(const TkStringView &name)
    {
        return Actions.containsKey(name) ? Actions[name].active : false;
    }

    InputActionState InputSystem::getActionState(const TkStringView &name)
    {
        return Actions.containsKey(name) ? Actions[name].state : InputActionState::kNone;
    }

    float InputSystem::getAxis(const TkStringView &name)
    {
        return Axes.containsKey(name) ? Axes[name].value : 0.0f;
    }

    float InputSystem::getAxisRaw(const TkStringView &name)
    {
        return Axes.containsKey(name) ? Axes[name].valueRaw : 0.0f;
    }

    // void InputSystem::setInputMappingToDefaultSettings()
    // {
    //     InputSettings *settings = InputSettings::Get();
    //     if (settings)
    //     {
    //         ActionMappings = settings->ActionMappings;
    //         AxisMappings = settings->AxisMappings;
    //         Axes.clear();
    //         Actions.clear();
    //     }
    // }

    ActionConfig InputSystem::getActionConfigByName(const TkStringView &name)
    {
        ActionConfig config = {};
        for (const auto &a : ActionMappings)
        {
            if (a.name == name)
            {
                config = a;
                return config;
            }
        }
        return config;
    }

    TkArray<ActionConfig> InputSystem::getAllActionConfigsByName(const TkStringView &name)
    {
        TkArray<ActionConfig> actionConfigs;
        for (const auto &a : ActionMappings)
        {
            if (a.name == name)
                actionConfigs.add(a);
        }
        return actionConfigs;
    }

    AxisConfig InputSystem::getAxisConfigByName(const TkStringView &name)
    {
        AxisConfig config = {};
        for (const auto &a : AxisMappings)
        {
            if (a.name == name)
            {
                config = a;
                return config;
            }
        }
        return config;
    }

    TkArray<AxisConfig> InputSystem::getAllAxisConfigsByName(const TkStringView &name)
    {
        TkArray<AxisConfig> actionConfigs;
        for (const auto &a : AxisMappings)
        {
            if (a.name == name)
                actionConfigs.add(a);
        }
        return actionConfigs;
    }

    void InputSystem::setAxisConfigByName(const TkStringView &name, AxisConfig &config, bool all)
    {
        for (int i = 0; i < AxisMappings.count(); ++i)
        {
            auto &mapping = AxisMappings.at(i);
            if (mapping.name.compare(name.toString()) == 0)
            {
                if (config.name.isEmpty())
                    config.name = name;
                mapping = config;
                if (!all)
                    break;
            }
        }
    }

    void InputSystem::setAxisConfigByName(const TkStringView &name, InputAxisType inputType, const KeyboardKeys positiveButton, const KeyboardKeys negativeButton, bool all)
    {
        for (int i = 0; i < AxisMappings.count(); ++i)
        {
            auto &mapping = AxisMappings.at(i);
            if (mapping.name.compare(name.toString()) == 0 && mapping.axis == inputType)
            {
                mapping.positiveButton = positiveButton;
                mapping.negativeButton = negativeButton;
                if (!all)
                    break;
            }
        }
    }

    void InputSystem::setAxisConfigByName(const TkStringView &name, InputAxisType inputType, const GamepadButton positiveButton, const GamepadButton negativeButton, InputGamepadIndex gamepadIndex, bool all)
    {
        for (int i = 0; i < AxisMappings.count(); ++i)
        {
            auto &mapping = AxisMappings.at(i);
            if (mapping.name.compare(name.toString()) == 0 && mapping.gamepad == gamepadIndex && mapping.axis == inputType)
            {
                mapping.gamepadPositiveButton = positiveButton;
                mapping.gamepadNegativeButton = negativeButton;
                if (!all)
                    break;
            }
        }
    }

    void InputSystem::setAxisConfigByName(const TkStringView &name, InputAxisType inputType, const float gravity, const float deadZone, const float sensitivity, const float scale, const bool snap, bool all)
    {
        for (int i = 0; i < AxisMappings.count(); ++i)
        {
            auto &mapping = AxisMappings.at(i);
            if (mapping.name.compare(name.toString()) == 0 && mapping.axis == inputType)
            {
                mapping.gravity = gravity;
                mapping.deadZone = deadZone;
                mapping.sensitivity = sensitivity;
                mapping.scale = scale;
                mapping.snap = snap;
                if (!all)
                    break;
            }
        }
    }

    void InputSystem::setActionConfigByName(const TkStringView &name, const KeyboardKeys key, bool all)
    {
        for (int i = 0; i < ActionMappings.count(); ++i)
        {
            auto &mapping = ActionMappings.at(i);
            if (mapping.name.compare(name.toString()) == 0)
            {
                mapping.key = key;
                if (!all)
                    break;
            }
        }
    }

    void InputSystem::setActionConfigByName(const TkStringView &name, const MouseButton mouseButton, bool all)
    {
        for (int i = 0; i < ActionMappings.count(); ++i)
        {
            auto &mapping = ActionMappings.at(i);
            if (mapping.name.compare(name.toString()) == 0)
            {
                mapping.mouseButton = mouseButton;
                if (!all)
                    break;
            }
        }
    }

    void InputSystem::setActionConfigByName(const TkStringView &name, const GamepadButton gamepadButton, InputGamepadIndex gamepadIndex, bool all)
    {
        for (int i = 0; i < ActionMappings.count(); ++i)
        {
            auto &mapping = ActionMappings.at(i);
            if (mapping.name.compare(name.toString()) == 0 && mapping.gamepad == gamepadIndex)
            {
                mapping.gamepadButton = gamepadButton;
                if (!all)
                    break;
            }
        }
    }

    void InputSystem::setActionConfigByName(const TkStringView &name, ActionConfig &config, bool all)
    {
        for (int i = 0; i < ActionMappings.count(); ++i)
        {
            auto &mapping = ActionMappings.at(i);
            if (mapping.name.compare(name.toString()) == 0)
            {
                if (config.name.isEmpty())
                    config.name = name;
                mapping = config;
                if (!all)
                    break;
            }
        }
    }

    void InputSystem::update(float dt)
    {
        PROFILE();
        const u64 frame = ++s_FrameIndex;
        InputEvents.clear();
        const auto windows = getInputWindowsSnapshot();
        const bool inputFocused = [&windows]()
        {
            for (auto* window : windows)
            {
                if (window && window->getParams().allowInput && window->isFocused())
                    return true;
            }
            return false;
        }();

        // If application has no user focus then simply clear the state
        if (!inputFocused)
        {
            if (InputSystem::Mouse)
                InputSystem::Mouse->resetState();
            if (InputSystem::Keyboard)
                InputSystem::Keyboard->resetState();
            for (i32 i = 0; i < InputSystem::Gamepads.count(); i++)
                InputSystem::Gamepads[i]->resetState();
            Axes.clear();
            Actions.clear();
            return;
        }

        // Update input devices state
        if (InputSystem::Mouse)
        {
            if (InputSystem::Mouse->update(InputEvents))
            {
                TkMemory::deletePtr(InputSystem::Mouse);
                InputSystem::Mouse = nullptr;
            }
        }
        if (InputSystem::Keyboard)
        {
            if (InputSystem::Keyboard->update(InputEvents))
            {
                TkMemory::deletePtr(InputSystem::Keyboard);
                InputSystem::Keyboard = nullptr;
            }
        }
        for (i32 i = 0; i < InputSystem::Gamepads.count(); i++)
        {
            if (InputSystem::Gamepads[i]->update(InputEvents))
            {
                TkMemory::deletePtr(InputSystem::Gamepads[i]);
                InputSystem::Gamepads.removeAtKeepOrder(i);
                InputSystem::onGamepadsChanged();
                i--;
                if (InputSystem::Gamepads.isEmpty())
                    break;
            }
        }
        for (i32 i = 0; i < InputSystem::CustomDevices.count(); i++)
        {
            if (InputSystem::CustomDevices[i]->update(InputEvents))
            {
                TkMemory::deletePtr(InputSystem::CustomDevices[i]);
                InputSystem::CustomDevices.removeAtKeepOrder(i);
                i--;
                if (InputSystem::CustomDevices.isEmpty())
                    break;
            }
        }

        // Send gamepads change events
        if (s_GamepadsChanged)
        {
            s_GamepadsChanged = false;
            InputSystem::GamepadsChanged();
        }

        TkWindow *defaultWindow = findPrimaryInputWindow(windows);

        // Lock the cursor to the active input window before virtual axis evaluation so recentering
        // does not leak into the next frame as synthetic mouse delta.
        const auto lockMode = Screen::getCursorLock();
        if (lockMode == CursorLockMode::kLocked && InputSystem::Mouse)
        {
            if (auto *window = defaultWindow)
            {
                const Pos screenCenter = window->clientToScreen(window->getClientSize() * 0.5f);
                if (screenCenter > Pos::Minimum)
                    InputSystem::Mouse->setMousePosition(screenCenter);
            }
        }

        // Send input events for the focused window
        for (const auto &e : InputEvents)
        {
            auto window = e.Target ? e.Target : defaultWindow;
            if (!WindowSystem::isLiveWindow(window) || window->isClosed())
                continue;
            switch (e.Type)
            {
            // Keyboard events
            case InputEventType::kChar:
                window->getEventGroup().onCharInput(e.charData.c);
                break;
            case InputEventType::kKeyDown:
                window->getEventGroup().onKeyDown(e.keyData.key);
                break;
            case InputEventType::kKeyUp:
                window->getEventGroup().onKeyUp(e.keyData.key);
                break;
            // Mouse events
            case InputEventType::kMouseDown:
                window->getEventGroup().onMouseDown(window->screenToClient(e.mouseData.pos), e.mouseData.button);
                break;
            case InputEventType::kMouseUp:
                window->getEventGroup().onMouseUp(window->screenToClient(e.mouseData.pos), e.mouseData.button);
                break;
            case InputEventType::kMouseDoubleClick:
                window->getEventGroup().onMouseDoubleClick(window->screenToClient(e.mouseData.pos), e.mouseData.button);
                break;
            case InputEventType::kMouseWheel:
                window->getEventGroup().onMouseWheel(window->screenToClient(e.mouseWheelData.pos), e.mouseWheelData.wheelDelta);
                break;
            case InputEventType::kMouseMove:
                window->getEventGroup().onMouseMove(window->screenToClient(e.mouseData.pos));
                break;
            case InputEventType::kMouseLeave:
                window->getEventGroup().onMouseLeave();
                break;
            // Touch events
            case InputEventType::kTouchDown:
                window->getEventGroup().onTouchDown(window->screenToClient(e.touchData.pos), e.touchData.pointerId);
                break;
            case InputEventType::kTouchMove:
                window->getEventGroup().onTouchMove(window->screenToClient(e.touchData.pos), e.touchData.pointerId);
                break;
            case InputEventType::kTouchUp:
                window->getEventGroup().onTouchUp(window->screenToClient(e.touchData.pos), e.touchData.pointerId);
                break;
            }
        }

        // Skip virtual input processing if no input-enabled window is focused.
        if (!inputFocused)
        {
            Axes.clear();
            Actions.clear();
            return;
        }

        // Send input events
        for (const auto &e : InputEvents)
        {
            switch (e.Type)
            {
            // Keyboard events
            case InputEventType::kChar:
                InputSystem::CharInput(e.charData.c);
                break;
            case InputEventType::kKeyDown:
                InputSystem::KeyDown(e.keyData.key);
                break;
            case InputEventType::kKeyUp:
                InputSystem::KeyUp(e.keyData.key);
                break;
            // Mouse events
            case InputEventType::kMouseDown:
                InputSystem::MouseDown(e.mouseData.pos, e.mouseData.button);
                break;
            case InputEventType::kMouseUp:
                InputSystem::MouseUp(e.mouseData.pos, e.mouseData.button);
                break;
            case InputEventType::kMouseDoubleClick:
                InputSystem::MouseDoubleClick(e.mouseData.pos, e.mouseData.button);
                break;
            case InputEventType::kMouseWheel:
                InputSystem::MouseWheel(e.mouseWheelData.pos, e.mouseWheelData.wheelDelta);
                break;
            case InputEventType::kMouseMove:
                InputSystem::MouseMove(e.mouseData.pos);
                break;
            case InputEventType::kMouseLeave:
                InputSystem::MouseLeave();
                break;
            // Touch events
            case InputEventType::kTouchDown:
                InputSystem::TouchDown(e.touchData.pos, e.touchData.pointerId);
                break;
            case InputEventType::kTouchMove:
                InputSystem::TouchMove(e.touchData.pos, e.touchData.pointerId);
                break;
            case InputEventType::kTouchUp:
                InputSystem::TouchUp(e.touchData.pos, e.touchData.pointerId);
                break;
            }
        }

        // Update all actions
        for (i32 i = 0; i < InputSystem::ActionMappings.count(); i++)
        {
            const auto &config = InputSystem::ActionMappings[i];
            const TkStringView name = config.name;
            ActionData &data = Actions[name];

            if (data.frameIndex != frame)
            {
                data.active = false;
                data.state = InputActionState::kWaiting;

                // Mark as updated in this frame. Duplicate mappings with the same name are initialized only once.
                data.frameIndex = frame;
            }
        }
        for (i32 i = 0; i < InputSystem::ActionMappings.count(); i++)
        {
            const auto &config = InputSystem::ActionMappings[i];
            const TkStringView name = config.name;
            ActionData &data = Actions[name];

            bool isActive;
            if (config.mode == InputActionMode::kPressing)
            {
                isActive = InputSystem::getKey(config.key) || InputSystem::getMouseButton(config.mouseButton) || InputSystem::getGamepadButton(config.gamepad, config.gamepadButton);
            }
            else if (config.mode == InputActionMode::kPress)
            {
                isActive = InputSystem::getKeyDown(config.key) || InputSystem::getMouseButtonDown(config.mouseButton) || InputSystem::getGamepadButtonDown(config.gamepad, config.gamepadButton);
            }
            else
            {
                isActive = InputSystem::getKeyUp(config.key) || InputSystem::getMouseButtonUp(config.mouseButton) || InputSystem::getGamepadButtonUp(config.gamepad, config.gamepadButton);
            }

            InputActionState candidateState = InputActionState::kWaiting;
            if (InputSystem::getKeyUp(config.key) || InputSystem::getMouseButtonUp(config.mouseButton) || InputSystem::getGamepadButtonUp(config.gamepad, config.gamepadButton))
            {
                candidateState = InputActionState::kRelease;
            }
            else if (InputSystem::getKeyDown(config.key) || InputSystem::getMouseButtonDown(config.mouseButton) || InputSystem::getGamepadButtonDown(config.gamepad, config.gamepadButton))
            {
                candidateState = InputActionState::kPress;
            }
            else if (InputSystem::getKey(config.key) || InputSystem::getMouseButton(config.mouseButton) || InputSystem::getGamepadButton(config.gamepad, config.gamepadButton))
            {
                candidateState = InputActionState::kPressing;
            }

            if (getActionStatePriority(candidateState) > getActionStatePriority(data.state))
                data.state = candidateState;
            data.active |= isActive;
        }

        // Update all axes
        AxesValues.resize(InputSystem::AxisMappings.count(), false);
        for (i32 i = 0; i < InputSystem::AxisMappings.count(); i++)
        {
            const auto &config = InputSystem::AxisMappings[i];
            const TkStringView name = config.name;
            const AxisData &data = Axes[name];

            // Get key raw value
            const bool isPositiveKey = InputSystem::getKey(config.positiveButton) || InputSystem::getGamepadButton(config.gamepad, config.gamepadPositiveButton);
            const bool isNegativeKey = InputSystem::getKey(config.negativeButton) || InputSystem::getGamepadButton(config.gamepad, config.gamepadNegativeButton);
            float keyRawValue = 0;
            if (isPositiveKey && !isNegativeKey)
            {
                keyRawValue = 1;
            }
            else if (!isPositiveKey && isNegativeKey)
            {
                keyRawValue = -1;
            }

            // Apply keyboard curve smoothing and snapping
            float prevKeyValue = data.prevKeyValue;
            if (config.snap && TkMath::notSameSign(data.prevKeyValue, keyRawValue))
            {
                prevKeyValue = 0;
            }
            float keyValue;
            if (TkMath::abs(prevKeyValue) <= TkMath::abs(keyRawValue))
            {
                keyValue = TkMath::lerpStable(prevKeyValue, keyRawValue, TkMath::saturate(dt * config.sensitivity));
            }
            else
            {
                keyValue = TkMath::lerpStable(prevKeyValue, keyRawValue, TkMath::saturate(dt * config.gravity));
            }

            // Get axis raw value
            float axisRawValue = 0.0f;
            switch (config.axis)
            {
            case InputAxisType::kMouseX:
                axisRawValue = InputSystem::getMousePositionDelta().x * config.sensitivity;
                break;
            case InputAxisType::kMouseY:
                axisRawValue = InputSystem::getMousePositionDelta().y * config.sensitivity;
                break;
            case InputAxisType::kMouseWheel:
                axisRawValue = InputSystem::getMouseScrollDelta() * config.sensitivity;
                break;
            case InputAxisType::kGamepadLeftStickX:
                axisRawValue = InputSystem::getGamepadAxis(config.gamepad, GamepadAxis::kLeftStickX);
                break;
            case InputAxisType::kGamepadLeftStickY:
                axisRawValue = InputSystem::getGamepadAxis(config.gamepad, GamepadAxis::kLeftStickY);
                break;
            case InputAxisType::kGamepadRightStickX:
                axisRawValue = InputSystem::getGamepadAxis(config.gamepad, GamepadAxis::kRightStickX);
                break;
            case InputAxisType::kGamepadRightStickY:
                axisRawValue = InputSystem::getGamepadAxis(config.gamepad, GamepadAxis::kRightStickY);
                break;
            case InputAxisType::kGamepadLeftTrigger:
                axisRawValue = InputSystem::getGamepadAxis(config.gamepad, GamepadAxis::kLeftTrigger);
                break;
            case InputAxisType::kGamepadRightTrigger:
                axisRawValue = InputSystem::getGamepadAxis(config.gamepad, GamepadAxis::kRightTrigger);
                break;
            case InputAxisType::kGamepadDPadX:
                if (InputSystem::getGamepadButton(config.gamepad, GamepadButton::kDPadRight))
                    axisRawValue = 1;
                else if (InputSystem::getGamepadButton(config.gamepad, GamepadButton::kDPadLeft))
                    axisRawValue = -1;
                break;
            case InputAxisType::kGamepadDPadY:
                if (InputSystem::getGamepadButton(config.gamepad, GamepadButton::kDPadUp))
                    axisRawValue = 1;
                else if (InputSystem::getGamepadButton(config.gamepad, GamepadButton::kDPadDown))
                    axisRawValue = -1;
                break;
            default:
                break;
            }

            // Apply dead zone
            const float deadZone = config.deadZone;
            float axisValue = axisRawValue >= deadZone || axisRawValue <= -deadZone ? axisRawValue : 0.0f;
            keyValue = keyValue >= deadZone || keyValue <= -deadZone ? keyValue : 0.0f;

            auto &e = AxesValues[i];
            e.used = false;
            e.prevKeyValue = keyValue;

            // Select keyboard input or axis input (choose the higher absolute value).
            // Keep value/raw from the same candidate so smoothing and raw reporting do not get mixed.
            if (TkMath::abs(keyValue) > TkMath::abs(axisValue))
            {
                e.value = keyValue;
                e.rawValue = keyRawValue;
            }
            else
            {
                e.value = axisValue;
                e.rawValue = axisRawValue;
            }

            // Scale
            e.value *= config.scale;
        }
        for (i32 i = 0; i < InputSystem::AxisMappings.count(); i++)
        {
            auto &e = AxesValues[i];
            if (e.used)
                continue;
            const auto &config = InputSystem::AxisMappings[i];
            const TkStringView name = config.name;
            AxisData &data = Axes[name];

            // Blend final axis raw value between all entries
            // Virtual axis with the same name may be used more than once, select the highest absolute value
            for (i32 j = i + 1; j < InputSystem::AxisMappings.count(); j++)
            {
                auto &other = AxesValues[j];
                if (!other.used && InputSystem::AxisMappings[j].name == config.name)
                {
                    if (TkMath::abs(other.value) > TkMath::abs(e.value))
                    {
                        e = other;
                    }
                    other.used = true;
                }
            }

            // Setup axis data
            data.prevKeyValue = e.prevKeyValue;
            data.prevValue = data.value;
            data.valueRaw = e.rawValue;
            data.value = e.value;

            // Mark as updated in this frame
            data.frameIndex = frame;
        }

        // Remove not used entries
        TkArray<TkString> actionsToRemove;
        for (auto i = Actions.Begin(); i.isNotEnd(); ++i)
        {
            if (i->value.frameIndex != frame)
            {
                actionsToRemove.add(i->key);
            }
        }
        for (const TkString& key : actionsToRemove)
            Actions.remove(key);

        TkArray<TkString> axesToRemove;
        for (auto i = Axes.Begin(); i.isNotEnd(); ++i)
        {
            if (i->value.frameIndex != frame)
            {
                axesToRemove.add(i->key);
            }
        }
        for (const TkString& key : axesToRemove)
            Axes.remove(key);

        // Send events for the active actions and axes.
        for (auto i = Axes.Begin(); i.isNotEnd(); ++i)
        {
            if (TkMath::abs(i->value.value - i->value.prevValue) > 0.0001f)
            {
                InputSystem::AxisValueChanged(i->key);
            }
        }

        for (auto i = Actions.Begin(); i.isNotEnd(); ++i)
        {
            if (i->value.state != InputActionState::kWaiting)
            {
                InputSystem::ActionTriggered(i->key, i->value.state);
            }
        }
    }

    void InputSystem::dispose()
    {
        // Dispose input devices
        if (InputSystem::Mouse)
        {
            TkMemory::deletePtr(InputSystem::Mouse);
            InputSystem::Mouse = nullptr;
        }
        if (InputSystem::Keyboard)
        {
            TkMemory::deletePtr(InputSystem::Keyboard);
            InputSystem::Keyboard = nullptr;
        }
        for (i32 i = 0; i < InputSystem::Gamepads.count(); i++)
            TkMemory::deletePtr(InputSystem::Gamepads[i]);
        InputSystem::Gamepads.clear();
        for (i32 i = 0; i < InputSystem::CustomDevices.count(); i++)
            TkMemory::deletePtr(InputSystem::CustomDevices[i]);
        InputSystem::CustomDevices.clear();
        InputEvents.clear();
        AxesValues.clear();
        Actions.clear();
        Axes.clear();
    }

}
