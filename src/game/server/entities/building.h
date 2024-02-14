/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_IBUILDING_H
#define GAME_SERVER_ENTITIES_IBUILDING_H

#include <game/server/entity.h>

class IBuilding : public CEntity
{
public:
    /* Constants */
    static int const ms_PhysSize = 14;

    /* Constructor */
    IBuilding(CGameWorld *pGameWorld, vec2 Pos0, vec2 Pos1, int MaxProgress, int EarnPoint, int Team, float Gravity = 0.5);
    ~IBuilding();

    /* CEntity functions */
    virtual void Reset();
    virtual void TickDefered();

    /* IBuilding */
    int GetTeam() { return m_Team; };
    int GetProgress() { return m_Progress; };
    int GetPointEarnPerSec() { return m_PointEarnPerSec; }
    int GetMaxProgress() { return m_MaxProgress; }
    bool IsShow() { return m_Show; };

    /* IBuilding functions */
    virtual void SnapBuilding(int SnappingClient);
    virtual void Hide();
    virtual void Show();
    virtual void TakeDamage(int Who, int Weapon, int Damage = 0);
    virtual void Gravity();

private:
    /* IBuilding Constants */
    int m_Progress;
    int m_PointEarnPerSec;
    vec2 m_LowerPos;
    vec2 m_UpperPos;
    vec2 m_Vel;
    vec2 m_RelPos;
    int m_MaxProgress;

    int m_LaserSnap[2];

    float m_StepX;
    float m_StepY;

    int m_Team;

    bool m_Show;

    float m_Gravity;
    // You may not to touch it.
protected:
    virtual void Snap(int SnappingClient);
};

#endif