#pragma once

#include <game/server/entity.h>
#include <vector>

#define VEHICLE_MAX_IDS 24
class IVehicle : public CEntity
{
private:
    int m_Type;
    int m_Direction;
    int m_Health;
    int m_NumSit;
    int m_Owners[MAX_CLIENTS];
    vec2 m_Sits[MAX_CLIENTS];
    int m_IDs[VEHICLE_MAX_IDS];

public:
    IVehicle(CGameWorld *pGameWorld, vec2 Pos, int Type, int NumSits, float ProximityRadius);
    ~IVehicle();

    /* Vehicle Functions */
    virtual void GravityVehicle();

    virtual void Tick();
    virtual void Snap();
    virtual void Reset();

    int GetType() { return m_Type; }
    int GetDirection() { return m_Direction; }
    int GetHealth() { return m_Health; }
    int GetOwner(int Slot);
    int GetSit(int Slot);
};

class CVehicles
{
private:
    std::vector<IVehicle *> m_vVehicles;
public:
    CVehicles(CGameContext *pGameServer);
    ~CVehicles();

    void CreateVehicle(vec2 Pos, int Type);

    int GetVOwner
};