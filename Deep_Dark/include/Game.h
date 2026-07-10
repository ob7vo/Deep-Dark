#pragma once
#include "SaveSystem.h"
#include "StateManager.h"
#include "PreparationState.h"
#include "StageGameState.h"
#include "Camera.h"

class Game {
public:
    static Game game;
    static Game& Get() {return game;};

    Game();

    void Start();
    void Initialize();

private:
    void Run();
    void PollEvents();

    void SetMousePosition();
    void SetFPSText(float deltaTime, float& time);

    sf::RenderWindow renderWindow;
    Camera camera;
    sf::Text fpsText = sf::Text(baseFont);

    StateManager stateManager;
};