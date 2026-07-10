#include "pch.h"
#include "Game.h"

const float MAX_DELTA_TIME = 0.033f;
const unsigned int FRAMERATE_LIMIT = 60;
const int ASPECT_WIDTH = 1000;
const int ASPECT_HEIGHT = 850;
const sf::Color WINDOW_COLOR(54, 2, 11);

const float FPSTimer = 0.5f;

Game Game::game;

Game::Game() : 
    renderWindow(sf::VideoMode({ ASPECT_WIDTH, ASPECT_HEIGHT }), "SFML works!",
        sf::Style::Titlebar | sf::Style::Close),
    camera(renderWindow),
    stateManager(camera)
{}

void Game::Initialize(){
    bool overwritePlayerSave = false;
    bool overwriteStageSave = true; 
    bool overwriteUnitSave = false;
    SaveSystem::Initialize(overwritePlayerSave, overwriteStageSave, overwriteUnitSave);

    Textures::initializeAll();
    Surge::init_animations();

    fpsText.setCharacterSize(20);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition({ 600, 50 });
}

void Game::SetMousePosition() {
    sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(renderWindow);
    sf::Vector2f mouseWorldPos = renderWindow.mapPixelToCoords(mouseScreenPos);
    camera.update_mouse_pos(mouseWorldPos, mouseScreenPos);
}
void Game::SetFPSText(float deltaTime, float& time) {
    time += deltaTime;
    if (time < FPSTimer) return;

    time = 0.0f;
    float fps = 1 / deltaTime;

    fpsText.setString(std::format("FPS: {}", fps));
}

void Game::Start() {
    return;

    //Choosing starting state for testing
    std::array<ArmorySlot, UnitConfig::MAX_EQUIP_SLOTS> slots = ArmorySlot::default_armory_loadout(camera);
    StageEnterData stageEnterData(0, 0, slots, false);
    //*/

    PrepEnterData prepData(MenuType::STAGE_SELECT, MenuType::ARMORY_EQUIP);
    OnStateEnterData enterData(GameState::Type::MAIN_MENU);
    stateManager.switch_state(&prepData);

    Run();
}

void Game::Run() {
    sf::Clock clock;
    float timeSinceLastFPSUpdate = 0.0f;

    while (renderWindow.isOpen())
    {
        float deltaTime = std::min(clock.restart().asSeconds(), MAX_DELTA_TIME);
        SetMousePosition();
        SetFPSText(deltaTime, timeSinceLastFPSUpdate);

        PollEvents();

        renderWindow.clear(WINDOW_COLOR);

        camera.renderer.queue_ui_draw(&fpsText);
        stateManager.update(deltaTime);
        stateManager.render();

        renderWindow.display();

        if (auto newEnterData = stateManager.gameState->get_next_state()) 
            stateManager.switch_state(newEnterData);
    }
}
void Game::PollEvents() {
    while (const std::optional event = renderWindow.pollEvent()){
        stateManager.handle_events(*event);

        if (event->is<sf::Event::Closed>()) {
            SaveSystem::SaveAll();
            renderWindow.close();
        }
    }
}

//indent line