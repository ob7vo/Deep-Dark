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

	sf::FloatRect windowBounds;
	sf::FloatRect titleBarBounds;
    sf::Sprite titleBarSprite = sf::Sprite(defaultTexture);// must be its own sprite for proper resizing
	sf::Sprite windowBorderSprite = sf::Sprite(defaultTexture);

	// recalculated every frame from the hover position, used for cursor feedback.
    WindowRegion mouseRegion  = WindowRegion::None;
	// locked in on mouse-down, and drives move()/resize() until the button is released.
    WindowRegion activeRegion = WindowRegion::None;

	explicit Window(Camera& cam) : ButtonMenu<BUTTONS>(cam) {};
	~Window() override = default;

	static constexpr float WINDOW_BORDER_MARGIN = 6.f; // pixel threshold near edge
    static constexpr float TITLE_BAR_HEIGHT     = 40.f;
    static constexpr float MIN_WINDOW_WIDTH     = 120.f;
    static constexpr float MIN_WINDOW_HEIGHT    = 80.f;
 
	WindowRegion get_region(sf::Vector2f mousePos);
 
	void draw() override;
	void reset_positions() override; // required by MenuBase; was previously unimplemented
	
	bool on_mouse_press(bool isM1) final;
	void on_mouse_hold(bool isM1) final;
	bool on_mouse_release(bool isM1) final;

	void resize();
	void move();
 
    // Initialize windowBouunds/pos once, then call this to derive everything else.
    void set_bounds(sf::FloatRect bounds);
 
private:
    void sync_layout(sf::Vector2f buttonDelta = {0.f, 0.f});

};