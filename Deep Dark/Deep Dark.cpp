//#include "imgui/imgui.h"
//#include "imgui/imgui-SFML.h"
#include <iostream>
#include <fstream>
#include "StageManager.h"
#include "ButtonManager.h"
#include "Tween.h"
#include "json.hpp"
#include "Camera.h"
using json = nlohmann::json;

const float MAX_DELTA_TIME = 0.033f;
const int ASPECT_WIDTH = 900;
const int ASPECT_HEIGHT = 800;
sf::Vector2f SPEED(0.2f,0.2f);

void print_unit_stats(const Unit& unit) {
   // printf("hp: %d\n", unit.hp);
   // printf("dmg: %d\n", unit.dmg);
   // printf("knockbacks: %d\n", unit.knockbacks);
}
sf::Vector2f get_mouse_position(sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    return window.mapPixelToCoords(mousePos);
}
int main()
{
    sf::Clock clock;
    sf::RenderWindow window(sf::VideoMode({ ASPECT_WIDTH, ASPECT_WIDTH }), "SFML works!");
    Camera cam(100.0f);
    cam.view = window.getDefaultView();

    Surge::init_animations();
    BaseCannon::init_animations();

    std::vector<std::string> slots = { "configs/player_units/soldier/soldier.json"};
    std::ifstream stageFile("configs/stage_data/stage_1.json");
    /*
    std::string content((std::istreambuf_iterator<char>(stageFile)),
        std::istreambuf_iterator<char>());
    std::cout << "File contents:\n" << content << std::endl;

    // Reset stream and parse
    stageFile.clear();
    stageFile.seekg(0);
    return 0;
    
    try {
        std::ifstream stageFile("configs/stage_data/stage_1.json");
        json stageJson = json::parse(stageFile);
    }
    catch (const json::parse_error& e) {
        std::cout << "Parse error: " << e.what() << std::endl;
        std::cout << "Error ID: " << e.id << std::endl;
        std::cout << "Byte position: " << e.byte << std::endl;
    }
    */
    json stageJson = json::parse(stageFile);
    stageFile.close();
    StageManager stageManager(stageJson, slots);

    ButtonManager buttonManager;
    std::function<void()> print = []() { std::cout << "you clicked me! (I sound so gay bro)" << std::endl; };
  //  buttonManager.add_button({ 300.0f,250.f }, {100.f, 50.f}, sf::Color::Cyan, print);
    while (window.isOpen())
    {
        sf::Vector2f mousePos = get_mouse_position(window);
        while (const std::optional event = window.pollEvent()){
            stageManager.handle_events(*event);
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (auto size = event->getIf<sf::Event::Resized>()) {
                // Update the view to the new size
     //         cam.view.setSize(sf::Vector2f(size->size));
     //         window.setView(cam.view);
                window.setView(cam.get_view(size->size));
            }
            else if (event->is<sf::Event::MouseButtonPressed>())
                buttonManager.register_click(mousePos);
        }

        float deltaTime = clock.restart().asSeconds();
        deltaTime = std::min(deltaTime, MAX_DELTA_TIME);

        window.clear();
        stageManager.update_game_ticks(window, deltaTime);
      //  buttonManager.tick(window, mousePos);
        window.display();
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