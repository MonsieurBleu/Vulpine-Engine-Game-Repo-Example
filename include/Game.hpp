#pragma once
#include <../Engine/include/App.hpp>

class Game final : public App
{
    private : 

        MeshMaterial depthOnlyMaterial;
        MeshMaterial PBR;
        MeshMaterial skyboxMaterial;

        
    public : 
        Game(GLFWwindow* window);
        void init(int paramSample);
        bool userInput(GLFWKeyInfo input);
        void mainloop();
};