#pragma once
#include "SFML\Graphics.hpp"
#include "Bounds.h"

const float MIN_ZOOM = 0.6f;
const float MAX_ZOOM = 1.4f;
const float FRICTION = 0.85f;
const float ZOOM_SPEED = 0.1f;
const float FIXED_DELTA_TIME = 0.0333f;

class Camera
{
	sf::RenderWindow& window;
	sf::Vector2f mousePos = { 0.f, 0.f };

	Bounds limits;
	bool locked = false;
public:
	float zoomLevel = 1;

	float scrollSpeed = 4.f;
	sf::Vector2f pos = { 0.f,0.f };

	sf::View view;
	sf::View uiView;
	std::vector<sf::Drawable*> uiDrawQueue;
	std::vector<std::unique_ptr<sf::Drawable>> tempUIDrawQueue; // for UI created on the Stack

	sf::Vector2f dragOrigin{ 0.f,0.f };
	sf::Vector2f velocity{ 0.f,0.f };
	bool dragging = false;

	Camera(sf::RenderWindow& window);
	void update(float deltaTime);
	void draw_all_ui();

	void handle_events(sf::Event event);
	void register_click(sf::Event event);

	void zoom(sf::Event::MouseWheelScrolled scroll);
	void zoom(sf::Keyboard::Key key);
	void apply_velocity(float deltaTime);
	void move(sf::Keyboard::Key key);
	void click_and_drag();

	sf::Vector2f get_pos_within_bounds(sf::Vector2f targetPos);
	void update_pos(sf::Vector2f pos);

	inline void queue_ui_draw(sf::Drawable* draw) { uiDrawQueue.emplace_back(draw); }
	template<typename T>
	inline void queue_temp_ui_draw(const T& drawable) {
		static_assert(std::is_base_of<sf::Drawable, T>::value, "Must be drawable");
		tempUIDrawQueue.emplace_back(std::make_unique<T>(drawable));  // Copies T
	}

	inline void set_bounds(sf::FloatRect newBounds) { limits.set_bounds(newBounds); }
	inline Bounds get_bounds() { return limits; }
	inline void lock_camera() { locked = true; dragging = false; }
	inline void unlock_camera() { locked = false; }
	inline void set_mouse_pos(sf::Vector2f mPos) { mousePos = mPos; }
	inline bool has_velocity() {
		return std::abs(velocity.x) > 0.1f && std::abs(velocity.y) > 0.1f;
	}
};