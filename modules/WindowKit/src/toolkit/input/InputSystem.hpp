/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/input/InputDevice.hpp>
#include <toolkit/input/VirtualInput.hpp>

namespace toolkit
{
    class Mouse;
    class Keyboard;
    class Gamepad;
    class InputDevice;

    // The user input handling service.
    class TK_API InputSystem
    {
    public:
        // Get the mouse (null if platform does not support mouse or it is not connected).
        static Mouse *Mouse;
        // Get the keyboard (null if platform does not support keyboard or it is not connected).
        static Keyboard *Keyboard;
        // Get the gamepads.
        static TkArray<Gamepad *> Gamepads;

        // Get the gamepads count.
        static i32 getGamepadsCount();
        // Get the gamepads count.
        static Gamepad *getGamepad(i32 index);

        // Action called when gamepads collection gets changed (during input update).
        static TkAction<> GamepadsChanged;
        // Called when gamepads collection gets changed.
        static void onGamepadsChanged();

        // Get or sets the custom input devices.
        static TkArray<InputDevice *> CustomDevices;

    public:
        // Setup input system.
        static void init();
        // Update input system.
        static void update(float dt);
        // Dispose input system.
        static void dispose();

    public:
        // Event fired on character input.
        static TkAction<wchar> CharInput;
        // Event fired on key pressed.
        static TkAction<KeyboardKeys> KeyDown;
        // Event fired on key released.
        static TkAction<KeyboardKeys> KeyUp;
        // Event fired when mouse button goes down.
        static TkAction<const Pos &, MouseButton> MouseDown;
        // Event fired when mouse button goes up.
        static TkAction<const Pos &, MouseButton> MouseUp;
        // Event fired when mouse button double clicks.
        static TkAction<const Pos &, MouseButton> MouseDoubleClick;
        // Event fired when mouse wheel is scrolling (wheel delta is normalized).
        static TkAction<const Pos &, float> MouseWheel;
        // Event fired when mouse moves.
        static TkAction<const Pos &> MouseMove;
        // Event fired when mouse leaves window.
        static TkAction<> MouseLeave;
        // Event fired when touch action begins.
        static TkAction<const Pos &, i32> TouchDown;
        // Event fired when touch action moves.
        static TkAction<const Pos &, i32> TouchMove;
        // Event fired when touch action ends.
        static TkAction<const Pos &, i32> TouchUp;

    public:
        // Get the text entered during the current frame (Unicode).
        static TkStringView getInputText();
        // Get the key state (true if key is being pressed during this frame).
        static bool getKey(KeyboardKeys key);
        // Get the key 'down' state (true if key was pressed in this frame).
        static bool getKeyDown(KeyboardKeys key);
        // Get the key 'up' state (true if key was released in this frame).
        static bool getKeyUp(KeyboardKeys key);

    public:
        // Get the mouse position in the active input window coordinates.
        static Pos getMousePosition();
        // Sets the mouse position in the active input window coordinates.
        static void setMousePosition(const Pos &position);
        // Get the mouse position in screen-space coordinates.
        static Pos getMouseScreenPosition();
        // Sets the mouse position in screen-space coordinates.
        static void setMouseScreenPosition(const Pos &position);
        // Get the mouse position change during the last frame.
        static Pos getMousePositionDelta();
        // Get the mouse wheel change during the last frame.
        static float getMouseScrollDelta();
        // Get the mouse button state.
        static bool getMouseButton(MouseButton button);
        // Get the mouse button down state.
        static bool getMouseButtonDown(MouseButton button);
        // Get the mouse button up state.
        static bool getMouseButtonUp(MouseButton button);

    public:
        // Get the gamepad axis value.
        static float getGamepadAxis(i32 gamepadIndex, GamepadAxis axis);
        // Get the gamepad button state (true if being pressed during the current frame).
        static bool getGamepadButton(i32 gamepadIndex, GamepadButton button);
        // Get the gamepad button down state (true if was pressed during the current frame).
        static bool getGamepadButtonDown(i32 gamepadIndex, GamepadButton button);
        // Get the gamepad button up state (true if was released during the current frame).
        static bool getGamepadButtonUp(i32 gamepadIndex, GamepadButton button);
        // Get the gamepad axis value.
        static float getGamepadAxis(InputGamepadIndex gamepad, GamepadAxis axis);
        // Get the gamepad button state (true if being pressed during the current frame).
        static bool getGamepadButton(InputGamepadIndex gamepad, GamepadButton button);
        // Get the gamepad button down state (true if was pressed during the current frame).
        static bool getGamepadButtonDown(InputGamepadIndex gamepad, GamepadButton button);
        // Get the gamepad button up state (true if was released during the current frame).
        static bool getGamepadButtonUp(InputGamepadIndex gamepad, GamepadButton button);

    public:
        // Maps a discrete button or key press events to a "friendly name" that will later be bound to event-driven behavior. The end effect is that pressing (and/or releasing) a key, mouse button, or keypad button.
        static TkArray<ActionConfig> ActionMappings;
        // Maps keyboard, controller, or mouse inputs to a "friendly name" that will later be bound to continuous game behavior, such as movement. The inputs mapped in AxisMappings are continuously polled, even if they are just reporting that their input value.
        static TkArray<AxisConfig> AxisMappings;
        // Event fired when virtual input action is triggered. Called before scripts update. See <see cref="ActionMappings"/> to edit configuration.
        static TkAction<TkStringView, InputActionState> ActionTriggered;
        // Event fired when virtual input axis is changed. Called before scripts update. See <see cref="AxisMappings"/> to edit configuration.
        static TkAction<TkStringView> AxisValueChanged;
        // Get the value of the virtual action identified by name. Use <see cref="ActionMappings"/> to get the current config.
        static bool getAction(const TkStringView &name);
        // Get the value of the virtual action identified by name. Use <see cref="ActionMappings"/> to get the current config.
        static InputActionState getActionState(const TkStringView &name);
        // Get the value of the virtual axis identified by name. Use <see cref="AxisMappings"/> to get the current config.
        static float getAxis(const TkStringView &name);
        // Get the raw value of the virtual axis identified by name with no smoothing filtering applied. Use <see cref="AxisMappings"/> to get the current config.
        static float getAxisRaw(const TkStringView &name);

        // // Sets and overwrites the Action and Axis mappings with the values from a new InputSettings.
        // static void setInputMappingFromSettings(const JsonAssetReference<class InputSettings> &settings);
        // // Sets and overwrites the Action and Axis mappings with the values from the InputSettings in GameSettings.
        // static void setInputMappingToDefaultSettings();

        // Get the first action configuration by name.
        static ActionConfig getActionConfigByName(const TkStringView &name);
        // Get all the action configurations by name.
        static TkArray<ActionConfig> getAllActionConfigsByName(const TkStringView &name);
        // Sets the action configuration keyboard key by name.
        static void setActionConfigByName(const TkStringView &name, const KeyboardKeys key, bool all = false);
        // Sets the action configuration mouse button by name.
        static void setActionConfigByName(const TkStringView &name, const MouseButton mouseButton, bool all = false);
        // Sets the action configuration gamepad button by name and index.
        static void setActionConfigByName(const TkStringView &name, const GamepadButton gamepadButton, InputGamepadIndex gamepadIndex, bool all = false);
        // Sets the action configuration by name.
        static void setActionConfigByName(const TkStringView &name, ActionConfig &config, bool all = false);
        // Get the first axis configurations by name.
        static AxisConfig getAxisConfigByName(const TkStringView &name);
        // Get all the axis configurations by name.
        static TkArray<AxisConfig> getAllAxisConfigsByName(const TkStringView &name);
        // Sets the axis configuration keyboard key by name and type.
        static void setAxisConfigByName(const TkStringView &name, AxisConfig &config, bool all = false);
        // Sets the axis configuration keyboard key buttons by name and type.
        static void setAxisConfigByName(const TkStringView &name, InputAxisType inputType, const KeyboardKeys positiveButton, const KeyboardKeys negativeButton, bool all = false);
        // Sets the axis configuration gamepad buttons by name, type, and index.
        static void setAxisConfigByName(const TkStringView &name, InputAxisType inputType, const GamepadButton positiveButton, const GamepadButton negativeButton, InputGamepadIndex gamepadIndex, bool all = false);
        // Sets the axis configuration accessories by name, and type.
        static void setAxisConfigByName(const TkStringView &name, InputAxisType inputType, const float gravity, const float deadZone, const float sensitivity, const float scale, const bool snap, bool all = false);
    };

}
