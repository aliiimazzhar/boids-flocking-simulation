cmake_minimum_required(VERSION 3.9.1)

project(BoidSimulation)

option(ENABLE_OPENMP "Enable OpenMP parallel updates" ON)
option(ENABLE_MPI "Enable MPI distributed updates" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin")
set(PROJECT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(PROJECT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(PROJECT_LIBRARIES_DIR "${CMAKE_SOURCE_DIR}/libs")

# Libs

# -SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# -GLAD(OpenGL)
set(GLAD_DIR "${PROJECT_LIBRARIES_DIR}/glad")
add_library(glad "${GLAD_DIR}/src/glad.c")
include_directories("${GLAD_DIR}/include")

# -GLM
set(GLM_DIR "${PROJECT_LIBRARIES_DIR}/glm")
add_subdirectory(${GLM_DIR} EXCLUDE_FROM_ALL)

# -ImGui
# Compile imGUI as an static library.
set(IMGUI_DIR "${PROJECT_LIBRARIES_DIR}/imgui")
file(GLOB IMGUI_SOURCES
   ${IMGUI_DIR}/*.cpp 
   ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
   ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp)
add_library(imgui ${IMGUI_SOURCES})
target_link_libraries(imgui PUBLIC glad)

target_include_directories(
   imgui 
   PUBLIC 
      ${IMGUI_DIR} 
      ${IMGUI_DIR}/backends
)

# Executable
add_executable(${PROJECT_NAME} 
   "${PROJECT_SOURCE_DIR}/boidSimulation/main.cpp"
   "${PROJECT_SOURCE_DIR}/boidSimulation/Shaders/glShaderLoader.cpp"
   "${PROJECT_SOURCE_DIR}/boidSimulation/Renderer/Renderer.cpp"
   "${PROJECT_SOURCE_DIR}/boidSimulation/Window/WindowManager.cpp"
   "${PROJECT_SOURCE_DIR}/boidSimulation/Math/util.cpp"
   "${PROJECT_SOURCE_DIR}/boidSimulation/Simulation/Boid.cpp"
   "${PROJECT_SOURCE_DIR}/boidSimulation/Simulation/Flock.cpp"
   "${PROJECT_SOURCE_DIR}/boidSimulation/Simulation/Simulation.cpp"
   "${PROJECT_SOURCE_DIR}/boidSimulation/Simulation/Obstacle.cpp"
)

target_include_directories(
   ${PROJECT_NAME}
   PUBLIC
      "${PROJECT_INCLUDE_DIR}"
   PRIVATE
      "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(${PROJECT_NAME} PUBLIC
   glad 
   glm
   ${SDL2_LIBRARIES} 
   imgui
   PRIVATE ${CMAKE_DL_LIBS})
   # CMAKE_DL_LIBS -> is the library libdl which helps to link dynamic
   # libraries. We need it in order to use GLAD.

if(ENABLE_OPENMP)
   find_package(OpenMP QUIET)
   if(OpenMP_CXX_FOUND)
      target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
      target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_OPENMP=1)
      message(STATUS "OpenMP enabled")
   else()
      message(STATUS "OpenMP not found, building without OpenMP")
   endif()
endif()

if(ENABLE_MPI)
   find_package(MPI QUIET COMPONENTS CXX)
   if(MPI_CXX_FOUND)
      target_link_libraries(${PROJECT_NAME} PRIVATE MPI::MPI_CXX)
      target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_MPI=1)
      message(STATUS "MPI enabled")
   else()
      message(STATUS "MPI not found, building without MPI")
   endif()
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")

