#pragma once
#include "Bounds.h"
#include "MouseCursor.h"
#include "UILayout.h"

const float MIN_ZOOM = 0.6f;
const float MAX_ZOOM = 1.4f;
const float FRICTION = 0.85f;
const float ZOOM_SPEED = 0.1f;
const int FIXED_FRAMERATE = 30;
const float FIXED_DELTA_TIME = 0.0333f;

const sf::Texture defTexture("sprites/defaults/defaultTexture.png");

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
	sf::VertexArray create_dark_overlay(float left, float top, float width, float height, sf::Color color = UI::Colors::BLACK_TRANSPARENT, float fill = 1.f) const;

	void update(float deltaTime);
	void update_projection();

	void draw_all_ui();
	void draw_overlay(const sf::Sprite& sprite, sf::Color color = UI::Colors::BLACK_TRANSPARENT, float fill = 1.f);
	void darken_screen(float opacity = 0.5f);

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

	void reset();

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
	void clear_queues();

	bool within_camera(sf::FloatRect rect) const;

	void change_lock(bool lock);
	void set_cursor_ui(sf::Texture& uiTex, sf::Vector2f normOrigin, float opacity, sf::Vector2f normScale);

	sf::Vector2f norm_to_pixels(sf::Vector2f norm) const;
	sf::Vector2f norm_to_pixels_size(sf::Vector2f norm) const;
	sf::Vector2f get_norm_sprite_scale(const sf::Sprite& sprite, sf::Vector2f normScale) const;
	unsigned int get_norm_font_size(sf::Text& text, float normHeight) const;
	bool setup_sprite(sf::Vector2f normPos, sf::Vector2f scale, sf::Sprite& sprite, 
		const sf::Texture& texture, sf::IntRect rect = {}) const;

	inline void close_window() { window.close(); }
	inline sf::RenderWindow& get_window() const { return window; }
	inline sf::Vector2f& getMouseWorldPos() { return cursor.worldPos; }
	inline sf::Vector2i& getMouseScreenPos() { return cursor.screenPos; }
};
