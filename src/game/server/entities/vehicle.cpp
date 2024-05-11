#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include "character.h"

#include "vehicle.h"

IVehicle::IVehicle(CGameWorld *pGameWorld, vec2 Pos, int Type, int Sits, float ProximityRadius)
    : CEntity(pGameWorld, CGameWorld::ENTTYPE_VEHICLE, Pos, ProximityRadius)
{
    m_Type = Type;
    m_Direction = 0;
    m_Health = 100;
    m_NumSit = Sits;
    for (int i = 0; i < m_NumSit; i++)
    {
        m_Sits[i] = vec2(0, 0);
        m_Owners[i] = -1;
    }
    for (int i = 0; i < VEHICLE_MAX_IDS; i++)
        m_IDs[i] = Server()->SnapNewID();

    Reset();
    GameWorld()->InsertEntity(this);
}

IVehicle::~IVehicle()
{
    for (int i = 0; i < m_NumSit; i++)
    {
        if (GameServer()->GetPlayerChar(m_Owners[i]))
            GameServer()->GetPlayerChar(m_Owners[i])->m_Vehicle = -1;
        m_Owners[i] = -1;
    }

    for (int i = 0; i < VEHICLE_MAX_IDS; i++)
        Server()->SnapFreeID(m_IDs[i]);

    GameWorld()->DestroyEntity(this);
}

void IVehicle::Tick()
{
    GravityVehicle();
}

void IVehicle::Snap()
{}

void IVehicle::Reset()
{

}

void IVehicle::GravityVehicle()
{
    
}

int IVehicle::GetOwner(int Slot)
{
    if (Slot >= m_NumSit)
        return -1;

    return (m_Owners[Slot] < 0) ? -1 : m_Owners[Slot];
}

int IVehicle::GetSit(int Slot)
{
    if (Slot >= m_NumSit)
        return -1;

    return (m_Sits[Slot] < 0) ? -1 : m_Sits[Slot];
}