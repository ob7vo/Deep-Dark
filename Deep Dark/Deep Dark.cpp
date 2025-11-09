//#include "imgui/imgui.h"
//#include "imgui/imgui-SFML.h"
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
const int ASPECT_WIDTH = 900;
const int ASPECT_HEIGHT = 800;
sf::Vector2f MOUSE_POS{ 0.f,0.f };
sf::Color WINDOW_COLOR(54, 2, 11);

sf::Text fpsText(baseFont);
float timeSinceLastFPSUpdate = 0.0f;
const float FPSTimer = 0.5f;

void set_mouse_position(sf::RenderWindow& window, Camera& cam) {
    sf::Vector2i mouseScreenPos = sf::Mouse::getPosition(window);
    MOUSE_POS = window.mapPixelToCoords(mouseScreenPos);
    cam.set_mouse_pos(MOUSE_POS, mouseScreenPos);
}
void set_fps_text(float deltaTime) {
    timeSinceLastFPSUpdate += deltaTime;
    if (timeSinceLastFPSUpdate < FPSTimer) return;

    timeSinceLastFPSUpdate = 0.0f;
    float fps = 1 / deltaTime;
    std::stringstream ss;
    ss << "FPS: " << std::fixed << std::setprecision(1) << fps;
    fpsText.setString(ss.str());
}
int main()
{
    std::cout << "SFML Version: "
        << SFML_VERSION_MAJOR << "."
        << SFML_VERSION_MINOR << "."
        << SFML_VERSION_PATCH << std::endl;

    sf::Clock clock;
    sf::RenderWindow window(sf::VideoMode({ ASPECT_WIDTH, ASPECT_WIDTH }), "SFML works!");
    //window.setFramerateLimit(FRAMERATE_LIMIT);

    TextureManager::initialize();
    Surge::init_animations();
    BaseCannon::init_animations();

    SaveSystem::Get().initialize();

    fpsText.setCharacterSize(20);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition({ 600, 50 });

    Camera cam(window);
    StateManager stateManager(cam);

    // pair <id, gear>
    /*
    std::vector<std::pair<int,int>> slots =  { {0,3}, {1,2} };
    std::ifstream stageFile("configs/stage_data/stage_1.json");
    json stageJson = json::parse(stageFile);
    stageFile.close();
    
    StageEnterData stageEnterData(stageJson, slots, {0, -1});
    */
    PrepEnterData prepData(MenuType::ARMORY_EQUIP, MenuType::HOME_BASE);
   // OnStateEnterData enterData(GameState::Type::MAIN_MENU);
    stateManager.switch_state(&prepData);

    while (window.isOpen())
    {
        set_mouse_position(window, cam);
        float deltaTime = clock.restart().asSeconds();
       // deltaTime = std::min(deltaTime, MAX_DELTA_TIME);

        while (const std::optional event = window.pollEvent()){
            stateManager.handle_events(*event);

            if (event->is<sf::Event::Closed>())
                window.close();
            else if (auto size = event->getIf<sf::Event::Resized>()) {
                // Update the view to the new size
                cam.view.setSize(sf::Vector2f(size->size));
                cam.uiView.setSize(sf::Vector2f(size->size));
                window.setView(cam.view);
            }
        }

        set_fps_text(deltaTime);

        window.clear(WINDOW_COLOR);

        cam.queue_ui_draw(&fpsText);
        stateManager.update(deltaTime);
        stateManager.render();

        window.display();

        if (auto enterData = stateManager.gameState->get_next_state()) 
            stateManager.switch_state(enterData);
    }
}

/*
   std::cout << "{(" << _rect.position.x << ", " << _rect.position.y << ")"
        << ", (" << _rect.size.x << ", " << _rect.size.y << ")}" << std::endl;
*/
 // Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

  //  "cannon_json_path": "configs/base_data/enemy_bases/orbital_base.json",
  //  "cannon_json_path": "configs/base_data/player_bases/p_wave_base.json",
   /*   "forced_spawn_times": [
        [ 1.5, 1 ],
        [ 3.3, 0 ]
      ]*/
      /*"forced_spawn_times": [
            [ 3.5, 1 ],
            [ 4.0, 2 ],
            [ 5.0, 2 ],
            [ 7.3, 1 ]
          ]*/