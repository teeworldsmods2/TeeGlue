/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>

#include "character.h"
#include <game/server/player.h>
#include <engine/shared/config.h>

#include "building.h"

IBuilding::IBuilding(CGameWorld *pWorld, vec2 Pos0, vec2 Pos1, int MaxProgress, int EarnPoint, int Team, float Gravity)
    : CEntity(pWorld, CGameWorld::ENTTYPE_BUILDING, Pos0, ms_PhysSize)
{
    m_LowerPos = Pos0;
    m_UpperPos = Pos1;
    m_PointEarnPerSec = EarnPoint;
    m_LaserSnap[0] = Server()->SnapNewID();
    m_LaserSnap[1] = Server()->SnapNewID();
    m_MaxProgress = MaxProgress;
    m_Team = Team;
    m_Gravity = Gravity;
    m_Vel = vec2(0, 0);
    Reset();
    GameWorld()->InsertEntity(this);
}

IBuilding::~IBuilding()
{
    Server()->SnapFreeID(m_LaserSnap[0]);
    Server()->SnapFreeID(m_LaserSnap[1]);
}

void IBuilding::Reset()
{
    m_Progress = 0;
}

void IBuilding::Snap(int SnappingClient)
{
    if (NetworkClipped(SnappingClient, m_LowerPos) && NetworkClipped(SnappingClient, m_UpperPos))
        return;

    if (IsShow())
    {
        SnapBuilding(SnappingClient);
    }
    else
    {
        m_StepX = fabs(m_LowerPos.x - m_UpperPos.x) / m_MaxProgress;
        m_StepY = fabs(m_LowerPos.y - m_UpperPos.y) / m_MaxProgress;

        /* When Construction */
        CNetObj_Flag Flag;
        if (m_LowerPos.x < m_UpperPos.x)
            Flag.m_X = round_to_int(m_LowerPos.x + abs(m_Progress) * m_StepX) + 14.f;
        else
            Flag.m_X = round_to_int(m_LowerPos.x + (-abs(m_Progress)) * m_StepX) + 14.f;
        if (m_LowerPos.y < m_UpperPos.y)
            Flag.m_Y = round_to_int(m_LowerPos.y + abs(m_Progress) * m_StepY);
        else
            Flag.m_Y = round_to_int(m_LowerPos.y + (-abs(m_Progress)) * m_StepY);

        m_Pos = vec2(Flag.m_X, Flag.m_Y);
        Flag.m_Team = GetTeam();
        NetConverter()->SnapNewItemConvert(&Flag, this, NETOBJTYPE_FLAG, GetID(), sizeof(CNetObj_Flag), SnappingClient);

        CNetObj_Laser Laser0;
        Laser0.m_FromX = m_LowerPos.x;
        Laser0.m_FromY = m_LowerPos.y;
        Laser0.m_X = m_UpperPos.x;
        Laser0.m_Y = m_UpperPos.y;
        Laser0.m_StartTick = Server()->Tick();
        NetConverter()->SnapNewItemConvert(&Laser0, this, NETOBJTYPE_LASER, m_LaserSnap[0], sizeof(CNetObj_Laser), SnappingClient);

        CNetObj_Laser Laser1;
        Laser1.m_FromX = m_UpperPos.x;
        Laser1.m_FromY = m_UpperPos.y;
        Laser1.m_X = m_UpperPos.x;
        Laser1.m_Y = m_LowerPos.y;
        Laser1.m_StartTick = Server()->Tick();
        NetConverter()->SnapNewItemConvert(&Laser1, this, NETOBJTYPE_LASER, m_LaserSnap[1], sizeof(CNetObj_Laser), SnappingClient);
    }
}

void IBuilding::SnapBuilding(int SnappingClient)
{
    Hide();
}

void IBuilding::TickDefered()
{
    if (m_LowerPos.x < m_UpperPos.x)
    {
        m_RelPos.x = m_UpperPos.x - m_LowerPos.x;
    }
    else
    {
        m_RelPos.x = m_LowerPos.x - m_UpperPos.x;
    }
    if (m_LowerPos.y > m_UpperPos.y)
    {
        m_RelPos.y = m_UpperPos.y - m_LowerPos.y;
        Gravity();
    }
    else
    {
        m_RelPos.y = m_LowerPos.y - m_UpperPos.y;
        Gravity();
    }
    m_UpperPos = m_LowerPos + m_RelPos;
}

void IBuilding::Show()
{
    m_Show = true;
}

void IBuilding::Hide()
{
    m_Show = false;
}

void IBuilding::TakeDamage(int Who, int Weapon, int Damage)
{
    if (Weapon == WEAPON_HAMMER)
    {
        if (m_Progress >= m_MaxProgress)
            return;

        GameServer()->CreateHammerHit(GetPos());
        m_Progress += Damage ? Damage : 25;
        if (m_Progress >= m_MaxProgress)
        {
            m_Progress = m_MaxProgress;
            Show();
        }
    }
}

void IBuilding::Gravity()
{
    m_Vel.y += m_Gravity;
    GameServer()->Collision()->MoveBox(&m_LowerPos, &m_Vel, vec2(24.0f, 24.0f), 0.4f);
}