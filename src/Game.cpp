#include <Game.hpp>
#include <../Engine/include/Globals.hpp>
#include <GameObject.hpp>
#include <CompilingOptions.hpp>
#include <MathsUtils.hpp>
#include <Audio.hpp>

#include <thread>

Game::Game(GLFWwindow *window) : App(window) {}

void Game::init(int paramSample)
{
    setIcon("ressources/icon.png");

    ambientLight = vec3(0.1);

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

    /* Loading 3D Materials */
    depthOnlyMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/depthOnly.frag",
            "shader/foward/basic.vert",
            ""));

    depthOnlyStencilMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/depthOnlyStencil.frag",
            "shader/foward/basic.vert",
            ""));

    depthOnlyInstancedMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/depthOnlyStencil.frag",
            "shader/foward/basicInstance.vert",
            ""));

    GameGlobals::PBR = MeshMaterial(
        new ShaderProgram(
            "shader/foward/PBR.frag",
            "shader/foward/basic.vert",
            "",
            globals.standartShaderUniform3D()));

    GameGlobals::PBRstencil = MeshMaterial(
        new ShaderProgram(
            "shader/foward/PBR.frag",
            "shader/foward/basic.vert",
            "",
            globals.standartShaderUniform3D()));

    GameGlobals::PBRinstanced = MeshMaterial(
        new ShaderProgram(
            "shader/foward/PBR.frag",
            "shader/foward/basicInstance.vert",
            "",
            globals.standartShaderUniform3D()));

    skyboxMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/foward/Skybox.frag",
            "shader/foward/basic.vert",
            "",
            globals.standartShaderUniform3D()));

    GameGlobals::PBRstencil.depthOnly = depthOnlyStencilMaterial;
    GameGlobals::PBRinstanced.depthOnly = depthOnlyInstancedMaterial;
    scene.depthOnlyMaterial = depthOnlyMaterial;

    /* UI */
    FUIfont = FontRef(new FontUFT8);
    FUIfont->readCSV("ressources/fonts/Roboto/out.csv");
    FUIfont->setAtlas(Texture2D().loadFromFileKTX("ressources/fonts/Roboto/out.ktx"));
    defaultFontMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/2D/sprite.frag",
            "shader/2D/sprite.vert",
            "",
            globals.standartShaderUniform2D()));

    defaultSUIMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/2D/fastui.frag",
            "shader/2D/fastui.vert",
            "",
            globals.standartShaderUniform2D()));

    fuiBatch = SimpleUiTileBatchRef(new SimpleUiTileBatch);
    fuiBatch->setMaterial(defaultSUIMaterial);
    fuiBatch->state.position.z = 0.0;
    fuiBatch->state.forceUpdate();

    /* VSYNC and fps limit */
    globals.fpsLimiter.activate();
    globals.fpsLimiter.freq = 144.f;
    glfwSwapInterval(0);

    handItems = std::make_shared<HandItemHandler>();
}

bool Game::userInput(GLFWKeyInfo input)
{
    if (baseInput(input))
        return true;

    playerControler->doInputs(input);

    handItems->inputs(input);

    if (input.action == GLFW_PRESS)
    {
        switch (input.key)
        {
        case GLFW_KEY_ESCAPE:
            state = quit;
            break;

        case GLFW_KEY_F2:
            globals.currentCamera->toggleMouseFollow();
            break;

        case GLFW_KEY_1:
            Bloom.toggle();
            break;
        case GLFW_KEY_2:
            SSAO.toggle();
            break;

        case GLFW_KEY_F5:
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif
            finalProcessingStage.reset();
            Bloom.getShader().reset();
            SSAO.getShader().reset();
            depthOnlyMaterial->reset();
            GameGlobals::PBR->reset();
            GameGlobals::PBRstencil->reset();
            skyboxMaterial->reset();
            break;

        default:
            break;
        }
    }

    return true;
};

void Game::physicsLoop()
{
    physicsTicks.freq = 45.f;
    physicsTicks.activate();

    while (state != quit)
    {
        physicsTicks.start();

        physicsMutex.lock();
        physicsEngine.update(1.f / physicsTicks.freq);
        physicsMutex.unlock();

        physicsTicks.waitForEnd();
    }
}

void Game::mainloop()
{
    /* Loading Models and setting up the scene */
    ModelRef skybox = newModel(skyboxMaterial);
    skybox->loadFromFolder("ressources/models/skybox/", true, false);

    // skybox->invertFaces = true;
    skybox->depthWrite = true;
    skybox->state.frustumCulled = false;
    skybox->state.scaleScalar(1E6);
    scene.add(skybox);

    ModelRef floor = newModel(GameGlobals::PBR);
    floor->loadFromFolder("ressources/models/ground/");

    int gridSize = 10;
    int gridScale = 10;
    for (int i = -gridSize; i < gridSize; i++)
        for (int j = -gridSize; j < gridSize; j++)
        {
            ModelRef f = floor->copyWithSharedMesh();
            f->state
                .scaleScalar(gridScale)
                .setPosition(vec3(i * gridScale * 1.80, 0, j * gridScale * 1.80));
            scene.add(f);
        }

    int forestSize = 8;
    float treeScale = 0.5;

    ModelRef leaves = newModel(GameGlobals::PBRstencil);
    leaves->loadFromFolder("ressources/models/fantasy tree/");
    leaves->noBackFaceCulling = true;

    ModelRef trunk = newModel(GameGlobals::PBR);
    trunk->loadFromFolder("ressources/models/fantasy tree/trunk/");

    for (int i = -forestSize; i < forestSize; i++)
        for (int j = -forestSize; j < forestSize; j++)
        {
            ObjectGroupRef tree = newObjectGroup();
            tree->add(trunk->copyWithSharedMesh());
            ModelRef l = leaves->copyWithSharedMesh();
            l->noBackFaceCulling = true;
            tree->add(l);
            tree->state
                .scaleScalar(treeScale)
                .setPosition(vec3(i * treeScale * 40, 0, j * treeScale * 40));

            scene.add(tree);
        }

    /* Instanced Mesh example */
    // InstancedModelRef trunk = newInstancedModel();
    // trunk->setMaterial(PBRinstanced);
    // trunk->loadFromFolder("ressources/models/fantasy tree/trunk/");
    // trunk->allocate(2E4);

    // for(int i = -forestSize; i < forestSize; i++)
    // for(int j = -forestSize; j < forestSize; j++)
    // {
    //     ModelInstance &inst = *trunk->createInstance();
    //     inst.scaleScalar(treeScale)
    //         .setPosition(vec3(i*treeScale*40, 0, j*treeScale*40));
    //     inst.update();
    // }
    // trunk->updateInstances();
    // scene.add(trunk);

    SceneDirectionalLight sun = newDirectionLight(
        DirectionLight()
            .setColor(vec3(143, 107, 71) / vec3(255))
            .setDirection(normalize(vec3(-0.454528, -0.707103, 0.541673)))
            .setIntensity(5.0)
            );

    sun->cameraResolution = vec2(2048);
    sun->shadowCameraSize = vec2(90, 90);
    sun->activateShadows();
    scene.add(sun);

    /* FPS demo initialization */
    RigidBody::gravity = vec3(0.0, -80, 0.0);

    AABBCollider aabbCollider = AABBCollider(vec3(-32 * 5, -.1, -32 * 5), vec3(32 * 5, .1, 32 * 5));

    RigidBodyRef FloorBody = newRigidBody(
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, 0.0, 0.0),
        quat(0.0, 0.0, 0.0, 1.0),
        vec3(0.0, 0.0, 0.0),
        &aabbCollider,
        PhysicsMaterial(),
        0.0,
        false);

    physicsEngine.addObject(FloorBody);

    GameObject FloorGameObject(newObjectGroup(), FloorBody);
    FloorGameObject.getGroup()->add(floor);

    SphereCollider playerCollider = SphereCollider(2.0);
    RigidBodyRef playerBody = newRigidBody(
        vec3(0.0, 8.0, 0.0),
        vec3(0.0, 0.0, 0.0),
        quat(0.0, 0.0, 0.0, 1.0),
        vec3(0.0, 0.0, 0.0),
        &playerCollider,
        PhysicsMaterial(0.0f, 0.0f, 0.0f, 0.0f),
        1.0,
        true);

    physicsEngine.addObject(playerBody);

    playerControler =
        std::make_shared<FPSController>(window, playerBody, &camera, &inputs);
    FPSVariables::thingsYouCanStandOn.push_back(FloorBody);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glLineWidth(3.0);

    /* Setting up the UI */
    FastUI_context ui(fuiBatch, FUIfont, scene2D, defaultFontMaterial);
    FastUI_valueMenu menu(ui, {});

    menu->state.setPosition(vec3(-0.9, 0.5, 0)).scaleScalar(0.8); 
    globals.appTime.setMenuConst(menu);
    globals.cpuTime.setMenu(menu);
    globals.gpuTime.setMenu(menu);
    globals.fpsLimiter.setMenu(menu);
    physicsTicks.setMenu(menu);
    sun->setMenu(menu, U"Sun");

    menu.batch();
    scene2D.updateAllObjects();
    fuiBatch->batch();

    state = AppState::run;
    std::thread physicsThreads(&Game::physicsLoop, this);

    /* Music ! */
    AudioFile music1;
    music1.loadOGG("ressources/musics/Endless Space by GeorgeTantchev.ogg");

    AudioSource musicSource;
    musicSource
        .setBuffer(music1.getHandle())
        .setPosition(vec3(0, 0, 3))
        .play();

    // alSourcei(musicSource.getHandle(), AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(musicSource.getHandle(), AL_DIRECTION, 0.0, 0.0, 0.0);


    ModelRef lanterne = newModel(GameGlobals::PBR);
    lanterne->loadFromFolder("ressources/models/lantern/");
    lanterne->state
        .scaleScalar(0.01)
        .setPosition(vec3(2, 2, 0));
    scene.add(lanterne);

    ModelRef werewolf = newModel(GameGlobals::PBRstencil);
        werewolf->loadFromFolder("ressources/models/werewolf/",false,false);
        werewolf->state
            .scaleScalar(100)
            .setPosition(vec3(10, 0, 0));
        scene.add(werewolf);
    
    handItems->addItem(HandItemRef(new HandItem(HandItemType::lantern)));
    scene.add(handItems);

    /* Main Loop */
    while (state != AppState::quit)
    {
        mainloopStartRoutine();

        for (GLFWKeyInfo input; inputs.pull(input); userInput(input));

        float delta = min(globals.simulationTime.getDelta(), 0.05f);
        if (globals.windowHasFocus() && delta > 0.00001f)
        {
            // physicsEngine.update(delta);
            playerControler->update(delta);
            FloorGameObject.update(delta);
        }

        // float c = 0.5 + 0.5*cos(globals.appTime.getElapsedTime());
        // musicSource.setPitch(0.1 + c*2);

        menu.trackCursor();
        menu.updateText();

        mainloopPreRenderRoutine();

        /* UI & 2D Render */
        glEnable(GL_BLEND);
        glEnable(GL_FRAMEBUFFER_SRGB);

        scene2D.updateAllObjects();
        fuiBatch->batch();
        screenBuffer2D.activate();
        fuiBatch->draw();
        scene2D.cull();
        scene2D.draw();
        screenBuffer2D.deactivate();

        /* 3D Pre-Render */
        glDisable(GL_FRAMEBUFFER_SRGB);
        glDisable(GL_BLEND);
        glDepthFunc(GL_GREATER);
        glEnable(GL_DEPTH_TEST);

        scene.updateAllObjects();
        scene.generateShadowMaps();
        renderBuffer.activate();

        scene.cull();

        /* 3D Early Depth Testing */
        scene.depthOnlyDraw(*globals.currentCamera, true);
        glDepthFunc(GL_EQUAL);

        /* 3D Render */
        skybox->bindMap(0, 4);
        scene.genLightBuffer();
        scene.draw();
        renderBuffer.deactivate();

        /* Post Processing */
        renderBuffer.bindTextures();
        SSAO.render(*globals.currentCamera);
        Bloom.render(*globals.currentCamera);

        /* Final Screen Composition */
        glViewport(0, 0, globals.windowWidth(), globals.windowHeight());
        finalProcessingStage.activate();
        sun->shadowMap.bindTexture(0, 6);
        screenBuffer2D.bindTexture(0, 7);
        globals.drawFullscreenQuad();

        /* Main loop End */
        mainloopEndRoutine();
    }

    physicsThreads.join();
}
