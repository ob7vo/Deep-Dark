#include "pch.h"
#include "Game.h"

using json = nlohmann::json;

const int ASPECT_WIDTH = 1000;
const int ASPECT_HEIGHT = 850;
const sf::Color WINDOW_COLOR(54, 2, 11);

int main()
{
    std::cout << "SFML Version: "
        << SFML_VERSION_MAJOR << "."
        << SFML_VERSION_MINOR << "."
        << SFML_VERSION_PATCH << std::endl;

    sf::ContextSettings settings;
    settings.antiAliasingLevel = 0;
    settings.majorVersion = 3;
    settings.minorVersion = 0;
    settings.attributeFlags = sf::ContextSettings::Default;

    sf::RenderWindow renderWindow(
        sf::VideoMode({ ASPECT_WIDTH, ASPECT_HEIGHT }), 
        "work nowww!",
        sf::Style::Titlebar | sf::Style::Close, 
        sf::State::Windowed,
        settings
    );
    renderWindow.setFramerateLimit(60);

   // Game::Get().Initialize();
    
    if (!ImGui::SFML::Init(renderWindow))
       return -1;

    sf::CircleShape shape(50.f);
    shape.setFillColor(sf::Color::Green);
    sf::Clock deltaClock;
    
    while (renderWindow.isOpen())
    {
        while (const std::optional event = renderWindow.pollEvent())        
        {
            ImGui::SFML::ProcessEvent(renderWindow, *event);

            if (event->is<sf::Event::Closed>()) {
                renderWindow.close();
            }
        }

        
        ImGui::SFML::Update(renderWindow, deltaClock.restart());

        // ---- UI code ----
        ImGui::Begin("My Window");
        ImGui::Text("Hello, world!");

        static float value = 0.0f;
        ImGui::SliderFloat("Slider", &value, 0.0f, 1.0f);

        if (ImGui::Button("Click me"))
            printf("Button pressed\n");

        ImGui::End();
        // -----------------
        
        renderWindow.clear(sf::Color::Blue);

        ImGui::SFML::Render(renderWindow);
        renderWindow.draw(shape);
        
        renderWindow.display();
    }
    
    ImGui::SFML::Shutdown();

    return 0;
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
