﻿#ifndef SIMPLE_GAME_H
#define SIMPLE_GAME_H
#pragma once

#include "Game.h"
#include "BoxEntity.h"
#include "SphereEntity.h"

class SimpleGame :  public Game
{
public:
    SimpleGame();
    ~SimpleGame();
    bool InitGraphics() override;
    bool InitEntities() override;
    void Update(const GameTime& time) override;

private:
    BoxEntity* boxEntity;
	SphereEntity* sphereEntity;
};

#endif //SIMPLE_GAME_H
