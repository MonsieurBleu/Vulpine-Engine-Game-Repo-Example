#pragma once
#include <../Engine/include/App.hpp>
#include <demos/FPS/FPSController.hpp>

class Game final : public App
{
private:
    MeshMaterial depthOnlyMaterial;
    MeshMaterial PBR;
    MeshMaterial skyboxMaterial;
    MeshMaterial depthOnlyStencilMaterial;
    MeshMaterial PBRstencil;

    std::shared_ptr<FPSController> playerControler;
    PhysicsEngine physicsEngine;

public:
    Game(GLFWwindow *window);
    void init(int paramSample);
    bool userInput(GLFWKeyInfo input);
    void mainloop();
};