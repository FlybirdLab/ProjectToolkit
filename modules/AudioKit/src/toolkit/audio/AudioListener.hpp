/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/audio/common.hpp>
#include <toolkit/math/Quat.hpp>
#include <toolkit/math/Vector3.hpp>

namespace toolkit
{
    class TK_API AudioListener
    {
    public:
        AudioListener();
        ~AudioListener();

        const Vec3 &getVelocity() const;

        const Vec3 &getPosition() const;
        void setPosition(const Vec3 &position);

        const Quat &getOrientation() const;
        void setOrientation(const Quat &orientation);

        void setTransform(const Vec3 &position, const Quat &orientation);

        // Per-frame listener update used to compute velocity from position delta.
        // Usually called by Audio::update().
        void update(f32 deltaTime);
        // Manually updates velocity from current and previous position.
        void updateVelocityFromPositionDelta(f32 deltaTime);
        // Resets tracked motion to zero velocity.
        void resetMotionTracking();

        // Listener must be explicitly activated in decoupled runtime.
        bool isActive() const;
        bool setActive(bool active);

    private:
        void onTransformChanged();

    private:
        Vec3 m_velocity = Vec3::kZero;
        Vec3 m_previousPos = Vec3::kZero;
        Vec3 m_position = Vec3::kZero;
        Quat m_orientation = Quat::kIdentity;
        bool m_active = false;
    };
}
