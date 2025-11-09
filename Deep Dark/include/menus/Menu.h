#pragma once
#include <SFML/Graphics.hpp>
#include "ButtonManager.h"

const sf::Font baseFont("fonts/KOMIKAX_.ttf");

enum class MenuType {
	START,
	HOME_BASE,
	STAGE_SELECT,
	ARMORY_EQUIP,
	AUGMENT,
	STAGE_PAUSE
};

struct MenuBase
{
	Camera& cam;

	bool paused = false;
	bool visible = true;

	MenuBase(Camera& cam) : cam(cam) {};
	virtual ~MenuBase() = default;

	virtual void draw() = 0;
	virtual bool on_mouse_press(bool isM1) = 0;
	virtual bool on_mouse_release(bool isM1) = 0;
	virtual void check_mouse_hover() = 0;
	virtual void reset_positions() = 0;
	virtual void update(float dt) = 0;
};

template<int BUTTONS>
struct Menu : public MenuBase
{
	ButtonManager<BUTTONS> buttonManager;

	Menu(Camera& cam) : MenuBase(cam) {};
	~Menu() = default;

	virtual void draw() = 0;
	virtual bool on_mouse_press(bool isM1) override {
		return buttonManager.on_mouse_press(cam.getMouseScreenPos(), isM1);
	}
	virtual bool on_mouse_release(bool isM1) { return true; }

	virtual void check_mouse_hover() override {
		buttonManager.check_mouse_hover(cam.getMouseScreenPos());
	}
	virtual void reset_positions() = 0;
	virtual void update(float dt) override {}

	inline Button& get_button(int index) { return buttonManager.buttons[index]; }
};

