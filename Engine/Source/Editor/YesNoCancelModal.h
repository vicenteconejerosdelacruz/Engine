#pragma once
#include <functional>

struct YesNoCancelModal
{
	YesNoCancelModal();
	void Init(std::string title, std::string text, std::function<void()> yes, std::function<void()> no, std::function<void()> cancel);
	void Show();
	void Hide();
	bool Showing();
	void Draw();

	bool show;
	std::string title;
	std::string text;
	std::function<void()> onYes;
	std::function<void()> onNo;
	std::function<void()> onCancel;
};