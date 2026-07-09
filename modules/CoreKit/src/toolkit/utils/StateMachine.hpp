/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/collection/Array.hpp>

namespace toolkit
{
    class StateMachine;

    // State machine state
    class TK_API State
    {
        friend StateMachine;

    protected:
        StateMachine *m_parent;

    protected:
        State(StateMachine *parent);
        ~State();

    public:
        // State's activity
        bool isActive() const;

        // Checks if can enter to that state
        virtual bool canEnter() const
        {
            return true;
        }

        // Checks if can exit from that state
        virtual bool canExit(State *nextState) const
        {
            return true;
        }

    protected:
        virtual void enterState()
        {
        }

        virtual void exitState()
        {
        }
    };

    // State machine logic patter
    class TK_API StateMachine
    {
        friend State;

    protected:
        State *m_currentState;
        TkArray<State *> m_states;

    public:
        StateMachine();
        virtual ~StateMachine();

    public:
        // Get current state
        virtual State *currentState()
        {
            return m_currentState;
        }

        // Get readonly states array
        const TkArray<State *> *getStates() const
        {
            return &m_states;
        }

    public:
        // Go to state
        void goToState(i32 stateIndex)
        {
            goToState(m_states[stateIndex]);
        }

        // Go to state
        virtual void goToState(State *state);

    protected:
        virtual void switchState(State *nextState);
    };

}