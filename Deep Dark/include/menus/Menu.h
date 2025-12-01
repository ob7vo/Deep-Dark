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

	explicit MenuBase(Camera& cam) : cam(cam) {};
	virtual ~MenuBase() = default;

	virtual void draw() = 0;
	/// <summary>Return value prevents camera drag when buttons are clicked</summary>
	virtual bool on_mouse_press(bool isM1) = 0; 
	virtual bool on_mouse_release(bool isM1) { return true; }
	virtual void check_mouse_hover() = 0;
	virtual void reset_positions() = 0;
	virtual void update(float dt) {}
};

template<int BUTTONS>
struct Menu : public MenuBase
{
	ButtonManager<BUTTONS> buttonManager;

	explicit Menu(Camera& cam) : MenuBase(cam) {};
	~Menu() override = default;

	void draw() override;
	/// <summary>Return value prevents camera drag when buttons are clicked</summary>
	bool on_mouse_press(bool isM1) override;
	inline bool on_mouse_release(bool isM1) override { return true; }
	void check_mouse_hover() override;
};

