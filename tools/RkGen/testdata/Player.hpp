#pragma rkgen
#pragma once

#include "BasicTypes.hpp"

namespace rkgen_test {

struct Entity
{
    int id;
};

cclass Player : public Entity
{
public:
    cfield() int hp;
    cfield(GetMana) int mana;
    cfield(GetShield, SetShield) int shield;
    cfield(, SetName) const char* name;
    cfield(readonly) Vec3 position;
    cfield(transient) void* runtimeOnly;

    Player(int id);

    cfunc void SetHp(int value);
    cfunc int GetHp() const noexcept;
    cfunc static Player* SpawnDefault();

private:
    int GetMana() const;
    int GetShield() const;
    void SetShield(int value);
    void SetName(const char* value);
};

}
