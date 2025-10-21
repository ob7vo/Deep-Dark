#pragma once
#include <SFML/Graphics.hpp>
#include "UILayout.h"
#include "ButtonManager.h"

const sf::Font baseFont("fonts/KOMIKAX_.ttf");

template<int BUTTONS>
struct Menu
{
	ButtonManager<BUTTONS> buttonManager;
	Camera& cam;

	bool paused = false;
	bool visible = true;

	Menu(Camera& cam) : cam(cam) {};
	~Menu() = default;

	virtual void draw() = 0;
	virtual void register_click(sf::Vector2f mousePos) { 
		buttonManager.register_click(mousePos); 
	}
	virtual void check_mouse_hover(sf::Vector2f mousePos){
		buttonManager.check_mouse_hover(mousePos);
	}

	inline Button& get_button(int index) { return buttonManager.buttons[index]; }
	
};

