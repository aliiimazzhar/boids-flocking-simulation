#include <boidSimulation/Simulation/Obstacle.h>

#include <boidSimulation/Settings/config.h>

#include <cmath>

Obstacle::Obstacle(const unsigned id) : m_id(id)
{
   std::uniform_real_distribution<> randPos(0.0, 1.0);
   std::uniform_real_distribution<> randRad(0.05, 0.1);

   m_center = glm::vec2(
         (randPos(config::gen) * 2.0) - 1.0,
         (randPos(config::gen) * 2.0) - 1.0
   );
   m_radius = randRad(config::gen);
      m_color[0] = m_color[1] = m_color[2] = glm::vec3(0.17f, 0.98f, 0.42f);

      createCircleTriangles();
}

Obstacle::Obstacle(const unsigned id, const glm::vec2& center) : m_id(id)
{
   std::uniform_real_distribution<> randRad(0.05, 0.1);

   m_center = center;
   m_radius = randRad(config::gen);
   m_color[0] = m_color[1] = m_color[2] = glm::vec3(0.17f, 0.98f, 0.42f);

   createCircleTriangles();
}

Obstacle::Obstacle(const unsigned id, const glm::vec2& center, float radius) : m_id(id)
{
   m_center = center;
   m_radius = radius;
   m_color[0] = m_color[1] = m_color[2] = glm::vec3(0.17f, 0.98f, 0.42f);

   createCircleTriangles();
}

void Obstacle::createCircleTriangles()
{
   constexpr float pi2 = 3.141592f * 2.0f;

   for (int i = 0; i < config::N_TRIANG_PER_CIRCLE; i++)
   {
      const float angleA = pi2 * static_cast<float>(i) / static_cast<float>(config::N_TRIANG_PER_CIRCLE);
      const float angleB = pi2 * static_cast<float>(i + 1) / static_cast<float>(config::N_TRIANG_PER_CIRCLE);

      m_pos[i][0] = m_center;
      m_pos[i][1] = glm::vec2(
         m_center.x + std::cos(angleA) * m_radius,
         m_center.y + std::sin(angleA) * m_radius
      );
      m_pos[i][2] = glm::vec2(
         m_center.x + std::cos(angleB) * m_radius,
         m_center.y + std::sin(angleB) * m_radius
      );
   }
}

bool Obstacle::doesCollide(const std::vector<Obstacle>& obstacles) const
{
   for (const auto& other : obstacles)
   {
      if (m_id == other.m_id)
         continue;

      float sumOfRadius = other.m_radius + m_radius;
      float distance = fabs(
            glm::distance(other.m_center, m_center)
      ) - sumOfRadius;

      if (distance <= 0.0)
         return true;
   }

   return false;
}

glm::vec3* Obstacle::getColor()
{
   return &(m_color[0]);
}

glm::vec2* Obstacle::getPos(const unsigned int i)
{
   return &(m_pos[i][0]);
}

glm::vec2 Obstacle::getCenter() const
{
   return m_center;
}

float Obstacle::getRadius() const
{
   return m_radius;
}
