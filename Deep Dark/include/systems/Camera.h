#pragma once
#include <SFML/Graphics.hpp>
#include "Bounds.h"
#include "TextureManager.h"
#include "MouseCursor.h"
#include <iostream>

const float MIN_ZOOM = 0.6f;
const float MAX_ZOOM = 1.4f;
const float FRICTION = 0.85f;
const float ZOOM_SPEED = 0.1f;
const int FIXED_FRAMERATE = 30;
const float FIXED_DELTA_TIME = 0.0333f;

const sf::Texture defTexture("sprites/defaults/defaultTexture.png");
const sf::Color blackTransperent(0, 0, 0, 128);
const sf::Color redTransperent(255, 0, 0, 128);

class Camera
{
private:
	sf::RenderWindow& window;
	float worldHeight = 100.f;

	Bounds limits;
	bool locked = true;

	std::vector<sf::Drawable*> uiDrawQueue;
	std::vector<std::unique_ptr<sf::Drawable>> tempUIDrawQueue; // for UI created on the Stack
	std::vector<sf::Drawable*> worldDrawQueue;

	sf::Vector2f pos = { 0.f,0.f };
	sf::View worldView;
	sf::View uiView;

public:
	float zoomLevel = 1;
	MouseCursor cursor;

	explicit Camera(sf::RenderWindow& window);
	sf::VertexArray create_dark_overlay(float left, float top, float width, float height, float percentage = 0.f) const;

	void update(float deltaTime);
	void update_projection();

	void draw_all_ui();
	void draw_grey_screen(float opacity = 0.5f);

	void handle_events(sf::Event event);
	void on_mouse_press(sf::Event::MouseButtonPressed click);
	void on_mouse_released(sf::Event::MouseButtonReleased release);

	void zoom(sf::Event::MouseWheelScrolled scroll);
	void zoom(sf::Keyboard::Key key);
	void apply_velocity(float deltaTime);
	void move(sf::Keyboard::Key key);
	void click_and_drag();

	sf::Vector2f get_pos_within_bounds(sf::Vector2f targetPos) const;
	void update_pos(sf::Vector2f pos);

	inline void reset() {
		worldView = window.getDefaultView();
		uiView = worldView;
		pos = worldView.getCenter();
		cursor.dragging = false;
	}

	/// <summary> Will cull Drawable if a Rect in passed in for its position + size </summary>
	inline void queue_world_draw(sf::Drawable* draw, sf::FloatRect rect = {}) {
		if (rect.size.x == 0 || within_camera(rect)) worldDrawQueue.emplace_back(draw);
	}
	inline void queue_ui_draw(sf::Drawable* draw) { uiDrawQueue.emplace_back(draw); }
	template<typename T>
	inline void queue_temp_ui_draw(const T& drawable) {
		static_assert(std::is_base_of<sf::Drawable, T>::value, "Must be drawable");
		tempUIDrawQueue.emplace_back(std::make_unique<T>(drawable));  // Copies T
	}
	inline void clear_queues() {
		uiDrawQueue.clear();
		tempUIDrawQueue.clear();
		worldDrawQueue.clear();
	}

	inline bool within_camera(sf::FloatRect rect) const {
		sf::Vector2f center = worldView.getCenter();
		sf::Vector2f size = worldView.getSize();

		return !(
			rect.position.x + rect.size.x < center.x - size.x * .5f ||// Too far left
			rect.position.x > center.x + size.x * .5f ||              // Too far right
			rect.position.y + rect.size.y < center.y - size.y * .5f ||// Too far up
			rect.position.y > center.y + size.y * .5f                 // Too far down
			);
	}

	inline void change_lock(bool lock) {
		if (lock) {
			locked = true;
			cursor.dragging = false;
			cursor.velocity = { 0.f, 0.f };
		}
		else
			locked = false;
	}

	void set_cursor_ui(sf::Texture& uiTex, sf::Vector2f normOrigin, float opacity, sf::Vector2f normScale);

	inline sf::Vector2f norm_to_pixels(sf::Vector2f norm) const {
		return  uiView.getSize() * norm;
	}
	inline sf::Vector2f norm_to_pixels_size(sf::Vector2f norm) const {
   	 	return norm * std::min(uiView.getSize().x, uiView.getSize().y); // both use same dimension!
    }

	sf::Vector2f get_norm_sprite_scale(const sf::Sprite& sprite, sf::Vector2f normScale) const;
	unsigned int get_norm_font_size(sf::Text& text, float normHeight) const;
	void set_sprite_params(sf::Vector2f normPos, sf::Vector2f scale,
		const std::string& path, sf::Texture& texture, sf::Sprite& sprite) const;

	inline void close_window() { window.close(); }
	inline sf::RenderWindow& get_window() const { return window; }
	inline sf::Vector2f& getMouseWorldPos() { return cursor.worldPos; }
	inline sf::Vector2i& getMouseScreenPos() { return cursor.screenPos; }
};
