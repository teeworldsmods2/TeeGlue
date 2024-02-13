/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "cp.h"

#include <game/server/gamecontext.h>
#include <engine/storage.h>
#include <engine/shared/linereader.h>
#include <engine/shared/config.h>
#include <game/server/entities/area-flag.h>
#include <engine/shared/console.h>

#include <stdio.h>
#include <string.h>

CGameControllerCP::CGameControllerCP(class CGameContext *pGameServer)
    : IGameController(pGameServer)
{
    m_pGameType = "ControlPoint";
    m_GameFlags = GAMEFLAG_TEAMS;

    LoadFlags();
    m_Mode = Config()->m_CPControlMode;
    m_NumFlag = 0;
    for (int i = 0; i < (int)m_AreaFlagInfo.size(); i++)
    {
        if (m_Mode == MODE_SCORE2WIN && m_AreaFlagInfo[i].m_PointEarnPerSec == 0)
            continue;
        new CAreaFlag(&GameServer()->m_World, m_AreaFlagInfo[i].m_LowerPos, m_AreaFlagInfo[i].m_UpperPos, m_AreaFlagInfo[i].m_MaxProgress,
                      m_AreaFlagInfo[i].m_PointEarnPerSec);
        m_NumFlag++;
    }
}

void CGameControllerCP::Tick()
{
    IGameController::Tick();

    int aScore[2];
    aScore[TEAM_RED] = 0;
    aScore[TEAM_BLUE] = 0;
    for (CAreaFlag *Flag = (CAreaFlag *)GameServer()->m_World.FindFirst(CGameWorld::ENTTYPE_AREA_FLAG); Flag; Flag = (CAreaFlag *)Flag->TypeNext())
    {
        if (Config()->m_CPControlMode == MODE_CONTROL_ALL)
        {
            if (Flag->GetProgress() < 0)
                aScore[TEAM_RED] += abs(Flag->GetProgress());
            if (Flag->GetProgress() > 0)
                aScore[TEAM_BLUE] += abs(Flag->GetProgress());
            m_aTeamscore[TEAM_RED] = aScore[TEAM_RED];
            m_aTeamscore[TEAM_BLUE] = aScore[TEAM_BLUE];
        }
        else if (Config()->m_CPControlMode == MODE_SCORE2WIN)
        {
            if (Server()->Tick() % 50 == 0)
            {
                int Score = ceil((float)abs(Flag->GetProgress())) / ((float)Flag->GetMaxProgress()) / ((float)Flag->GetPointEarnPerSec());
                if (Flag->GetProgress() < 0)
                    m_aTeamscore[TEAM_RED] += Score;
                if (Flag->GetProgress() > 0)
                    m_aTeamscore[TEAM_BLUE] += Score;
            }
        }
    }

    if (m_Mode == MODE_SCORE2WIN)
        Config()->m_SvScorelimit = m_NumFlag * 240;
    else
    {
        float Score = 0;
        for (int i = 0; i < (int)m_AreaFlagInfo.size(); i++)
            Score += m_AreaFlagInfo[i].m_MaxProgress;
        Config()->m_SvScorelimit = round_to_int(((int)m_AreaFlagInfo.size() * 240) * (float)(Config()->m_CPPercentToWinGame / 100.f));
    }
}

void CGameControllerCP::LoadFlags()
{
    char aFilename[IO_MAX_PATH_LENGTH];
    str_format(aFilename, sizeof(aFilename), "maps/%s.cfg", Config()->m_SvMap);

    // read file data into buffer
    IOHANDLE File = GameServer()->Storage()->OpenFile(aFilename, IOFLAG_READ, IStorage::TYPE_ALL);
    if (File)
    {
        CLineReader LineReader;
        LineReader.Init(File);
        const char *pLine;
        while ((pLine = LineReader.Get()))
        {
            if (!str_comp_num(pLine, "flag", 4))
            {
                int Point = 0;
                int MaxProgress = Config()->m_CPMaxProgress;
                vec2 Pos0, Pos1;
                if (sscanf(pLine, "flag t%d p%d lx%f ly%f ux%f uy%f", &MaxProgress, &Point, &Pos0.x, &Pos0.y, &Pos1.x, &Pos1.y))
                {
                    CAreaFlagInfo Temp;
                    Temp.m_PointEarnPerSec = Point;
                    Temp.m_LowerPos = vec2(Pos0.x * 32 + 32, Pos0.y * 32 + 32);
                    Temp.m_UpperPos = vec2(Pos1.x * 32 + 32, Pos1.y * 32 + 32);
                    Temp.m_MaxProgress = MaxProgress;
                    m_AreaFlagInfo.push_back(Temp);
                }
            }
            if (!str_comp_num(pLine, "cmd", 3))
            {
                char aBuf[256];
                sscanf(pLine, "cmd \"%s\"", aBuf);
                GameServer()->Console()->ExecuteLine(aBuf);
            }
        }
        io_close(File);
    }
}

void CGameControllerCP::OnRoundStart(int GameState)
{
}