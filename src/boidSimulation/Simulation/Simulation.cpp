#include <boidSimulation/Simulation.h>

#include <exception>
#include <chrono>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdlib>

namespace
{
constexpr float kControlPanelPreferredWidth = 460.0f;

int parseEnvInt(const char* name, const int fallback)
{
   const char* value = std::getenv(name);
   if (value == nullptr || *value == '\0')
      return fallback;

   try
   {
      return std::stoi(value);
   }
   catch (...)
   {
      return fallback;
   }
}

float getControlPanelWidth(const float windowWidth)
{
   const float minSimulationWidth = 180.0f;
   float maxPanelWidth = windowWidth - minSimulationWidth;

   if (maxPanelWidth < 140.0f)
      maxPanelWidth = windowWidth * 0.45f;

   return std::max(140.0f, std::min(kControlPanelPreferredWidth, maxPanelWidth));
}

void applyCyberImGuiTheme()
{
   ImGuiStyle& style = ImGui::GetStyle();
   ImVec4* colors = style.Colors;

   style.WindowPadding = ImVec2(12.0f, 10.0f);
   style.FramePadding = ImVec2(8.0f, 4.0f);
   style.ItemSpacing = ImVec2(8.0f, 6.0f);
   style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
   style.WindowRounding = 0.0f;
   style.ChildRounding = 0.0f;
   style.FrameRounding = 0.0f;
   style.PopupRounding = 0.0f;
   style.GrabRounding = 0.0f;
   style.ScrollbarRounding = 0.0f;
   style.TabRounding = 0.0f;
   style.WindowBorderSize = 1.0f;
   style.ChildBorderSize = 1.0f;
   style.PopupBorderSize = 1.0f;
   style.FrameBorderSize = 1.0f;
   style.TabBorderSize = 1.0f;

   colors[ImGuiCol_Text] = ImVec4(0.36f, 1.0f, 0.54f, 1.0f);
   colors[ImGuiCol_TextDisabled] = ImVec4(0.19f, 0.58f, 0.30f, 1.0f);
   colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.02f, 0.00f, 0.98f);
   colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.03f, 0.00f, 0.90f);
   colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.03f, 0.00f, 0.98f);
   colors[ImGuiCol_Border] = ImVec4(0.22f, 0.92f, 0.45f, 0.95f);
   colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
   colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.06f, 0.00f, 0.92f);
   colors[ImGuiCol_FrameBgHovered] = ImVec4(0.03f, 0.18f, 0.03f, 0.94f);
   colors[ImGuiCol_FrameBgActive] = ImVec4(0.05f, 0.30f, 0.07f, 0.98f);
   colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.07f, 0.00f, 0.95f);
   colors[ImGuiCol_TitleBgActive] = ImVec4(0.02f, 0.14f, 0.03f, 0.98f);
   colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.05f, 0.00f, 0.95f);
   colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.03f, 0.00f, 0.75f);
   colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.08f, 0.42f, 0.15f, 0.90f);
   colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.14f, 0.64f, 0.24f, 0.95f);
   colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.22f, 0.88f, 0.38f, 1.0f);
   colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 1.0f, 0.50f, 1.0f);
   colors[ImGuiCol_SliderGrab] = ImVec4(0.22f, 0.88f, 0.38f, 0.95f);
   colors[ImGuiCol_SliderGrabActive] = ImVec4(0.34f, 1.0f, 0.52f, 1.0f);
   colors[ImGuiCol_Button] = ImVec4(0.00f, 0.10f, 0.01f, 0.95f);
   colors[ImGuiCol_ButtonHovered] = ImVec4(0.04f, 0.26f, 0.06f, 0.98f);
   colors[ImGuiCol_ButtonActive] = ImVec4(0.10f, 0.44f, 0.14f, 1.0f);
   colors[ImGuiCol_Header] = ImVec4(0.00f, 0.12f, 0.02f, 0.90f);
   colors[ImGuiCol_HeaderHovered] = ImVec4(0.05f, 0.28f, 0.07f, 0.95f);
   colors[ImGuiCol_HeaderActive] = ImVec4(0.10f, 0.46f, 0.16f, 1.0f);
   colors[ImGuiCol_Separator] = ImVec4(0.22f, 0.92f, 0.42f, 0.90f);
   colors[ImGuiCol_SeparatorHovered] = ImVec4(0.30f, 1.0f, 0.50f, 1.0f);
   colors[ImGuiCol_SeparatorActive] = ImVec4(0.34f, 1.0f, 0.54f, 1.0f);
   colors[ImGuiCol_ResizeGrip] = ImVec4(0.16f, 0.62f, 0.24f, 0.50f);
   colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.92f, 0.40f, 0.90f);
   colors[ImGuiCol_ResizeGripActive] = ImVec4(0.34f, 1.0f, 0.52f, 1.0f);
   colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.10f, 0.01f, 0.92f);
   colors[ImGuiCol_TabHovered] = ImVec4(0.05f, 0.28f, 0.07f, 0.97f);
   colors[ImGuiCol_TabActive] = ImVec4(0.10f, 0.46f, 0.16f, 1.0f);
   colors[ImGuiCol_PlotLines] = ImVec4(0.30f, 1.0f, 0.50f, 1.0f);
   colors[ImGuiCol_PlotHistogram] = ImVec4(0.30f, 1.0f, 0.50f, 1.0f);
}
}

Simulation::Simulation()
{
   m_window = nullptr;
   m_renderer = nullptr;
   m_glContext = nullptr;
   m_appRunning = false;
   m_modeChangedByGui = false;
   m_lastUpdateMs = 0.0f;
   m_lastCommMs = 0.0f;
   m_lastRenderMs = 0.0f;
   m_lastFrameMs = 0.0f;
   m_serialBaselineMs = 0.0f;
   m_avgUpdateMs = 0.0f;
   m_avgCommMs = 0.0f;
   m_avgRenderMs = 0.0f;
   m_avgFrameMs = 0.0f;
   m_frameSampleCount = 0;
   m_serialBaselineFrames = 0;
   m_totalBoids = config::N_SPECIES * config::N_BOIDS;
   m_mpiRank = 0;
   m_mpiWorldSize = 1;
   m_runtimeMpiRootRank = 0;
   m_targetMpiProcs = 4;
   m_targetMpiRank = 0;
   m_localSpeciesStart = 0;
   m_localSpeciesEnd = config::N_SPECIES;
   m_hasMpiRuntime = false;
   m_isRenderProcess = true;
   m_isFullscreen = false;

#ifdef ENABLE_MPI
   int isInitialized = 0;
   MPI_Initialized(&isInitialized);
   if (!isInitialized)
      MPI_Init(nullptr, nullptr);

   m_hasMpiRuntime = true;
   MPI_Comm_rank(MPI_COMM_WORLD, &m_mpiRank);
   MPI_Comm_size(MPI_COMM_WORLD, &m_mpiWorldSize);
   m_runtimeMpiRootRank = std::clamp(parseEnvInt("MPI_RENDER_RANK", 0), 0, std::max(0, m_mpiWorldSize - 1));
   m_targetMpiProcs = m_mpiWorldSize;
   m_targetMpiRank = m_runtimeMpiRootRank;
   m_isRenderProcess = (m_mpiRank == m_runtimeMpiRootRank);
#endif

   computeLocalSpeciesRange();
   setupExecutionMode();
   setupMPIGatherLayout();
   
   try 
   {
      if (m_isRenderProcess)
      {
         // Init Window
         m_window = WindowManager::getInstance(
               config::RESOLUTION_W,
               config::RESOLUTION_H,
               config::WINDOW_TITLE
         );

         // Init Renderer
         m_renderer = Renderer::getInstance(m_window->getWindow());

         m_glContext = SDL_GL_CreateContext(m_window->getWindow());
         if (!m_glContext)
            throw std::runtime_error(std::string("Failed to create OpenGL context. Error: ") + SDL_GetError());

         initGLAD();

         initImGui();
      }

   } catch (const std::exception& e)
   {
      std::cout << e.what() << std::endl;
      m_appRunning = false;

      return;
   }

   createFlocks();
   createObstacles();

   if (m_isRenderProcess)
   {
      initShaders();
      initBuffers();
   }
   
   m_appRunning = true;
   m_FPS = 0;
}

Simulation::~Simulation()
{
   if (m_isRenderProcess && ImGui::GetCurrentContext() != nullptr)
   {
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplSDL2_Shutdown();
      ImGui::DestroyContext();
   }

   if (m_isRenderProcess && m_glContext != nullptr)
      SDL_GL_DeleteContext(m_glContext);

   if (m_isRenderProcess && m_renderer != nullptr)
      m_renderer->destroyRenderer();

   if (m_isRenderProcess && m_window != nullptr)
      m_window->destroyWindow();

#ifdef ENABLE_MPI
   if (m_hasMpiRuntime)
   {
      int isFinalized = 0;
      MPI_Finalized(&isFinalized);
      if (!isFinalized)
         MPI_Finalize();
   }
#endif
}

// Creates n species of boids.
void Simulation::createFlocks()
{
   m_flocks.reserve(config::MAX_N_SPECIES);
  
   for (int i = 0; i < config::MAX_N_SPECIES; i++)
      m_flocks.push_back(Flock());
}

void Simulation::createObstacles()
{
   m_obstacles.clear();
   m_obstacles.reserve(config::MAX_N_OBSTACLES);
}

void Simulation::addObstacleAtClick(const glm::vec2& clickPos)
{
   if ((int)m_obstacles.size() >= config::MAX_N_OBSTACLES)
      return;

   const unsigned newId = static_cast<unsigned>(m_obstacles.size());
   Obstacle newObstacle(newId, clickPos);

   std::uniform_real_distribution<float> jitter(-0.12f, 0.12f);
   int attempts = 0;

   while (newObstacle.doesCollide(m_obstacles) && attempts < 24)
   {
      glm::vec2 adjustedPos = clickPos + glm::vec2(jitter(config::gen), jitter(config::gen));
      adjustedPos.x = std::clamp(adjustedPos.x, -0.95f, 0.95f);
      adjustedPos.y = std::clamp(adjustedPos.y, -0.95f, 0.95f);
      newObstacle = Obstacle(newId, adjustedPos);
      attempts++;
   }

   if (newObstacle.doesCollide(m_obstacles))
      return;

   m_obstacles.push_back(newObstacle);

   if (m_isRenderProcess)
      fillObstacleChunk();
}

std::string Simulation::buildMpiRelaunchCommand() const
{
   return "RUN_WITH_MPI=1 MPI_PROCS=" + std::to_string(m_targetMpiProcs) +
      " MPI_RENDER_RANK=" + std::to_string(m_targetMpiRank) +
      " bash ./run.sh";
}

void Simulation::calculateFPS()
{
   static int fps = 0;
   static float lastTime = 0.0f;

   float currentTime = SDL_GetTicks() * 0.001f;
   fps++;

   if (currentTime - lastTime > 1.0f)
   {
      m_FPS = fps;
      lastTime = currentTime;
      fps = 0;
   }
}

void Simulation::run()
{
   while (m_appRunning) {
      auto frameStart = std::chrono::steady_clock::now();

      update();
      if (m_isRenderProcess)
         render();

      if (m_isRenderProcess)
         calculateFPS();

      syncRunState();

      auto frameEnd = std::chrono::steady_clock::now();
      m_lastFrameMs = std::chrono::duration<float, std::milli>(frameEnd - frameStart).count();
      
      recordMetrics();
   }
}


void Simulation::update()
{
   m_totalBoids = config::N_SPECIES * config::N_BOIDS;

   if (m_isRenderProcess)
      checkUserInput();

   syncExecutionMode();
   syncGuiScalars();

   if (m_isRenderProcess)
      updateGUI();

   syncExecutionMode();
   syncGuiScalars();

   if (m_executionMode == ExecutionMode::MPI)
      syncObstacles();

   auto updateStart = std::chrono::steady_clock::now();

   updateFlocks();

   auto updateEnd = std::chrono::steady_clock::now();
   m_lastUpdateMs = std::chrono::duration<float, std::milli>(updateEnd - updateStart).count();
   
   if (m_executionMode == ExecutionMode::MPI)
   {
      auto commStart = std::chrono::steady_clock::now();
      syncMPIBoidVertices();
      auto commEnd = std::chrono::steady_clock::now();
      m_lastCommMs = std::chrono::duration<float, std::milli>(commEnd - commStart).count();
   }

   if (m_isRenderProcess)
      updateBoidChunk();
}

void Simulation::updateFlocks()
{
   int start = 0;
   int end = config::N_SPECIES;

   if (m_executionMode == ExecutionMode::MPI)
   {
      if (m_isRenderProcess)
      {
         // Keep full local state warm on render rank for seamless mode switches.
         start = 0;
         end = config::N_SPECIES;
      }
      else
      {
         start = m_localSpeciesStart;
         end = m_localSpeciesEnd;
      }
   }

#ifdef ENABLE_OPENMP
   if (m_executionMode == ExecutionMode::OpenMP)
   {
#pragma omp parallel for schedule(static)
      for (int i = start; i < end; i++)
         m_flocks[i].toFlock(m_obstacles, config::N_BOIDS);
      return;
   }
#endif

   for (int i = start; i < end; i++)
      m_flocks[i].toFlock(m_obstacles, config::N_BOIDS);
}

void Simulation::render()
{
   auto renderStart = std::chrono::steady_clock::now();

   int windowWidth = 0;
   int windowHeight = 0;
   SDL_GetWindowSize(m_window->getWindow(), &windowWidth, &windowHeight);

   const float panelWidthF = getControlPanelWidth(static_cast<float>(windowWidth));
   const int panelWidth = std::clamp(static_cast<int>(panelWidthF), 0, std::max(0, windowWidth - 1));
   const int simWidth = std::max(1, windowWidth - panelWidth);
   const int simHeight = std::max(1, windowHeight);

   // Renders the frame logic
   glClearColor(0.0f, 0.03f, 0.02f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);

   // Keep simulation constrained to the right pane.
   glViewport(panelWidth, 0, simWidth, simHeight);

   
   // Renders the triangles
   glUseProgram(m_shaderProgram);
   glBindVertexArray(m_VAO);

   const int boidVertexCount = 3 * config::N_BOIDS * config::N_SPECIES;
   const int obstacleStartVertex = 3 * config::MAX_N_BOIDS * config::MAX_N_SPECIES;
   const int obstacleVertexCount = 3 * static_cast<int>(m_obstacles.size()) * config::N_TRIANG_PER_CIRCLE;

   // Draw only the active boids.
   glDrawArrays(GL_TRIANGLES, 0, boidVertexCount);

   // Obstacles are rendered in screen-space overlay for perfect circles.
   (void)obstacleStartVertex;
   (void)obstacleVertexCount;

   // Restore full viewport for ImGui composite pass.
   glViewport(0, 0, std::max(1, windowWidth), std::max(1, windowHeight));

   // Renders imgui
   // (we've to render it after glDraw to appear on top of the _window)
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

   m_window->swapBuffers();

   auto renderEnd = std::chrono::steady_clock::now();
   m_lastRenderMs = std::chrono::duration<float, std::milli>(renderEnd - renderStart).count();
}

void Simulation::checkUserInput()
{
   SDL_Event event;

   while (SDL_PollEvent(&event))
   {
      ImGui_ImplSDL2_ProcessEvent(&event);

      if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
      {
         if (event.key.keysym.sym == SDLK_ESCAPE)
         {
            m_appRunning = false;
            break;
         }

         if (event.key.keysym.sym == SDLK_F11)
         {
            const Uint32 targetFlags = m_isFullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP;
            if (SDL_SetWindowFullscreen(m_window->getWindow(), targetFlags) == 0)
               m_isFullscreen = !m_isFullscreen;
         }
      }

      if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
      {
         if (!ImGui::GetIO().WantCaptureMouse)
         {
            int windowWidth = 0;
            int windowHeight = 0;
            SDL_GetWindowSize(m_window->getWindow(), &windowWidth, &windowHeight);

            const float panelWidthF = getControlPanelWidth(static_cast<float>(windowWidth));
            const int panelWidth = std::clamp(static_cast<int>(panelWidthF), 0, std::max(0, windowWidth - 1));

            if (event.button.x >= panelWidth)
            {
               const float simWidth = std::max(1.0f, static_cast<float>(windowWidth - panelWidth));
               const float simHeight = std::max(1.0f, static_cast<float>(windowHeight));

               const float localX = (static_cast<float>(event.button.x) - static_cast<float>(panelWidth)) / simWidth;
               const float localY = static_cast<float>(event.button.y) / simHeight;

               glm::vec2 worldPos(
                  std::clamp((localX * 2.0f) - 1.0f, -0.98f, 0.98f),
                  std::clamp(1.0f - (localY * 2.0f), -0.98f, 0.98f)
               );

               addObstacleAtClick(worldPos);
            }
         }
      }

      if (event.type == SDL_QUIT ||
         (event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_CLOSE &&
             event.window.windowID == SDL_GetWindowID(m_window->getWindow())
         )
      ){
         m_appRunning = false;
         break;
      }
   }
}

// ========================== OpenGL functions ==========================

// Initializes GLAD and load all the GL function pointers.
void Simulation::initGLAD()
{
   if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress))
      throw std::runtime_error("Failed to initialize GLAD.");
}

// Updates the chunk of the buffer that contains the boids
void Simulation::updateBoidChunk()
{
   if (m_executionMode == ExecutionMode::MPI && m_isRenderProcess)
   {
      uploadBoidVerticesFromBuffer(m_globalBoidVertices);
      return;
   }

   for (int i = 0; i < config::N_SPECIES; i++)
   {
      for (int j = 0; j < config::N_BOIDS; j++)
      {
         unsigned int vertexOffset = (
               (6 * j) +
               (i * config::MAX_N_BOIDS * 6)
         ) * sizeof(float);

         updateChunkOfBuffer(
               m_bufferVertex, 
               vertexOffset,
               6,
               m_flocks[i].getBoidPosition(j)
         );
      }
   }
}

void Simulation::setupExecutionMode()
{
   if (canUseMPI())
      m_executionMode = ExecutionMode::MPI;
   else if (canUseOpenMP())
      m_executionMode = ExecutionMode::OpenMP;
   else
      m_executionMode = ExecutionMode::Serial;
}

bool Simulation::canUseOpenMP() const
{
#ifdef ENABLE_OPENMP
   return true;
#else
   return false;
#endif
}

bool Simulation::canUseMPI() const
{
   return m_hasMpiRuntime;
}

int Simulation::getOpenMPThreads() const
{
#ifdef ENABLE_OPENMP
   return omp_get_max_threads();
#else
   return 1;
#endif
}

void Simulation::syncRunState()
{
#ifdef ENABLE_MPI
   if (m_hasMpiRuntime)
   {
      int run = m_appRunning ? 1 : 0;
      MPI_Bcast(&run, 1, MPI_INT, m_runtimeMpiRootRank, MPI_COMM_WORLD);
      m_appRunning = (run != 0);
   }
#endif
}

void Simulation::syncExecutionMode()
{
#ifdef ENABLE_MPI
   if (m_hasMpiRuntime)
   {
      int mode = static_cast<int>(m_executionMode);
      MPI_Bcast(&mode, 1, MPI_INT, m_runtimeMpiRootRank, MPI_COMM_WORLD);
      m_executionMode = static_cast<ExecutionMode>(mode);
   }
#endif

   if (m_executionMode == ExecutionMode::OpenMP && !canUseOpenMP())
      m_executionMode = ExecutionMode::Serial;
   if (m_executionMode == ExecutionMode::MPI && !canUseMPI())
      m_executionMode = canUseOpenMP() ? ExecutionMode::OpenMP : ExecutionMode::Serial;
}

void Simulation::syncGuiScalars()
{
#ifdef ENABLE_MPI
   if (m_hasMpiRuntime && m_executionMode == ExecutionMode::MPI)
   {
      float scalars[4] = {
         config::alignmentScalar,
         config::cohesionScalar,
         config::separationScalar,
         config::speedScalar
      };
      MPI_Bcast(scalars, 4, MPI_FLOAT, m_runtimeMpiRootRank, MPI_COMM_WORLD);
      config::alignmentScalar = scalars[0];
      config::cohesionScalar = scalars[1];
      config::separationScalar = scalars[2];
      config::speedScalar = std::clamp(scalars[3], 0.4f, 2.2f);

      int counts[2] = {
         config::N_SPECIES,
         config::N_BOIDS
      };
      MPI_Bcast(counts, 2, MPI_INT, m_runtimeMpiRootRank, MPI_COMM_WORLD);

      config::N_SPECIES = std::clamp(counts[0], 1, config::MAX_N_SPECIES);
      config::N_BOIDS = std::clamp(counts[1], 1, config::MAX_N_BOIDS);

      computeLocalSpeciesRange();
      setupMPIGatherLayout();
   }
#endif
}

void Simulation::syncObstacles()
{
#ifdef ENABLE_MPI
   if (!m_hasMpiRuntime || m_executionMode != ExecutionMode::MPI)
      return;

   int obstacleCount = (m_mpiRank == m_runtimeMpiRootRank)
      ? static_cast<int>(m_obstacles.size())
      : 0;

   MPI_Bcast(&obstacleCount, 1, MPI_INT, m_runtimeMpiRootRank, MPI_COMM_WORLD);
   obstacleCount = std::clamp(obstacleCount, 0, config::MAX_N_OBSTACLES);

   std::vector<float> obstacleData(static_cast<size_t>(obstacleCount) * 3u, 0.0f);

   if (m_mpiRank == m_runtimeMpiRootRank)
   {
      for (int i = 0; i < obstacleCount; i++)
      {
         obstacleData[static_cast<size_t>(i) * 3u + 0u] = m_obstacles[i].getCenter().x;
         obstacleData[static_cast<size_t>(i) * 3u + 1u] = m_obstacles[i].getCenter().y;
         obstacleData[static_cast<size_t>(i) * 3u + 2u] = m_obstacles[i].getRadius();
      }
   }

   if (!obstacleData.empty())
   {
      MPI_Bcast(
         obstacleData.data(),
         static_cast<int>(obstacleData.size()),
         MPI_FLOAT,
         m_runtimeMpiRootRank,
         MPI_COMM_WORLD
      );
   }

   if (m_mpiRank == m_runtimeMpiRootRank)
      return;

   m_obstacles.clear();
   m_obstacles.reserve(config::MAX_N_OBSTACLES);

   for (int i = 0; i < obstacleCount; i++)
   {
      const size_t base = static_cast<size_t>(i) * 3u;
      const glm::vec2 center(obstacleData[base + 0u], obstacleData[base + 1u]);
      const float radius = obstacleData[base + 2u];
      m_obstacles.emplace_back(static_cast<unsigned>(i), center, radius);
   }
#endif
}

void Simulation::computeLocalSpeciesRange()
{
   const int species = config::N_SPECIES;
   const int size = (m_mpiWorldSize > 0) ? m_mpiWorldSize : 1;
   const int rank = (m_mpiRank >= 0) ? m_mpiRank : 0;
   const int base = species / size;
   const int remainder = species % size;

   m_localSpeciesStart = rank * base + std::min(rank, remainder);
   m_localSpeciesEnd = m_localSpeciesStart + base + (rank < remainder ? 1 : 0);
}

void Simulation::setupMPIGatherLayout()
{
   const int boidFloatsPerSpecies = config::N_BOIDS * 6;
   const int localSpecies = m_localSpeciesEnd - m_localSpeciesStart;
   const int localCount = localSpecies * boidFloatsPerSpecies;

   m_localBoidVertices.assign(localCount, 0.0f);

   if (!m_isRenderProcess)
      return;

   const int totalCount = config::N_SPECIES * boidFloatsPerSpecies;
   m_globalBoidVertices.assign(totalCount, 0.0f);

   if (!m_hasMpiRuntime)
      return;

   m_recvCounts.assign(m_mpiWorldSize, 0);
   m_recvDisplacements.assign(m_mpiWorldSize, 0);

   int displacement = 0;
   for (int rank = 0; rank < m_mpiWorldSize; rank++)
   {
      const int base = config::N_SPECIES / m_mpiWorldSize;
      const int remainder = config::N_SPECIES % m_mpiWorldSize;
      const int rankSpecies = base + (rank < remainder ? 1 : 0);

      m_recvCounts[rank] = rankSpecies * boidFloatsPerSpecies;
      m_recvDisplacements[rank] = displacement;
      displacement += m_recvCounts[rank];
   }
}

void Simulation::packLocalBoidVertices()
{
   const int localSpecies = m_localSpeciesEnd - m_localSpeciesStart;
   if (localSpecies <= 0)
      return;

   int idx = 0;
   for (int species = m_localSpeciesStart; species < m_localSpeciesEnd; species++)
   {
      for (int boid = 0; boid < config::N_BOIDS; boid++)
      {
         glm::vec2* pos = m_flocks[species].getBoidPosition(boid);
         for (int v = 0; v < 3; v++)
         {
            m_localBoidVertices[idx++] = pos[v].x;
            m_localBoidVertices[idx++] = pos[v].y;
         }
      }
   }
}

void Simulation::syncMPIBoidVertices()
{
   if (!canUseMPI())
      return;

#ifdef ENABLE_MPI
   packLocalBoidVertices();

   const int localCount = static_cast<int>(m_localBoidVertices.size());

   MPI_Gatherv(
      m_localBoidVertices.data(),
      localCount,
      MPI_FLOAT,
      m_isRenderProcess ? m_globalBoidVertices.data() : nullptr,
      m_isRenderProcess ? m_recvCounts.data() : nullptr,
      m_isRenderProcess ? m_recvDisplacements.data() : nullptr,
      MPI_FLOAT,
      m_runtimeMpiRootRank,
      MPI_COMM_WORLD
   );
#endif
}

void Simulation::uploadBoidVerticesFromBuffer(const std::vector<float>& vertices)
{
   const int boidFloatsPerSpecies = config::N_BOIDS * 6;

   for (int species = 0; species < config::N_SPECIES; species++)
   {
      const int speciesBase = species * boidFloatsPerSpecies;

      for (int boid = 0; boid < config::N_BOIDS; boid++)
      {
         const int boidBase = speciesBase + (boid * 6);
         unsigned int vertexOffset = (
               (6 * boid) +
               (species * config::MAX_N_BOIDS * 6)
         ) * sizeof(float);

         updateChunkOfBuffer(
               m_bufferVertex,
               vertexOffset,
               6,
               (void*)(&vertices[boidBase])
         );
      }
   }
}

void Simulation::updateChunkOfBuffer(
      const unsigned int buffer,
      const unsigned int vertexOffset,
      const unsigned int sizeData,
      void* data
){ 
   glBindVertexArray(m_VAO);
   glBindBuffer(GL_ARRAY_BUFFER, buffer);
   
   glBufferSubData(
         GL_ARRAY_BUFFER, vertexOffset, sizeof(float) * sizeData, data
   );

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
}

// Builds and compiles the shaders.
void Simulation::initShaders()
{
   try
   {
      const std::string vertexPath = std::string(PROJECT_ROOT_DIR) + "/shaders/boids.vert";
      const std::string fragmentPath = std::string(PROJECT_ROOT_DIR) + "/shaders/boids.frag";

      m_shaderProgram =
         glShaderLoader::loadShader(
            vertexPath.c_str(), 
            fragmentPath.c_str()
      );
   } catch(const std::exception& e)
   {
      std::cerr << e.what() << std::endl;
      m_appRunning = false;
   }
}

void Simulation::initBuffers()
{
   glGenVertexArrays(1, &m_VAO);
   
   createVertexBuffer();
   createColorBuffer();
   
   // Fills static buffers
   fillObstacleChunk();
   fillColorBoidBuffer();
}

void Simulation::createBuffer(
      const size_t bufferSize,
      unsigned int& buffer,
      const GLenum bufferType,
      const GLenum drawingType
) {

   glGenBuffers(1, &buffer);
   // We indicate the type of buffer
   glBindBuffer(bufferType, buffer);
   // We allocate the buffer with determined size but with 0 elements.
   glBufferData(bufferType, bufferSize, NULL, drawingType);

   // To unbind the buffer(state-machine..)
   glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Simulation::bindBufferToVAO(
      const unsigned int vao,
      const unsigned int index,
      const unsigned int buffer,
      const unsigned int offset,
      const unsigned int stride
) {
   // We bind the VAO to configure it.
   glBindVertexArray(vao);

   // 1) VAO.
   // 2) VAO's index that we want to use for this buffer.
   // (vertex buffer binding index)
   // (VAO contains "the pointers" to the VBOs.
   glEnableVertexArrayAttrib(vao, index);

   // 1) VAO,
   // 2) Vertex buffer binding index
   // 3) VBO
   // 4) Offset within the VBO.
   // 5) Stride(distance between the elements within the VBO).
   glVertexArrayVertexBuffer(vao, index, buffer, offset, stride);

   // After we finished all the configuration, we need to unbind the VAO to
   // prevent future errors.
   glBindVertexArray(0);
}

// Creates the "binding" with the shader.
void Simulation::bindVertexAttribute(
      const unsigned int vao,
      const unsigned int vaoIndex,
      const unsigned int shaderAttribIndex,
      const unsigned int nComponents,
      const GLenum type,
      const GLenum isNormalized,
      const unsigned int distance
) {

   glBindVertexArray(vao);

   // Associates a vertex attribute(an input variable to a shader that is 
   // supplied with per-vertex data) and a vertex buffer binding for a vertex
   // array object.
   // 1) VAO
   // 2) Shader attribute index(the one that is in the shader).
   // 3) Vertex buffer binding index.

   glVertexArrayAttribBinding(vao, shaderAttribIndex, vaoIndex);

   // 1) VAO.
   // 2) Shader Attribute index.
   // 3) Number of components.
   // 4) Type of component.
   // 5) If the components are normalized.
   // 6) Distance between elements within the buffer.
   glVertexArrayAttribFormat(
         vao, shaderAttribIndex, nComponents, type, isNormalized, distance
   );

   glBindVertexArray(0);
}


// Creates the buffer that will contains all the vertices of the boids and
// the obstacles.
void Simulation::createVertexBuffer()
{
   // Create buffer
   size_t bufferSize = (
       (6 * config::MAX_N_BOIDS * config::MAX_N_SPECIES) +
       (6 * config::N_TRIANG_PER_CIRCLE * config::MAX_N_OBSTACLES)
   ) * sizeof(float);

   // Create a big empty buffer for all the boids and the obstacles.
   createBuffer(bufferSize, m_bufferVertex, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
   bindBufferToVAO(m_VAO, 0, m_bufferVertex, 0, sizeof(float) * 2);
   bindVertexAttribute(m_VAO, 0, 0, 2, GL_FLOAT, GL_FALSE, 0);
}

// Creates the buffer that will contains all the colors for all the boids
// and obstacles.
void Simulation::createColorBuffer()
{
   size_t bufferSize = (
         (9 * config::MAX_N_BOIDS * config::MAX_N_SPECIES) +
         (9 * config::N_TRIANG_PER_CIRCLE * config::MAX_N_OBSTACLES)
   ) * sizeof(float);

   createBuffer(bufferSize, m_bufferColor, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
   bindBufferToVAO(m_VAO, 1, m_bufferColor, 0, sizeof(float) * 3);
   bindVertexAttribute(m_VAO, 1, 1, 3, GL_FLOAT, GL_FLOAT, 0);
}

// Fills the chunk that belongs to the boid's color.
void Simulation::fillColorBoidBuffer()
{
   for (int i = 0; i < config::N_SPECIES; i++)
   {
      for (int j = 0; j < config::N_BOIDS; j++)
      {
         const unsigned int vertexOffset = (
             (9 * j) +
             (i * config::MAX_N_BOIDS * 9)
         ) * sizeof(float);

         updateChunkOfBuffer(
               m_bufferColor,
               vertexOffset,
               9,
               m_flocks[i].getColor()
         );
      }
   }
}

// Fills the color and vertex chunks of the obstacles.
void Simulation::fillObstacleChunk()
{
   const unsigned int startIndexVertex =
      6 * config::MAX_N_BOIDS * config::MAX_N_SPECIES * sizeof(float);
   const unsigned int startIndexColor = 
      9 * config::MAX_N_SPECIES * config::MAX_N_BOIDS * sizeof(float);


   for (int i = 0; i < static_cast<int>(m_obstacles.size()); i++)
   {
      for (int j = 0; j < config::N_TRIANG_PER_CIRCLE; j++)
      {

         // Fills the vertex buffer

         const unsigned int vertexOffset = (
             (6 * j) +
             (i * config::N_TRIANG_PER_CIRCLE * 6)
         ) * sizeof(float) + startIndexVertex;

         updateChunkOfBuffer(
               m_bufferVertex,
               vertexOffset,
               6,
               m_obstacles[i].getPos(j)
         );

         // Fills the color buffer

         const unsigned int colorOffset = (
             (9 * j) +
             (i * config::N_TRIANG_PER_CIRCLE * 9)
         ) * sizeof(float) + startIndexColor;
         
         updateChunkOfBuffer(
               m_bufferColor,
               colorOffset,
               9,
               m_obstacles[i].getColor()
         );

      }
   }
}

// ========================== ImGui functions ==========================

void Simulation::initImGui()
{
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO(); (void) io;

   applyCyberImGuiTheme();
   
   ImGui_ImplSDL2_InitForOpenGL(m_window->getWindow(), m_glContext);
   ImGui_ImplOpenGL3_Init();
}

void Simulation::updateGUI()
{
   // Starts the Dear ImGui frame
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplSDL2_NewFrame(m_window->getWindow());
   ImGui::NewFrame();

   ImGuiIO& io = ImGui::GetIO();
   ImFont* uiFont = ImGui::GetFont();
   const float uiFontSize = ImGui::GetFontSize();
   const float panelWidth = getControlPanelWidth(io.DisplaySize.x);
   const ImVec2 simulationPanelPos(panelWidth, 0.0f);
   const ImVec2 simulationPanelSize(std::max(0.0f, io.DisplaySize.x - panelWidth), io.DisplaySize.y);
   const ImVec2 panelMin(0.0f, 0.0f);
   const ImVec2 panelMax(panelWidth, io.DisplaySize.y);

   ImDrawList* bg = ImGui::GetBackgroundDrawList();
   const ImVec2 gridMin = simulationPanelPos;
   const ImVec2 gridMax(io.DisplaySize.x, io.DisplaySize.y);
   const ImU32 gridColor = IM_COL32(44, 255, 106, 18);
   const float step = 36.0f;

   for (float x = gridMin.x + step; x < gridMax.x; x += step)
      bg->AddLine(ImVec2(x, gridMin.y), ImVec2(x, gridMax.y), gridColor, 1.0f);

   for (float y = gridMin.y + step; y < gridMax.y; y += step)
      bg->AddLine(ImVec2(gridMin.x, y), ImVec2(gridMax.x, y), gridColor, 1.0f);

   bg->AddRectFilled(panelMin, panelMax, IM_COL32(0, 6, 0, 240), 0.0f);
   bg->AddRectFilled(ImVec2(panelMin.x + 8.0f, panelMin.y + 8.0f), ImVec2(panelMax.x - 8.0f, panelMax.y - 8.0f), IM_COL32(0, 12, 0, 228), 0.0f);

   for (int i = 0; i < 3; ++i)
   {
      const float expand = 2.0f + (float)i * 2.0f;
      const int alpha = 70 - (i * 18);
      bg->AddRect(
         ImVec2(panelMin.x - expand, panelMin.y - expand),
         ImVec2(panelMax.x + expand, panelMax.y + expand),
         IM_COL32(44, 255, 106, alpha),
         0.0f,
         0,
         1.0f
      );
   }

   bg->AddRect(ImVec2(panelMin.x + 1.0f, panelMin.y + 1.0f), ImVec2(panelMax.x - 1.0f, panelMax.y - 1.0f), IM_COL32(44, 255, 106, 220), 0.0f, 0, 1.0f);
   bg->AddLine(ImVec2(panelWidth, 0.0f), ImVec2(panelWidth, io.DisplaySize.y), IM_COL32(44, 255, 106, 210), 1.0f);

   const char* modeLabel = "Serial";
   if (m_executionMode == ExecutionMode::OpenMP)
      modeLabel = "OpenMP";
   else if (m_executionMode == ExecutionMode::MPI)
      modeLabel = "MPI";

   auto drawSectionTitle = [&](const char* title)
   {
      ImGui::TextUnformatted(title);
   };

   // Draws the GUI
   ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
   ImGui::SetNextWindowSize(ImVec2(panelWidth, io.DisplaySize.y));
   ImGui::SetNextWindowBgAlpha(0.0f);
   ImGuiWindowFlags panelFlags = ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoBackground |
      ImGuiWindowFlags_NoTitleBar |
      ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoBringToFrontOnFocus;
   ImGui::Begin("##boid_control_panel", nullptr, panelFlags);
   ImGui::PushTextWrapPos(0.0f);

   ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.00f, 0.07f, 0.00f, 0.92f));
   ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.24f, 0.94f, 0.44f, 0.95f));
   if (ImGui::BeginChild("##cyber_status_strip", ImVec2(0.0f, 54.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
   {
      ImGui::Text(">_ BOID-SIM v1.0");

      std::string statusText = std::string("SYS.STATUS: ") + (m_appRunning ? "ACTIVE" : "NOT ACTIVE") +
         "   MODE: " + modeLabel;
      ImGui::TextWrapped("%s", statusText.c_str());
   }
   ImGui::EndChild();
   ImGui::PopStyleColor(2);

   ImGui::Spacing();
   drawSectionTitle("[ TELEMETRY ]");
   ImGui::Separator();
   ImGui::Text("FPS: %u", m_FPS);
   ImGui::Text("SPECIES: %d", config::N_SPECIES);
   ImGui::Text("BOIDS / SPECIES: %d", config::N_BOIDS);
   ImGui::Text("OBSTACLES: %d / %d", static_cast<int>(m_obstacles.size()), config::MAX_N_OBSTACLES);

   ImGui::Spacing();
   drawSectionTitle("[ EXECUTION MODE ]");
   ImGui::Separator();

   auto drawModeButton = [&](const char* label, const ExecutionMode mode, const bool available)
   {
      const bool selected = (m_executionMode == mode);

      if (!available)
         ImGui::BeginDisabled();

      ImGui::PushStyleColor(ImGuiCol_Button, selected ? ImVec4(0.14f, 0.74f, 0.28f, 0.98f) : ImVec4(0.00f, 0.11f, 0.01f, 0.94f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selected ? ImVec4(0.22f, 0.92f, 0.38f, 1.0f) : ImVec4(0.05f, 0.28f, 0.07f, 0.98f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.30f, 1.0f, 0.50f, 1.0f));

      if (ImGui::Button(label, ImVec2(92.0f, 0.0f)))
      {
         m_executionMode = mode;
      }

      ImGui::PopStyleColor(3);

      if (!available)
         ImGui::EndDisabled();
   };

   drawModeButton("SERIAL", ExecutionMode::Serial, true);
   ImGui::SameLine();
   drawModeButton("OPENMP", ExecutionMode::OpenMP, canUseOpenMP());
   ImGui::SameLine();
   drawModeButton("MPI", ExecutionMode::MPI, m_hasMpiRuntime);

   ImGui::Text("ACTIVE MODE: %s", modeLabel);

   if (!canUseOpenMP())
      ImGui::Text("OpenMP: not available in this build");
   else
      ImGui::Text("OpenMP threads: %d", getOpenMPThreads());

   if (m_hasMpiRuntime)
      ImGui::Text("MPI world size: %d", m_mpiWorldSize);
   else
      ImGui::Text("MPI: not available in this build");

   if (m_hasMpiRuntime && m_mpiWorldSize == 1)
      ImGui::Text("Tip: launch with RUN_WITH_MPI=1 MPI_PROCS=4 bash run.sh for real multi-rank MPI.");

   ImGui::Spacing();
   drawSectionTitle("[ PERFORMANCE ]");
   ImGui::Separator();
   ImGui::Text("Simulation update (avg): %.3f ms", m_avgUpdateMs);
   ImGui::Text("MPI communication (avg): %.3f ms", m_avgCommMs);
   ImGui::Text("Render time (avg): %.3f ms", m_avgRenderMs);
   ImGui::Text("Frame time (avg): %.3f ms", m_avgFrameMs);
   ImGui::Text("Frame time (inst): %.3f ms", m_lastFrameMs);
   ImGui::Text("Serial baseline: %.3f ms", m_serialBaselineMs);

   ImGui::Spacing();
   drawSectionTitle("[ PARALLEL METRICS ]");
   ImGui::Separator();
   ImGui::Text("Speedup vs Serial: %.2fx", getSpeedup());
   ImGui::Text("Parallel Efficiency: %.1f%%", getEfficiency() * 100.0f);
   ImGui::Text("Compute vs Comms: %.1f%% simulation", (1.0f - getCommPercentage()) * 100.0f);

   if (m_executionMode == ExecutionMode::MPI)
      ImGui::Text("Rank 0 renders, all ranks simulate partitioned species.");

   ImGui::Spacing();
   drawSectionTitle("[ SWARM TUNING ]");
   ImGui::Separator();
   bool countChanged = false;
   countChanged |= ImGui::SliderInt("SPECIES", &config::N_SPECIES, 1, config::MAX_N_SPECIES);
   countChanged |= ImGui::SliderInt("BOIDS / SPECIES", &config::N_BOIDS, 1, config::MAX_N_BOIDS);

   if (countChanged)
   {
      computeLocalSpeciesRange();
      setupMPIGatherLayout();
   }

   ImGui::SliderFloat("ALIGNMENT" , &config::alignmentScalar, 0.0f, 2.0f);
   ImGui::SliderFloat("COHESION"  , &config::cohesionScalar, 0.0f, 2.0f);
   ImGui::SliderFloat("SEPARATION", &config::separationScalar, 0.0f, 2.0f);
   ImGui::SliderFloat("SPEED", &config::speedScalar, 0.4f, 2.2f, "%.2fx");

   ImGui::Spacing();
   drawSectionTitle("[ SPECIES COLORS ]");
   ImGui::Separator();
   bool speciesColorChanged = false;
   for (int i = 0; i < config::N_SPECIES; i++)
   {
      glm::vec3* color = m_flocks[i].getColor();
      float rgb[3] = { color->r, color->g, color->b };
      std::string label = "Species " + std::to_string(i + 1);

      if (ImGui::ColorEdit3(label.c_str(), rgb, ImGuiColorEditFlags_NoInputs))
      {
         m_flocks[i].setColor(glm::vec3(rgb[0], rgb[1], rgb[2]));
         speciesColorChanged = true;
      }
   }

   if (speciesColorChanged)
      fillColorBoidBuffer();

   ImGui::Spacing();
   drawSectionTitle("[ MPI CONTROL ]");
   ImGui::Separator();
   ImGui::Text("ACTIVE RANK: %d", m_mpiRank);
   ImGui::Text("ACTIVE PROCESSES: %d", m_mpiWorldSize);

   ImGui::InputInt("MPI PROCESSES (NEXT RUN)", &m_targetMpiProcs);
   ImGui::InputInt("MPI RANK (NEXT RUN)", &m_targetMpiRank);

   m_targetMpiProcs = std::clamp(m_targetMpiProcs, 1, 64);
   m_targetMpiRank = std::clamp(m_targetMpiRank, 0, m_targetMpiProcs - 1);

   if (m_targetMpiProcs != m_mpiWorldSize || m_targetMpiRank != m_mpiRank)
      ImGui::Text("Applies on next launch.");

   ImGui::TextUnformatted("NEXT LAUNCH COMMAND:");
   std::string mpiLaunchHint = buildMpiRelaunchCommand();
   ImGui::TextWrapped("%s", mpiLaunchHint.c_str());

   ImGui::PopTextWrapPos();
   ImGui::End();

   ImDrawList* fg = ImGui::GetForegroundDrawList();
   const float hudHeight = 34.0f;
   const ImVec2 hudMin(simulationPanelPos.x + 12.0f, simulationPanelPos.y + 10.0f);
   const ImVec2 hudMax(simulationPanelPos.x + simulationPanelSize.x - 12.0f, hudMin.y + hudHeight);
   fg->AddRectFilled(hudMin, hudMax, IM_COL32(0, 10, 0, 200), 0.0f);
   fg->AddRect(hudMin, hudMax, IM_COL32(44, 255, 106, 210), 0.0f, 0, 1.0f);

   const std::string hudLeft = "[ LIVE SIMULATION VIEW ]";
   const std::string hudRight = std::string("FPS: ") + std::to_string(m_FPS) + "   MODE: " + modeLabel;

   const float hudTitleSize = std::round(uiFontSize);
   const ImVec2 hudTitlePos(
      std::floor(hudMin.x + 10.0f),
      std::floor(hudMin.y + 8.0f)
   );
   fg->AddText(uiFont, hudTitleSize, hudTitlePos, IM_COL32(110, 255, 140, 255), hudLeft.c_str());
   const ImVec2 rightTextSize = ImGui::CalcTextSize(hudRight.c_str());
   fg->AddText(
      ImVec2(hudMax.x - rightTextSize.x - 10.0f, hudMin.y + 9.0f),
      IM_COL32(110, 255, 140, 255),
      hudRight.c_str()
   );

   if (simulationPanelSize.x > 1.0f && simulationPanelSize.y > 1.0f)
   {
      const char* obstacleLabel = "OBSTACLE";
      const float viewportScaleX = std::clamp(simulationPanelSize.x / 760.0f, 0.85f, 1.20f);
      const float viewportScaleY = std::clamp(simulationPanelSize.y / 700.0f, 0.88f, 1.16f);
      const float viewportScale = std::min(viewportScaleX, viewportScaleY);
      const float pixelsPerWorld = std::min(simulationPanelSize.x, simulationPanelSize.y) * 0.5f;

      for (const auto& obstacle : m_obstacles)
      {
         const glm::vec2 center = obstacle.getCenter();
         const float screenX = simulationPanelPos.x + ((center.x + 1.0f) * 0.5f) * simulationPanelSize.x;
         const float screenY = ((1.0f - center.y) * 0.5f) * simulationPanelSize.y;
         const float obstacleRadiusPx = std::max(6.0f, obstacle.getRadius() * pixelsPerWorld);

         const ImVec2 obstacleCenter(std::floor(screenX), std::floor(screenY));
         fg->AddCircle(obstacleCenter, obstacleRadiusPx + 3.5f, IM_COL32(46, 235, 96, 42), 48, 3.0f);
         fg->AddCircle(obstacleCenter, obstacleRadiusPx + 1.8f, IM_COL32(46, 235, 96, 58), 48, 2.0f);
         fg->AddCircleFilled(obstacleCenter, obstacleRadiusPx, IM_COL32(46, 235, 96, 230), 48);
         fg->AddCircle(obstacleCenter, obstacleRadiusPx, IM_COL32(0, 0, 0, 240), 48, 2.0f);

         const float baseScale = std::clamp(0.95f + (obstacle.getRadius() * 3.8f), 0.9f, 1.35f);
         const float labelScale = std::clamp(baseScale * viewportScale, 0.82f, 1.45f);
         const float labelFontSize = std::round(uiFontSize * labelScale);
         const ImVec2 labelSize = uiFont->CalcTextSizeA(labelFontSize, FLT_MAX, -1.0f, obstacleLabel);
         const ImVec2 textPos(
            std::floor(screenX - (labelSize.x * 0.5f)),
            std::floor(screenY - (labelSize.y * 0.5f))
         );
         fg->AddText(uiFont, labelFontSize, textPos, IM_COL32(0, 0, 0, 245), obstacleLabel);
      }
   }
}

void Simulation::recordMetrics()
{
   const float alpha = 0.12f;

   if (m_frameSampleCount == 0)
   {
      m_avgUpdateMs = m_lastUpdateMs;
      m_avgCommMs = m_lastCommMs;
      m_avgRenderMs = m_lastRenderMs;
      m_avgFrameMs = m_lastFrameMs;
   }
   else
   {
      m_avgUpdateMs = (1.0f - alpha) * m_avgUpdateMs + alpha * m_lastUpdateMs;
      m_avgCommMs = (1.0f - alpha) * m_avgCommMs + alpha * m_lastCommMs;
      m_avgRenderMs = (1.0f - alpha) * m_avgRenderMs + alpha * m_lastRenderMs;
      m_avgFrameMs = (1.0f - alpha) * m_avgFrameMs + alpha * m_lastFrameMs;
   }

   // Learn a stable serial baseline from averaged serial frames.
   if (m_executionMode == ExecutionMode::Serial)
   {
      if (m_serialBaselineFrames == 0)
         m_serialBaselineMs = m_avgFrameMs;
      else
         m_serialBaselineMs = 0.95f * m_serialBaselineMs + 0.05f * m_avgFrameMs;

      m_serialBaselineFrames++;
   }
   else if (m_serialBaselineMs == 0.0f)
   {
      // Fallback when user starts in parallel mode before ever running serial.
      m_serialBaselineMs = m_avgFrameMs;
   }

   m_frameSampleCount++;
}

float Simulation::getSpeedup() const
{
   if (m_serialBaselineMs < 0.001f)
      return 1.0f;
   return m_serialBaselineMs / std::max(m_avgFrameMs, 0.001f);
}

float Simulation::getEfficiency() const
{
   const int parallelCount = (m_executionMode == ExecutionMode::OpenMP)
      ? getOpenMPThreads()
      : ((m_executionMode == ExecutionMode::MPI) ? m_mpiWorldSize : 1);

   if (parallelCount <= 1)
      return 1.0f;

   const float speedup = getSpeedup();
   const float maxPossibleSpeedup = static_cast<float>(parallelCount);
   
   return std::min(1.0f, speedup / maxPossibleSpeedup);
}

float Simulation::getCommPercentage() const
{
   if (m_executionMode != ExecutionMode::MPI)
      return 0.0f;

   const float totalTime = m_avgUpdateMs + m_avgCommMs;
   if (totalTime < 0.001f)
      return 0.0f;

   return m_avgCommMs / totalTime;
}
