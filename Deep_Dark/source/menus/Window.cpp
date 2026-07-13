#include "pch.h"
#include "Window.h"
#include "Camera.h"
#include "UILayout.h"

template<int BUTTONS>
void Window<BUTTONS>::draw() {
	if (!this->visible) return; 
	this->cam.renderer.queue_ui_draw(&windowBorderSprite);
	this->cam.renderer.queue_ui_draw(&titleBarSprite);
	this->buttonManager.draw(this->cam);
}

template<int BUTTONS>
void Window<BUTTONS>::reset_positions() {
    this->sync_layout();
}

template<int BUTTONS>
bool Window<BUTTONS>::on_mouse_press(bool isM1) {
    sf::Vector2f mousePos = this->cam.getMouseWorldPos();
    this->activeRegion = this->get_region(mousePos);

    if (this->activeRegion == WindowRegion::TitleBar) {
        mouseDragOffset = mousePos - pos;
    }
    // Resize regions don't need an offset: resize() reads mousePos directly each call.
    return true;
}
template<int BUTTONS>
void Window<BUTTONS>::on_mouse_hold(bool isM1) {
    switch (this->activeRegion) {
        case WindowRegion::TitleBar:
            this->move();
            break;
        case WindowRegion::None:
        case WindowRegion::Interior:
            break;
        default:
            this->resize();
            break;
    }
}
template<int BUTTONS>
bool Window<BUTTONS>::on_mouse_release(bool isM1) {
    this->activeRegion = WindowRegion::None;
    return true;
}

template<int BUTTONS>
void Window<BUTTONS>::resize() {
	if (!this->visible) return;

    sf::Vector2f mousePos = this->cam.getMouseWorldPos();

    float left   = windowBounds.position.x;
    float top    = windowBounds.position.y;
    float right  = left + windowBounds.size.x;
    float bottom = top  + windowBounds.size.y;
 
    switch (activeRegion) {
        case WindowRegion::Left:
        case WindowRegion::TopLeft:
        case WindowRegion::BottomLeft:
            left = std::min(mousePos.x, right - MIN_WINDOW_WIDTH);
            break;
        default: break;
    }
    switch (activeRegion) {
        case WindowRegion::Right:
        case WindowRegion::TopRight:
        case WindowRegion::BottomRight:
            right = std::max(mousePos.x, left + MIN_WINDOW_WIDTH);
            break;
        default: break;
    }
    switch (activeRegion) {
        case WindowRegion::Top:
        case WindowRegion::TopLeft:
        case WindowRegion::TopRight:
            top = std::min(mousePos.y, bottom - MIN_WINDOW_HEIGHT);
            break;
        default: break;
    }
    switch (activeRegion) {
        case WindowRegion::Bottom:
        case WindowRegion::BottomLeft:
        case WindowRegion::BottomRight:
            bottom = std::max(mousePos.y, top + MIN_WINDOW_HEIGHT);
            break;
        default: break;
    }
 
    sf::Vector2f newPos = { left, top };
    sf::Vector2f delta  = newPos - pos; // only nonzero when dragging a Left/Top edge or corner
    pos = newPos;
    windowBounds = sf::FloatRect({ left, top }, { right - left, bottom - top });
    sync_layout(delta);
    // NOTE: this only translates buttons when the top-left corner moves (Left/Top/
    // TopLeft/TopRight/BottomLeft drags). Dragging Right/Bottom grows the window
    // without moving buttons, which is correct for buttons anchored top-left, but
    // any button meant to hug the right or bottom edge (e.g. a close button, or
    // resize grips) will NOT follow along — that needs separate anchor logic in
    // ButtonManager/Button that this file has no visibility into.
}
template<int BUTTONS>
void Window<BUTTONS>::move() {
	if (!this->visible) return;

    sf::Vector2f newPos = this->cam.getMouseWorldPos() - mouseDragOffset;
    sf::Vector2f delta  = newPos - pos;
    pos = newPos;

    sync_layout(delta); // whole window translates -> buttons translate by the same delta
}

template<int BUTTONS>
void Window<BUTTONS>::set_bounds(sf::FloatRect bounds) {
    windowBounds = bounds;
    pos = bounds.position;
    sync_layout();
}
template<int BUTTONS>
void Window<BUTTONS>::sync_layout(sf::Vector2f buttonDelta) {
    windowBounds.position = pos;
 
    titleBarBounds = sf::FloatRect(pos, { windowBounds.size.x, TITLE_BAR_HEIGHT });
 
    // Solid-color sprites (defaultTexture is assumed to be a flat 1x1 or NxN white
    // texture), so non-uniform scaling here does not visibly distort anything.
    windowBorderSprite.setPosition(windowBounds.position);
    sf::Vector2u borderTexSize = windowBorderSprite.getTexture().getSize();
    windowBorderSprite.setScale({
        windowBounds.size.x / static_cast<float>(borderTexSize.x),
        windowBounds.size.y / static_cast<float>(borderTexSize.y)
    });
 
    titleBarSprite.setPosition(titleBarBounds.position);
    sf::Vector2u titleTexSize = titleBarSprite.getTexture().getSize();
    titleBarSprite.setScale({
        titleBarBounds.size.x / static_cast<float>(titleTexSize.x),
        titleBarBounds.size.y / static_cast<float>(titleTexSize.y)
    });
 
    // Button positions in this codebase are absolute (ButtonManager::on_mouse_press
    // and check_mouse_hover compare mouse coords directly against button.sprite
    // bounds — no parent-relative offset). So the fix is to translate every button
    // by the same delta the window's top-left corner moved. Requires adding
    // ButtonManager<BUTTONS>::translate(sf::Vector2f, int, int) — see the snippet
    // provided separately, since ButtonManager.h/Button.h weren't shared here.
    if (buttonDelta.x != 0.f || buttonDelta.y != 0.f) {
        this->buttonManager.translate(buttonDelta);
    }
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
