#pragma once

enum GameEngineState
{
	GES_None,
	GES_Boot
};

template<typename T>
struct GameEngineStatesMachine : public GameStatesMachine<T>
{
	SceneUnitId unit;
};

typedef GameEngineStatesMachine<GameEngineState> GEngineSM;