#include <Game.hpp>
#include <../Engine/include/Globals.hpp>
#include <GameObject.hpp>
#include <CompilingOptions.hpp>

#include <MathsUtils.hpp>
 

void printm4(const mat4& m)
{
    for(int i = 0; i < 4; i++)
    {
    for(int j = 0; j < 4; j++)
        std::cout << m[i][j] << "\t";
    std::cout << "\n";
    }
};



Game::Game(GLFWwindow* window) : App(window){}

void Game::init(int paramSample)
{
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

    /* Loading Materials */
    depthOnlyMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/depthOnly.frag",
            "shader/foward/basic.vert",
            ""
        )
    );

    depthOnlyStencilMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/depthOnlyStencil.frag",
            "shader/foward/basic.vert",
            ""
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

    PBRstencil = MeshMaterial(
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

    PBRstencil.depthOnly = depthOnlyStencilMaterial;
    scene.depthOnlyMaterial = depthOnlyMaterial;   
}

bool Game::userInput(GLFWKeyInfo input)
{
    playerControler->doInputs(input);

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
    /* Loading Models and setting up the scene */
    ModelRef skybox = newModel(skyboxMaterial);
    skybox->loadFromFolder("ressources/models/skybox/", true, false);
    // skybox->invertFaces = true;
    skybox->depthWrite = true;
    skybox->state.frustumCulled = false;
    skybox->state.scaleScalar(1E6);
    scene.add(skybox);

    ModelRef floor = newModel(PBR);
    floor->loadFromFolder("ressources/models/ground/");
    
    int gridSize = 8; 
    int gridScale = 5;
    for(int i = -gridSize; i < gridSize; i++)
    for(int j = -gridSize; j < gridSize; j++)
    {
        ModelRef f = floor->copyWithSharedMesh();
        f->state
            .scaleScalar(gridScale)
            .setPosition(vec3(i*gridScale*1.80, 0, j*gridScale*1.80));
        scene.add(f);
    }

    ModelRef leaves = newModel(PBRstencil);
    leaves->loadFromFolder("ressources/models/fantasy tree/");
    leaves->noBackFaceCulling = true;
    
    ModelRef trunk = newModel(PBR);
    trunk->loadFromFolder("ressources/models/fantasy tree/trunk/");

    // ObjectGroupRef tree = newObjectGroup();
    // tree->add(leaves);
    // tree->add(trunk);
    // tree->state.scaleScalar(0.5);
    // scene.add(tree);


    int forestSize = 8; 
    float treeScale = 0.5;
    for(int i = -forestSize; i < forestSize; i++)
    for(int j = -forestSize; j < forestSize; j++)
    {
        ObjectGroupRef tree = newObjectGroup();
        tree->add(trunk->copyWithSharedMesh());
        tree->add(leaves->copyWithSharedMesh());
        tree->state
            .scaleScalar(treeScale)
            .setPosition(vec3(i*forestSize*2, 0, j*forestSize*2));

        scene.add(tree);
    }


    SceneDirectionalLight sun = newDirectionLight(
        DirectionLight()
            .setColor(vec3(143, 107, 71)/vec3(255))
            .setDirection(normalize(vec3(-0.454528, -0.707103, 0.541673)))
            .setIntensity(1.0)
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
    
    /* Main Loop */
    while(state != quit)
    {
        mainloopStartRoutine();

        for(GLFWKeyInfo input; inputs.pull(input); userInput(input));

        float delta = min(globals.simulationTime.getDelta(), 0.05f);
        if(globals.windowHasFocus() && delta > 0.00001f)
        {
            physicsEngine.update(delta);
            playerControler->update(delta);
            FloorGameObject.update(delta);
        }

        mainloopPreRenderRoutine();

        /* UI & 2D Render */
        glEnable(GL_BLEND); 
        glEnable(GL_FRAMEBUFFER_SRGB);

        scene2D.updateAllObjects();
        screenBuffer2D.activate();
        scene2D.draw();
        screenBuffer2D.deactivate();

        /* 3D Pre-Render */
        glDisable(GL_FRAMEBUFFER_SRGB);
        glDisable(GL_BLEND);
        glDepthFunc(GL_GREATER);
        glEnable(GL_DEPTH_TEST);

        scene.updateAllObjects();
        scene.generateShadowMaps();

        /* 3D Early Depth Testing */
        renderBuffer.activate();
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

        sun->shadowCamera.setPosition(globals.currentCamera->getPosition());
        // printm4(sun->shadowCamera.getProjectionViewMatrix());

        /* Main loop End */
        mainloopEndRoutine();
    }
}
