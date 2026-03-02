#include "pch.h"
#include "CameraRenderer.h"
#include "Camera.h"

void CameraRenderer::draw() {
	cam.window.setView(cam.uiView);

	for (auto& p_Draw : uiDrawQueue)
		if (p_Draw) cam.window.draw(*p_Draw);
	uiDrawQueue.clear();

	for (const auto& u_draw : tempUIDrawQueue)
		if (u_draw) cam.window.draw(*u_draw);
	tempUIDrawQueue.clear();

	cam.window.setView(cam.worldView);

	for (auto& draw : worldDrawQueue)
		if (draw) cam.window.draw(*draw);
	worldDrawQueue.clear();
}
void CameraRenderer::clear_queues() {
	uiDrawQueue.clear();
	tempUIDrawQueue.clear();
	worldDrawQueue.clear();
}

sf::VertexArray CameraRenderer::create_dark_overlay(float left, float top, float width, float height, sf::Color color, float fill) const {
	auto darkOverlay = sf::VertexArray(sf::PrimitiveType::TriangleStrip, 4);

	float darkHeight = height * fill;

	darkOverlay[0].position = { left, top + height - darkHeight };
	darkOverlay[0].color = color;

	darkOverlay[1].position = { left + width, top + height - darkHeight };
	darkOverlay[1].color = color;

	darkOverlay[2].position = { left, top + height };
	darkOverlay[2].color = color;

	darkOverlay[3].position = { left + width, top + height };
	darkOverlay[3].color = color;

	return darkOverlay;
}
void CameraRenderer::darken_screen(float opacity) {
	sf::VertexArray darkOverlay = create_dark_overlay(0, 0, cam.uiView.getSize().x, cam.uiView.getSize().y);

	for (int i = 0; i < 4; i++)
		darkOverlay[i].color.a = (uint8_t)(255 * opacity);

	queue_temp_ui_draw(darkOverlay);
}
void CameraRenderer::draw_overlay(const sf::Sprite& sprite, sf::Color color, float fill) {
	sf::FloatRect bounds = sprite.getGlobalBounds();
	float left = bounds.position.x;
	float top = bounds.position.y;
	float height = bounds.size.y;
	float width = bounds.size.x;

	const sf::VertexArray overlay = create_dark_overlay(left, top, width, height, color, fill);
	queue_temp_ui_draw(overlay);
}

void CameraRenderer::draw_cursor_ui() { queue_ui_draw(&cam.cursor.ui); }

bool CameraRenderer::within_camera(sf::FloatRect rect) const {
	sf::Vector2f center = cam.worldView.getCenter();
	sf::Vector2f size = cam.worldView.getSize();

	return !(
		rect.position.x + rect.size.x < center.x - size.x * .5f ||// Too far left
		rect.position.x > center.x + size.x * .5f ||              // Too far right
		rect.position.y + rect.size.y < center.y - size.y * .5f ||// Too far up
		rect.position.y > center.y + size.y * .5f                 // Too far down
		);
}