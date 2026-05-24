#include "pch.h"
#include "Window.h"
#include "Camera.h"
#include "UILayout.h"

template<int BUTTONS>
void Window<BUTTONS>::draw() {
	if (!this->visible) return; 
	this->cam.renderer.queue_ui_draw(&windowBorderSprite);
	this->buttonManager.draw(this->cam);
}
template<int BUTTONS>
void Window<BUTTONS>::resize() {
	if (!this->visible) return;
	this->cam.renderer.queue_ui_draw(&windowBorderSprite);
	this->buttonManager.draw(this->cam);
}
template<int BUTTONS>
void Window<BUTTONS>::move() {
	if (!this->visible) return;
	this->cam.renderer.queue_ui_draw(&windowBorderSprite);
	this->buttonManager.draw(this->cam);
}
template<int BUTTONS>
WindowRegion Window<BUTTONS>::get_region(sf::Vector2f mousePos)
{
    sf::FloatRect bounds = windowBorderSprite.getGlobalBounds();
    sf::FloatRect expandedBounds = sf::FloatRect(
        {bounds.position.x - WINDOW_BORDER_MARGIN, bounds.position.y - WINDOW_BORDER_MARGIN},
        {bounds.size.x + WINDOW_BORDER_MARGIN * 2, bounds.size.y + WINDOW_BORDER_MARGIN * 2}
    );
    if (!expandedBounds.contains(mousePos)) return WindowRegion::None;

    bool left = mousePos.x < bounds.position.x + WINDOW_BORDER_MARGIN;
    bool right = mousePos.x > bounds.position.x + bounds.size.x - WINDOW_BORDER_MARGIN;
    bool top = mousePos.y < bounds.position.y + WINDOW_BORDER_MARGIN;
    bool bottom = mousePos.y > bounds.position.y + bounds.size.y - WINDOW_BORDER_MARGIN;
    bool titleBarBottom = mousePos.y < bounds.position.y + titleBarBounds.size.y;

    if (!top && !left && titleBarBottom) return WindowRegion::TitleBar;
    if (top && left)  return WindowRegion::TopLeft;
    else if (top && right) return WindowRegion::TopRight;
    else if (bottom && left) return WindowRegion::BottomLeft;
    else if (bottom && right) return WindowRegion::BottomRight;
    else if (top) return WindowRegion::Top;
    else if (bottom) return WindowRegion::Bottom;
    else if (left)   return WindowRegion::Left;
    else if (right)  return WindowRegion::Right;

    return WindowRegion::Interior;
}

template struct Window<UI::ArmoryMenu::BTN_COUNT>;
template struct Window<UI::StageSelect::BTN_COUNT>;
template struct Window<UI::StageSelect::StageNode::BTN_COUNT>;
template struct Window<UI::ArmoryMenu::StagePreview::BTN_COUNT>;
//template struct Window<UI::StageUI::BTN_COUNT>;
template struct Window<UI::StageUI::PauseMenu::BTN_COUNT>;
//template struct Window<UI::StageUI::ResultsScreen::BTN_COUNT>;
//template struct Window<UI::StartMenu::BTN_COUNT>;
template struct Window<UI::Workshop::BTN_COUNT>;
