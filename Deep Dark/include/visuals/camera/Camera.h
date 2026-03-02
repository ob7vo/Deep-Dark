#pragma once
#include "MouseCursor.h"
#include "CameraTransform.h"
#include "CameraRenderer.h"
#include "UILayout.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

const sf::Texture defTexture("sprites/defaults/defaultTexture.png");

class Camera
{
	friend struct CameraTransform;
	friend struct CameraRenderer;
	friend struct MouseCursor;

private:
	sf::RenderWindow& window;
	float worldHeight = 100.f;

	sf::View worldView;
	sf::View uiView;

	MouseCursor cursor;
public:
	CameraTransform transform;
	CameraRenderer renderer;

	explicit Camera(sf::RenderWindow& window);

	void update(float deltaTime);
	void update_pos();
	inline void update_mouse_pos(sf::Vector2f worldPos, sf::Vector2i screenPos) {
		cursor.set_pos(worldPos, screenPos);
	}

	void handle_events(sf::Event event);
	void on_mouse_press(sf::Event::MouseButtonPressed click);
	void on_mouse_released(sf::Event::MouseButtonReleased release);

	void reset();

	void change_lock(bool lock);
	void set_cursor_ui(const sf::Texture& uiTex, sf::Vector2f normOrigin, float opacity, sf::Vector2f normScale);

	sf::Vector2f norm_to_pixels(sf::Vector2f norm) const;
	sf::Vector2f norm_to_pixels_size(sf::Vector2f norm) const;
	sf::Vector2f get_norm_sprite_scale(const sf::Sprite& sprite, sf::Vector2f normScale) const;
	unsigned int get_norm_font_size(sf::Text& text, float normHeight) const;
	bool setup_sprite(sf::Vector2f normPos, sf::Vector2f scale, sf::Sprite& sprite, 
		const sf::Texture& texture, sf::IntRect textureRect = {}) const;
	std::pair<float, float> norm_to_pixels_pair(sf::Vector2f norm) const;

	inline void close_window() { window.close(); }
	inline sf::RenderWindow& getWindow() const { return window; }
	inline const sf::Vector2f& getMouseWorldPos() const { return cursor.worldPos; }
	inline const sf::Vector2i& getMouseScreenPos() const { return cursor.screenPos; }

	inline bool doubleClicked() const { return cursor.lastClickTime <= DOUBLE_CLICK_WINDOW; }
	inline float getZoomLevel() { return transform.zoomLevel; }
	inline sf::Vector2f getPosition() { return transform.worldPos; }
};