#pragma once
#include <functional>

struct DeleteHook
{
	bool marked = false;
	std::function<void()> onDelete = nullptr;

	void Hook(std::function<void()> onDelete)
	{
		this->onDelete = onDelete;
	}
	void operator=(bool v)
	{
		marked = v;
		if (v) { onDelete(); }
	}
	bool operator==(const DeleteHook& other) const
	{
		return marked == other.marked;
	}
	bool operator==(bool v) const
	{
		return marked == v;
	}
	bool operator==(bool v)
	{
		return marked == v;
	}
	explicit operator bool() const
	{
		return marked;
	}
	explicit operator bool()
	{
		return marked;
	}
	bool operator !() const
	{
		return !marked;
	}
	bool operator !()
	{
		return !marked;
	}
};