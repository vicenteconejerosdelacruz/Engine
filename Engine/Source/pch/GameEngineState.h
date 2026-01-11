#pragma once

enum GameEngineState
{
	GES_None,
	GES_Boot,
	GES_Step
};

template<typename T>
struct GameEngineStatesMachine : public GameStatesMachine<T>
{
	SceneUnitId unit;
};

typedef GameEngineStatesMachine<GameEngineState> GEngineSM;