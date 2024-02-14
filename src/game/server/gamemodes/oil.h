/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_OIL_H
#define GAME_SERVER_GAMEMODES_OIL_H
#include <game/server/gamecontroller.h>

class CGameControllerOIL : public IGameController
{
public:
    CGameControllerOIL(class CGameContext *pGameServer);
    virtual void Tick();
    // add more virtual functions here if you wish
};
#endif
