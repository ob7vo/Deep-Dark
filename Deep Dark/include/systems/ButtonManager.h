#pragma once
#include "Button.h"
#include <array>

const bool LEFT_CLICK = true;
const bool RIGHT_CLICK = false;

template<int BUTTONS>
struct ButtonManager
{
	std::array<Button, BUTTONS> buttons;
	ButtonManager() = default;

	void draw(Camera& cam, int start = 0, int end = BUTTONS);
	void check_mouse_hover(sf::Vector2i mPos, int start = 0, int end = BUTTONS);
	bool on_mouse_press(sf::Vector2i mPos, bool m1, int start = 0, int end = BUTTONS);
};

