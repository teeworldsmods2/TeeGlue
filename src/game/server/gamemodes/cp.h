/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_CP_H
#define GAME_SERVER_GAMEMODES_CP_H
#include <game/server/gamecontroller.h>

enum
{
    MODE_CONTROL_ALL = 0,
    MODE_SCORE2WIN,
};

class CGameControllerCP : public IGameController
{
public:
    CGameControllerCP(class CGameContext *pGameServer);
    virtual void Tick();

public:
    void LoadFlags();
};
#endif
