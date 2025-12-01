#include "pch.h"
#include "game_saves/SaveSystem.h"
#include "StageManager.h"
#include "StateManager.h"
#include "PreparationState.h"
#include "ButtonManager.h"
#include "Tween.h"
#include "StageGameState.h"

using json = nlohmann::json;

const float MAX_DELTA_TIME = 0.033f;
const unsigned int FRAMERATE_LIMIT = 60;
const int ASPECT_WIDTH = 1728;
const int ASPECT_HEIGHT = 972;
const sf::Color WINDOW_COLOR(54, 2, 11);


const float FPSTimer = 0.5f;

void set_mouse_position(const sf::RenderWindow& window, Camera& cam) {
    sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
    sf::Vector2f MOUSE_POS = window.mapPixelToCoords(mouseScreenPos);
    cam.cursor.set_pos(MOUSE_POS, mouseScreenPos);
}
void set_fps_text(float deltaTime, float& time, sf::Text& fpsText) {
    time += deltaTime;
    if (time < FPSTimer) return;

    time = 0.0f;
    float fps = 1 / deltaTime;

    fpsText.setString(std::format("FPS: {}", fps));
}
int main()
{
    sf::Text fpsText(baseFont);
    float timeSinceLastFPSUpdate = 0.0f;

    std::cout << "SFML Version: "
        << SFML_VERSION_MAJOR << "."
        << SFML_VERSION_MINOR << "."
        << SFML_VERSION_PATCH << std::endl;

    sf::Clock clock;
    sf::RenderWindow window(sf::VideoMode({ ASPECT_WIDTH, ASPECT_HEIGHT }), "SFML works!",
        sf::Style::Titlebar | sf::Style::Close);
 
    TextureManager::initialize();
    BaseCannon::init_animations();

    SaveSystem::Get().initialize();

    fpsText.setCharacterSize(20);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition({ 600, 50 });

    Camera cam(window);
    StateManager stateManager(cam);

    /**/
    std::array<ArmorySlot, 10> slots = ArmorySlot::default_armory_loadout();
    std::string stageJsonPath = "configs/stage_data/stage_1.json";
    StageEnterData stageEnterData(stageJsonPath, 0, slots);
    //*/

    PrepEnterData prepData(MenuType::ARMORY_EQUIP, MenuType::STAGE_SELECT);
    OnStateEnterData enterData(GameState::Type::MAIN_MENU);
    stateManager.switch_state(&prepData);

    while (window.isOpen())
    {
        set_mouse_position(window, cam);
        float deltaTime = clock.restart().asSeconds();
        deltaTime = std::min(deltaTime, MAX_DELTA_TIME);

        while (const std::optional event = window.pollEvent()){
            stateManager.handle_events(*event);

            if (event->is<sf::Event::Closed>())
                window.close();
        }

        set_fps_text(deltaTime, timeSinceLastFPSUpdate, fpsText);

        window.clear(WINDOW_COLOR);

        cam.queue_ui_draw(&fpsText);
        stateManager.update(deltaTime);
        stateManager.render();

        window.display();

        if (auto newEnterData = stateManager.gameState->get_next_state()) 
            stateManager.switch_state(newEnterData);
    }
}

 // Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
