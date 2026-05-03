#pragma once
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include "ButtonManager.h"
#include "UILayout.h"

const sf::Font baseFont("fonts/KOMIKAX_.ttf");

enum class MenuType {
	MAIN_MENU,
	HOME_BASE,
	STAGE_SELECT,
	STAGE_SET,
	ARMORY_EQUIP,
	WORKSHOP_MENU,
};

struct MenuBase
{
	Camera& cam;

	bool paused = false;
	bool visible = true;
	bool clickable = true;

	explicit MenuBase(Camera& cam) : cam(cam) {};
	virtual ~MenuBase() = default;

	virtual void draw() = 0;
	/// <summary>Return value prevents camera drag when buttons are clicked</summary>
	virtual bool on_mouse_press(bool isM1) = 0; 
	virtual bool on_mouse_release(bool isM1) { return true; }
	virtual void check_mouse_hover() = 0;

	virtual void reset_positions() = 0;
	virtual void slide(float t) {};

	virtual void update(float dt) {}

	virtual void on_enter() { reset_positions(); };
	virtual void on_exit() {};

	virtual void activate() { visible = clickable = true; }
	virtual void deactivate() { visible = clickable = false; }
	
	static std::string GetMenuName(MenuType type);
};

template<int BUTTONS>
struct ButtonMenu : public MenuBase
{
	ButtonManager<BUTTONS> buttonManager;
	explicit ButtonMenu(Camera& cam) : MenuBase(cam) {}

	// Virtual with default — subclasses can extend
	bool on_mouse_press(bool isM1) override;
	bool on_mouse_release(bool isM1) override { return true; }
	void check_mouse_hover() override;
};

template<int BUTTONS>
struct Menu : public ButtonMenu<BUTTONS>
{
	explicit Menu(Camera& cam) : ButtonMenu<BUTTONS>(cam) {};
	~Menu() override = default;

	void draw() override;
};

