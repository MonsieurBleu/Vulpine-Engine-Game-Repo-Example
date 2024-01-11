#include <Game.hpp>
#include <../Engine/include/Globals.hpp>

Game::Game(GLFWwindow* window) : App(window){}

void Game::init(int paramSample)
{
    
}

bool Game::userInput(GLFWKeyInfo input)
{
    if(input.action == GLFW_PRESS)
    {
        switch (input.key)
        {
        case GLFW_KEY_ESCAPE:
            state = quit;
            break;
        
        default:
            break;
        }

    }
    return true;
};

void Game::mainloop()
{
    finalProcessingStage = ShaderProgram(
        "shader/post-process/final composing.frag", 
        "shader/post-process/basic.vert", 
        "", 
        globals.standartShaderUniform2D());
    
    finalProcessingStage.addUniform(ShaderUniform(Bloom.getIsEnableAddr(), 10));

    camera.init(radians(70.0f), globals.windowWidth(), globals.windowHeight(), 0.1f, 1E4f);
    camera.setMouseFollow(false);

    while(state != quit)
    {
        mainloopStartRoutine();

        GLFWKeyInfo input;
        while(inputs.pull(input))
            userInput(input);

        mainloopPreRenderRoutine();

        /* UI & 2D Render */
        glEnable(GL_BLEND); 
        glEnable(GL_FRAMEBUFFER_SRGB);

        scene2D.updateAllObjects();
        screenBuffer2D.activate();
        scene2D.draw();

        /* 3D Pre-Render */
        glDisable(GL_FRAMEBUFFER_SRGB);
        glDisable(GL_BLEND);
        glDepthFunc(GL_GREATER);

        scene.updateAllObjects();
        scene.genLightBuffer();
        scene.generateShadowMaps();
        renderBuffer.activate();

        /* 3D Early Depth Testing */
        scene.depthOnlyDraw(*globals.currentCamera, true);
        glDepthFunc(GL_EQUAL);

        /* 3D Render */
        scene.draw();   
        renderBuffer.deactivate();

        /* Post Processing */
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        renderBuffer.bindTextures();
        SSAO.render(camera);
        Bloom.render(camera);

        /* Final Screen Composition */
        glViewport(0, 0, globals.windowWidth(), globals.windowHeight());
        finalProcessingStage.activate();
        globals.drawFullscreenQuad();
        finalProcessingStage.deactivate();

        /* Main loop End */
        mainloopEndRoutine();
    }
}
