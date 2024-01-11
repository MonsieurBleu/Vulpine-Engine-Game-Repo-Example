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

        case GLFW_KEY_F2 :
            globals.currentCamera->toggleMouseFollow();
            break;
        
        case GLFW_KEY_1 : Bloom.toggle(); break;
        case GLFW_KEY_2 : SSAO.toggle(); break;
        
        case GLFW_KEY_F5 :
            #ifdef _WIN32
            system("cls");
            #else
            system("clear");
            #endif
            finalProcessingStage.reset();
            Bloom.getShader().reset();
            SSAO.getShader().reset();
            depthOnlyMaterial->reset();
            PBR->reset();
            skyboxMaterial->reset();
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
    camera.setPosition(vec3(0, 1, 0));
    camera.setDirection(vec3(1, 0, 0));

    /* Loading Materials */
    depthOnlyMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/depthOnly.frag",
            "shader/foward/basic.vert",
            "", 
            globals.standartShaderUniform3D()
        )
    );

    PBR = MeshMaterial(
        new ShaderProgram(
            "shader/foward/PBR.frag",
            "shader/foward/basic.vert",
            "", 
            globals.standartShaderUniform3D()
        )
    );

    skyboxMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/foward/skybox.frag",
            "shader/foward/basic.vert",
            "", 
            globals.standartShaderUniform3D()
        )
    );

    /* Loading Models */
    ModelRef skybox = newModel(skyboxMaterial);
    skybox->loadFromFolder("ressources/models/skybox/", true, false);
    skybox->invertFaces = true;
    skybox->state.scaleScalar(1E3);

    ModelRef ground = newModel(PBR);
    ground->loadFromFolder("ressources/models/ground/");
    ground->state.scaleScalar(10);

    SceneDirectionalLight sun = newDirectionLight();
    sun->setIntensity(0.75).setColor(vec3(1, 0.9, 0.85)).setDirection(vec3(-1, -1, 0.75));

    scene.add(skybox).add(ground).add(sun);
    scene.depthOnlyMaterial = depthOnlyMaterial;

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
        skybox->bindMap(0, 4);
        scene.draw();   
        renderBuffer.deactivate();

        /* Post Processing */
        renderBuffer.bindTextures();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        SSAO.render(*globals.currentCamera);
        Bloom.render(*globals.currentCamera);

        /* Final Screen Composition */
        glViewport(0, 0, globals.windowWidth(), globals.windowHeight());
        finalProcessingStage.activate();
        screenBuffer2D.bindTexture(0, 7);
        globals.drawFullscreenQuad();
        finalProcessingStage.deactivate();

        /* Main loop End */
        mainloopEndRoutine();
    }
}
