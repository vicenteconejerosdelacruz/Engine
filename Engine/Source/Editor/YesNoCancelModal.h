#pragma once
#include <functional>

struct YesNoCancelModal
{
	YesNoCancelModal();
	void Init(std::function<void()> yes, std::function<void()> no, std::function<void()> cancel);
	void Show();
	void Hide();
	bool Showing();
	void Draw(const char* title, const char* text);

	bool show;
	std::function<void()> onYes;
	std::function<void()> onNo;
	std::function<void()> onCancel;
};