#pragma once
#include <../Engine/include/App.hpp>

class Game final : public App
{
    private : 
        
    public : 
        Game(GLFWwindow* window);
        void init(int paramSample);
        bool userInput(GLFWKeyInfo input);
        void mainloop();
};