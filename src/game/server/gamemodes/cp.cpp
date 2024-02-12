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
    for (int i = 0; i < (int)m_AreaFlagInfo.size(); i++)
        new CAreaFlag(&GameServer()->m_World, m_AreaFlagInfo[i].m_LowerPos, m_AreaFlagInfo[i].m_UpperPos, m_AreaFlagInfo[i].m_DefaultTeam,
                      m_AreaFlagInfo[i].m_PointEarnPerSec);
    Config()->m_SvScorelimit = m_AreaFlagInfo.size() * 100;
}

void CGameControllerCP::Tick()
{
    int Score[2];
    Score[TEAM_RED] = 0;
    Score[TEAM_BLUE] = 0;
    for (CAreaFlag *Flag = (CAreaFlag *)GameServer()->m_World.FindFirst(CGameWorld::ENTTYPE_AREA_FLAG); Flag; Flag = (CAreaFlag *)Flag->TypeNext())
    {
        if (Config()->m_CPControlMode == MODE_CONTROL_ALL)
        {
            if (Flag->GetTeam() == TEAM_RED)
                Score[TEAM_RED] += 100;
            if (Flag->GetTeam() == TEAM_BLUE)
                Score[TEAM_BLUE] += 100;
            m_aTeamscore[TEAM_RED] = Score[TEAM_RED];
            m_aTeamscore[TEAM_BLUE] = Score[TEAM_BLUE];
        }
        else if (Config()->m_CPControlMode == MODE_SCORE2WIN)
        {
            if (Server()->Tick() % 50 == 0)
                if (Flag->GetTeam() == TEAM_RED)
                    m_aTeamscore[TEAM_RED] += Flag->GetPointEarnPerSec();
            if (Flag->GetTeam() == TEAM_BLUE)
                m_aTeamscore[TEAM_BLUE] += Flag->GetPointEarnPerSec();
        }
    }

    IGameController::Tick();
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
                int DefaultTeam = TEAM_SPECTATORS;
                vec2 Pos0, Pos1;
                if (sscanf(pLine, "flag t%d p%d lx%f ly%f ux%f uy%f", &DefaultTeam, &Point, &Pos0.x, &Pos0.y, &Pos1.x, &Pos1.y))
                {
                    CAreaFlagInfo Temp;
                    Temp.m_PointEarnPerSec = Point;
                    Temp.m_LowerPos = vec2(Pos0.x * 32 + 32, Pos0.y * 32 + 32);
                    Temp.m_UpperPos = vec2(Pos1.x * 32 + 32, Pos1.y * 32 + 32);
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
