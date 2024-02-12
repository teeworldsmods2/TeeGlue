/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>

#include "character.h"
#include <game/server/player.h>
#include <engine/shared/config.h>

#include "area-flag.h"

CAreaFlag::CAreaFlag(CGameWorld *pWorld, vec2 Pos0, vec2 Pos1, int DefaultTeam, int EarnPoint)
    : CEntity(pWorld, CGameWorld::ENTTYPE_AREA_FLAG, Pos0, ms_PhysSize)
{
    m_LowerPos = Pos0;
    m_UpperPos = Pos1;
    m_PointEarnPerSec = EarnPoint;
    m_LaserSnap = Server()->SnapNewID();

    GameWorld()->InsertEntity(this);
}

CAreaFlag::~CAreaFlag()
{
    Server()->SnapFreeID(m_LaserSnap);
}

void CAreaFlag::Reset()
{
    m_StepX = fabs(m_LowerPos.x - m_UpperPos.x) / Config()->m_CPMaxProgress;
    m_StepY = fabs(m_LowerPos.y - m_UpperPos.y) / Config()->m_CPMaxProgress;
    m_Progress = 0;
}

void CAreaFlag::Snap(int SnappingClient)
{
    if (NetworkClipped(SnappingClient))
        return;

    CNetObj_Flag Flag;
    if (m_LowerPos.x < m_UpperPos.x)
        Flag.m_X = round_to_int(m_LowerPos.x + abs(m_Progress) * m_StepX) + 14.f;
    else
        Flag.m_X = round_to_int(m_LowerPos.x + (-abs(m_Progress)) * m_StepX) + 14.f;
    if (m_LowerPos.y < m_UpperPos.y)
        Flag.m_Y = round_to_int(m_LowerPos.y + abs(m_Progress) * m_StepY);
    else
        Flag.m_Y = round_to_int(m_LowerPos.y + (-abs(m_Progress)) * m_StepY);

    Flag.m_Team = (m_Progress < 0) ? TEAM_RED : TEAM_BLUE;
    NetConverter()->SnapNewItemConvert(&Flag, this, NETOBJTYPE_FLAG, GetID(), sizeof(CNetObj_Flag), SnappingClient);

    CNetObj_Laser Laser;
    Laser.m_FromX = m_LowerPos.x;
    Laser.m_FromY = m_LowerPos.y;
    Laser.m_X = m_UpperPos.x;
    Laser.m_Y = m_UpperPos.y;
    Laser.m_StartTick = Server()->Tick();
    NetConverter()->SnapNewItemConvert(&Laser, this, NETOBJTYPE_LASER, m_LaserSnap, sizeof(CNetObj_Laser), SnappingClient);
}

void CAreaFlag::TickDefered()
{
    int Progress = 0;
    for (CCharacter *p = (CCharacter *)GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); p; p = (CCharacter *)p->TypeNext())
    {
        if (GameServer()->Collision()->IntersectLine(m_LowerPos, p->GetPos(), 0, 0, CCollision::COLFLAG_HOLD) && GameServer()->Collision()->IntersectLine(m_UpperPos, p->GetPos(), 0, 0, CCollision::COLFLAG_HOLD))
            continue;

        if (p->GetPlayer()->GetTeam() == TEAM_RED)
            Progress--;
        else if (p->GetPlayer()->GetTeam() == TEAM_BLUE) // I'm not sure if team = spec...(it was happened before)
            Progress++;

        int Percent = (int)(((float)(abs(m_Progress)) / (float)Config()->m_CPMaxProgress) * 100);
        char aBuf[64];
        str_format(aBuf, sizeof(aBuf), "Capturing the base! %d%%", Percent);
        GameServer()->SendBroadcast(aBuf, p->GetPlayer()->GetCID());
    }
    if ((m_Progress + Progress) >= Config()->m_CPMaxProgress)
    {
        if (GetTeam() == -1)
            GameServer()->CreateSound(m_UpperPos, SOUND_CTF_CAPTURE);
        m_Progress = Config()->m_CPMaxProgress;
    }
    else if ((m_Progress + Progress) <= -Config()->m_CPMaxProgress)
    {
        if (GetTeam() == -1)
            GameServer()->CreateSound(m_UpperPos, SOUND_CTF_CAPTURE);
        m_Progress = -Config()->m_CPMaxProgress;
    }
    else
        m_Progress += Progress;
}

int CAreaFlag::GetTeam()
{
    if (m_Progress >= Config()->m_CPMaxProgress)
        return TEAM_BLUE;
    else if (m_Progress <= -Config()->m_CPMaxProgress)
        return TEAM_RED;
    return -1;
}