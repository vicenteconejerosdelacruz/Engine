#pragma once
#include <map>
#include <functional>

template<typename T>
struct GameStatesMachine {
public:
	T currentState;
	std::map<T, std::function<void(GameStatesMachine<T>*, T)>> onEnter;
	std::map<T, std::function<void(GameStatesMachine<T>*, T)>> onLeave;
	std::map<T, std::function<void(GameStatesMachine<T>*)>> onStep;
	std::map<T, std::function<void(GameStatesMachine<T>*)>> onRender;
	std::map<T, std::function<void(GameStatesMachine<T>*)>> onPostRender;

	void ChangeState(T newState)
	{
		T prevState = currentState;
		if (onLeave.contains(currentState)) { onLeave.at(currentState)(this, newState); }
		currentState = newState;
		if (onEnter.contains(currentState)) { onEnter.at(currentState)(this, prevState); }
	}

	void Step()
	{
		if (onStep.contains(currentState)) { onStep.at(currentState)(this); }
	}

	void Render()
	{
		if (onRender.contains(currentState)) { onRender.at(currentState)(this); }
	}

	void PostRender()
	{
		if (onPostRender.contains(currentState)) { onPostRender.at(currentState)(this); }
	}
};