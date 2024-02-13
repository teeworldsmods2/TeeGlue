/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>

#include "character.h"
#include <game/server/player.h>
#include <engine/shared/config.h>

#include "area-flag.h"

CAreaFlag::CAreaFlag(CGameWorld *pWorld, vec2 Pos0, vec2 Pos1, int MaxProgress, int EarnPoint)
    : CEntity(pWorld, CGameWorld::ENTTYPE_AREA_FLAG, Pos0, ms_PhysSize)
{
    m_LowerPos = Pos0;
    m_UpperPos = Pos1;
    m_PointEarnPerSec = EarnPoint;
    m_LaserSnap[0] = Server()->SnapNewID();
    m_LaserSnap[1] = Server()->SnapNewID();
    m_MaxProgress = MaxProgress;
    Reset();
    GameWorld()->InsertEntity(this);
}

CAreaFlag::~CAreaFlag()
{
    Server()->SnapFreeID(m_LaserSnap[0]);
    Server()->SnapFreeID(m_LaserSnap[1]);
}

void CAreaFlag::Reset()
{
    m_StepX = fabs(m_LowerPos.x - m_UpperPos.x) / m_MaxProgress;
    m_StepY = fabs(m_LowerPos.y - m_UpperPos.y) / m_MaxProgress;
    m_Progress = 0;
}

void CAreaFlag::Snap(int SnappingClient)
{
    if (NetworkClipped(SnappingClient, m_LowerPos) && NetworkClipped(SnappingClient, m_UpperPos))
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

        if (GetTeam() == -1)
        {
            if (Server()->Tick() % 5 == 0)
            {
                GameServer()->CreateSound(m_LowerPos, SOUND_HOOK_ATTACH_GROUND);
                GameServer()->CreateSound(m_UpperPos, SOUND_HOOK_LOOP);
            }
            char aBuf[64];
            int Percent = (int)(((float)(abs(m_Progress)) / (float)m_MaxProgress) * 100);
            str_format(aBuf, sizeof(aBuf), "Capturing the base! %d%%", Percent);
            GameServer()->SendBroadcast(aBuf, p->GetPlayer()->GetCID());
        }
    }
    if ((m_Progress + Progress) >= m_MaxProgress)
    {
        if (GetTeam() == -1)
        {
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                CPlayer *p = GameServer()->m_apPlayers[i];
                if (!p || !p->GetCharacter())
                    continue;

                if (p->GetTeam() == TEAM_RED)
                    GameServer()->CreateSound(p->GetCharacter()->GetPos(), SOUND_CTF_DROP, CmaskOne(i));
                if (p->GetTeam() == TEAM_BLUE)
                    GameServer()->CreateSound(p->GetCharacter()->GetPos(), SOUND_CTF_CAPTURE, CmaskOne(i));
            }

            GameServer()->SendBroadcast("The Blue Team has Captured a Base!", -1);
            GameServer()->SendChat(-1, CHAT_ALL, -1, "The Blue Team has Captured a Base!");
        }
        m_Progress = m_MaxProgress;
    }
    else if ((m_Progress + Progress) <= -m_MaxProgress)
    {
        if (GetTeam() == -1)
        {
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                CPlayer *p = GameServer()->m_apPlayers[i];
                if (!p || !p->GetCharacter())
                    continue;

                if (p->GetTeam() == TEAM_RED)
                    GameServer()->CreateSound(p->GetCharacter()->GetPos(), SOUND_CTF_CAPTURE, CmaskOne(i));
                if (p->GetTeam() == TEAM_BLUE)
                    GameServer()->CreateSound(p->GetCharacter()->GetPos(), SOUND_CTF_DROP, CmaskOne(i));
            }
            GameServer()->SendBroadcast("The Red Team has Captured a Base!", -1);
            GameServer()->SendChat(-1, CHAT_ALL, -1, "The Red Team has Captured a Base!");
        }
        m_Progress = -m_MaxProgress;
    }
    else
        m_Progress += Progress;
}

int CAreaFlag::GetTeam()
{
    if (m_Progress >= m_MaxProgress)
        return TEAM_BLUE;
    else if (m_Progress <= -m_MaxProgress)
        return TEAM_RED;
    return -1;
}