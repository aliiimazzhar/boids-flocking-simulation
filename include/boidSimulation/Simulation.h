#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <array>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <SDL2/SDL.h>
#include <glad/glad.h>

#ifdef ENABLE_MPI
#include <mpi.h>
#endif

#ifdef ENABLE_OPENMP
#include <omp.h>
#endif

#include <boidSimulation/Window/WindowManager.h>
#include <boidSimulation/Renderer/Renderer.h>
#include <boidSimulation/Shaders/glShaderLoader.h>
#include <boidSimulation/Settings/config.h>
#include <boidSimulation/Simulation/Flock.h>
#include <boidSimulation/Simulation/Obstacle.h>

class Simulation
{

public:

   enum class ExecutionMode : int
   {
      Serial = 0,
      OpenMP = 1,
      MPI = 2
   };


   Simulation();
   ~Simulation();

   void run();

private:

   std::vector<Flock> m_flocks;
   std::vector<Obstacle> m_obstacles;

   bool m_appRunning;
   unsigned int m_FPS;
   
   WindowManager* m_window;
   Renderer*      m_renderer;
   SDL_GLContext  m_glContext;
   unsigned int   m_shaderProgram;

   unsigned int m_bufferVertex, m_bufferColor;
   unsigned int m_VAO;

   ExecutionMode m_executionMode;
   bool m_modeChangedByGui;
   float m_lastUpdateMs;
   float m_lastCommMs;
   float m_lastRenderMs;
   float m_lastFrameMs;
   float m_serialBaselineMs;
   float m_avgUpdateMs;
   float m_avgCommMs;
   float m_avgRenderMs;
   float m_avgFrameMs;
   int m_frameSampleCount;
   int m_serialBaselineFrames;

   int m_totalBoids;

   int m_mpiRank;
   int m_mpiWorldSize;
   int m_runtimeMpiRootRank;
   int m_targetMpiProcs;
   int m_targetMpiRank;
   int m_localSpeciesStart;
   int m_localSpeciesEnd;
   bool m_hasMpiRuntime;
   bool m_isRenderProcess;
   bool m_isFullscreen;

   std::vector<float> m_localBoidVertices;
   std::vector<float> m_globalBoidVertices;
   std::vector<int> m_recvCounts;
   std::vector<int> m_recvDisplacements;

   // ========================== Methods ==========================

   void update();
   void render();
   void checkUserInput();
   void calculateFPS();
   void createFlocks();
   void createObstacles();
   void addObstacleAtClick(const glm::vec2& clickPos);
   std::string buildMpiRelaunchCommand() const;
   void updateFlocks();
   void setupExecutionMode();
   bool canUseOpenMP() const;
   bool canUseMPI() const;
   int getOpenMPThreads() const;
   void syncRunState();
   void syncExecutionMode();
   void syncGuiScalars();
   void syncObstacles();
   void computeLocalSpeciesRange();
   void setupMPIGatherLayout();
   void packLocalBoidVertices();
   void syncMPIBoidVertices();
   void uploadBoidVerticesFromBuffer(const std::vector<float>& vertices);
   void recordMetrics();
   float getSpeedup() const;
   float getEfficiency() const;
   float getCommPercentage() const;

   // OpenGL functions
   void initGLAD();
   void createBuffer(
         const size_t bufferSize,
         unsigned int& buffer,
         const GLenum bufferType,
         const GLenum drawingType
   );
   void bindBufferToVAO(
      const unsigned int vao,
      const unsigned int index,
      const unsigned int buffer,
      const unsigned int offset,
      const unsigned int stride
   );
   void bindVertexAttribute(
      const unsigned int vao,
      const unsigned int vaoIndex,
      const unsigned int shaderAttribIndex,
      const unsigned int nComponents,
      const GLenum type,
      const GLenum isNormalized,
      const unsigned int distance
   );
   void updateBoidChunk();
   void updateChunkOfBuffer(
         const unsigned int buffer,
         const unsigned int vertexOffset,
         const unsigned int sizeData,
         void* data
   );
   void initBuffers();
   void createVertexBuffer();
   void createColorBuffer();
   void fillObstacleChunk();
   void fillColorBoidBuffer();
   void initShaders();

   // ImGui functions
   void initImGui();
   void updateGUI();

};
