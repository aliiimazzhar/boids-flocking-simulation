#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <boidSimulation/Simulation/Boid.h>
#include <boidSimulation/Simulation/Obstacle.h>

class Flock
{

public:

   Flock();
   ~Flock();
   void toFlock(const std::vector<Obstacle>& obstacles, const int activeBoids);
   glm::vec2* getBoidPosition(const int i);
   glm::vec3* getColor();
   void setColor(const glm::vec3& color);

private:

   std::vector<Boid> m_boids;
   glm::vec3 m_color[3];

   glm::vec2 alignment(const Boid& boid, const int activeBoids) const;
   glm::vec2 cohesion(const Boid& boid, const int activeBoids) const;
   glm::vec2 separation(const Boid& boid, const int activeBoids) const;
   
   glm::vec2 getAverageVector(const Boid& boid, const int type, const int activeBoids) const;
   glm::vec3 pickRandColor() const;
   glm::vec2 avoidObstacles(
         const Boid& boid, const std::vector<Obstacle>& obstacles
   ) const;
      void resolveObstacleCollision(
         Boid& boid, const std::vector<Obstacle>& obstacles
      ) const;
};
