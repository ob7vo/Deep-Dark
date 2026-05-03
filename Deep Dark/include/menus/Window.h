#pragma once
#include "Menu.h"

enum class WindowRegion {
    TitleBar,
    Interior,
    None, Left, Right, Top, Bottom,
    TopLeft, TopRight, BottomLeft, BottomRight
};

// Movable and risizable window
template<int BUTTONS>
struct Window : public ButtonMenu<BUTTONS>
{
	sf::Vector2f pos;
	// offset from mouse position when window is being dragged
	sf::Vector2f mouseDragOffset; 

	sf::FloatRect windowBouunds;
	sf::FloatRect titleBarBounds;
    sf::Sprite titleBarSprite = sf::Sprite(defaultTexture);// must be its own sprite for proper resizing
	sf::Sprite windowBorderSprite = sf::Sprite(defaultTexture);
    WindowRegion mouseRegion = WindowRegion::None;

	explicit Window(Camera& cam) : ButtonMenu<BUTTONS>(cam) {};
	~Window() override = default;

    static constexpr float WINDOW_BORDER_MARGIN = 6.f; // pixel threshold near edge

	WindowRegion get_region(sf::Vector2f mousePos);

	void draw() override;
	void resize();
	void move();
};