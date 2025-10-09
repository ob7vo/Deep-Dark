#include "Camera.h"
Camera::Camera(float zoomLevel) : zoomLevel(zoomLevel) {

}
sf::View Camera::get_view(sf::Vector2u windowSize) {
	float aspect = (float)windowSize.x / (float)windowSize.y;
	sf::Vector2f size = {};
	if (aspect < 1)
		size = { zoomLevel, zoomLevel / aspect };
	else
		size = { zoomLevel * aspect, zoomLevel };
	
	sf::Vector2f center = { (float)windowSize.x / 2,(float)windowSize.y / 2 };
	view = sf::View(center, size);
	return view;
}