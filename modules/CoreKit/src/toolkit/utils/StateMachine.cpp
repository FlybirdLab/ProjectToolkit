/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include "StateMachine.hpp"

namespace toolkit
{
    State::State(StateMachine *parent)
        : m_parent(parent)
    {
        m_parent->m_states.add(this);
    }

    State::~State()
    {
        if (m_parent)
            m_parent->m_states.remove(this);
    }

    bool State::isActive() const
    {
        return m_parent->m_currentState == this;
    }

    StateMachine::StateMachine()
        : m_currentState(nullptr)
    {
    }

    StateMachine::~StateMachine()
    {
        for (i32 i = 0; i < m_states.count(); i++)
        {
            m_states[i]->m_parent = nullptr;
        }
    }

    void StateMachine::goToState(State *state)
    {
        // Prevent from entering the same state
        if (state == m_currentState)
            return;

        // Check if cannot leave current state
        if (m_currentState && !m_currentState->canExit(state))
            return;

        // Check if cannot enter new state
        if (state && !state->canEnter())
            return;

        // Change state
        switchState(state);
    }

    void StateMachine::switchState(State *nextState)
    {
        if (m_currentState)
            m_currentState->exitState();

        m_currentState = nextState;

        if (m_currentState)
            m_currentState->enterState();
    }

}