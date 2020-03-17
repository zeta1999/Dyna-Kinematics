#include <iostream>

#include "shader_loader.h"
#include "texture_loader.h"
#include "model_loader.h"
#include "menu_state.h"
#include "play_state.h"
#include "pause_state.h"
#include "game.h"

Game::Game()
   : mFSM()
   , mWindow()
   //, mSoundEngine(irrklang::createIrrKlangDevice(), [=](irrklang::ISoundEngine* soundEngine){soundEngine->drop();})
   , mCamera()
   , mRenderer2D()
   , mModelManager()
   , mTextureManager()
   , mShaderManager()
{

}

Game::~Game()
{

}

bool Game::initialize(const std::string& title)
{
   // Initialize the window
   mWindow = std::make_shared<Window>(title);

   if (!mWindow->initialize())
   {
      std::cout << "Error - Game::initialize - Failed to initialize the window" << "\n";
      return false;
   }

   // Initialize the camera
   float widthInPix = 1280.0f;
   float heightInPix = 720.0f;
   float aspectRatio = (widthInPix / heightInPix);

   mCamera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 95.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f),
                                      0.0f,
                                      0.0f,
                                      45.0f,       // Fovy
                                      aspectRatio, // Aspect ratio
                                      0.1f,        // Near
                                      130.0f,      // Far
                                      20.0f,       // Movement speed
                                      0.1f);       // Mouse sensitivity

   // Initialize the 2D renderer
   //glm::mat4 orthoProj = glm::ortho(0.0f,        // Left
   //                                 widthInPix,  // Right
   //                                 heightInPix, // Bottom
   //                                 0.0f,        // Top
   //                                -1.0f,        // Near
   //                                 1.0f);       // Far
   glm::mat4 orthoProj = glm::ortho(-widthInPix / 2,  // Left
                                     widthInPix / 2,  // Right
                                    -heightInPix / 2, // Bottom
                                     heightInPix / 2, // Top
                                    -1.0f,            // Near
                                     1.0f);           // Far

   auto texture2DShader = mShaderManager.loadResource<ShaderLoader>("texture_2D",
                                                                    "resources/shaders/texture_2D.vs",
                                                                    "resources/shaders/texture_2D.fs");
   texture2DShader->use();
   texture2DShader->setInt("image", 0);
   texture2DShader->setMat4("projection", orthoProj);

   auto color2DShader = mShaderManager.loadResource<ShaderLoader>("color_2D",
                                                                  "resources/shaders/color_2D.vs",
                                                                  "resources/shaders/color_2D.fs");
   color2DShader->use();
   color2DShader->setMat4("projection", orthoProj);

   mRenderer2D = std::make_unique<Renderer2D>(texture2DShader, color2DShader);

   // Create the FSM
   mFSM = std::make_shared<FiniteStateMachine>();

   // Initialize the states
   std::unordered_map<std::string, std::shared_ptr<State>> mStates;

   mStates["menu"] = std::make_shared<MenuState>(mFSM,
                                                 mWindow,
                                                 mCamera,
                                                 mRenderer2D);

   mStates["play"] = std::make_shared<PlayState>(mFSM,
                                                 mWindow,
                                                 mSoundEngine,
                                                 mCamera,
                                                 mRenderer2D);

   mStates["pause"] = std::make_shared<PauseState>(mFSM,
                                                   mWindow,
                                                   mCamera,
                                                   mRenderer2D);

   // Initialize the FSM
   mFSM->initialize(std::move(mStates), "menu");

   return true;
}

void Game::executeGameLoop()
{
   double currentFrame = 0.0;
   double lastFrame    = 0.0;
   float  deltaTime    = 0.0f;

   while (!mWindow->shouldClose())
   {
      currentFrame = glfwGetTime();
      deltaTime    = static_cast<float>(currentFrame - lastFrame);
      lastFrame    = currentFrame;

      mFSM->processInputInCurrentState(deltaTime);
      mFSM->updateCurrentState(deltaTime);
      mFSM->renderCurrentState();
   }
}
