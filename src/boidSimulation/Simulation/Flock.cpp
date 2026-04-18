#include <boidSimulation/Simulation/Flock.h>

#include <boidSimulation/Settings/config.h>
#include <boidSimulation/Math/util.h>

#include <algorithm>
#include <cmath>

#define ALIGNMENT  0
#define COHESION   1
#define SEPARATION 2

namespace
{
constexpr float kObstacleNudge = 0.0015f;
constexpr float kEpsilon = 0.00001f;

glm::vec2 closestPointOnSegment(const glm::vec2& a, const glm::vec2& b, const glm::vec2& p)
{
   const glm::vec2 ab = b - a;
   const float abLen2 = glm::dot(ab, ab);
   if (abLen2 <= kEpsilon)
      return a;

   const float t = std::clamp(glm::dot(p - a, ab) / abLen2, 0.0f, 1.0f);
   return a + (ab * t);
}

const glm::vec3 kSpeciesPalette[] = {
   glm::vec3(0.95f, 0.42f, 0.42f),
   glm::vec3(0.35f, 0.80f, 1.00f),
   glm::vec3(0.98f, 0.85f, 0.35f),
   glm::vec3(0.83f, 0.50f, 1.00f),
   glm::vec3(0.35f, 1.00f, 0.62f),
};

int s_speciesColorIndex = 0;
}

Flock::Flock()
{
   m_boids.reserve(config::MAX_N_BOIDS);
   m_color[0] = m_color[1] = m_color[2] = pickRandColor();
   
   for (int i = 0; i < config::MAX_N_BOIDS; i++)
      m_boids.push_back(Boid(i));
}

void Flock::toFlock(const std::vector<Obstacle>& obstacles, const int activeBoids)
{
   const float maxSpeed = config::MAX_SPEED * config::speedScalar;
   const float minSpeed = config::MIN_SPEED * config::speedScalar;
   const int boidsToUpdate = std::clamp(activeBoids, 0, (int)m_boids.size());

   for (int i = 0; i < boidsToUpdate; i++)
   {
      auto& boid = m_boids[i];
      glm::vec2 newAcceleration = glm::vec2(0.0);
      
      newAcceleration += alignment(boid, boidsToUpdate)  * config::alignmentScalar;
      newAcceleration += cohesion(boid, boidsToUpdate)   * config::cohesionScalar;
      newAcceleration += separation(boid, boidsToUpdate) * config::separationScalar;

      // Avoids collisions
      newAcceleration += avoidObstacles(boid, obstacles);
      
      // Makes the acceleration softer(more realistic).
      newAcceleration *= 0.005f;
      
      // Updates the velocity
      boid.m_velocity += newAcceleration;
      
      // Limits the velocity because if not, it will increase infinitly.
      boid.m_velocity = util::clampMag(boid.m_velocity, maxSpeed);
      
      // If we changed the velocity it means that it is almost
      // for sure that we changed the direction, so we have
      // to rotate the vertices of the triangle.
      if (!util::isZeroVector(newAcceleration))
         boid.rotate();
      
      boid.moveFoward();
      resolveObstacleCollision(boid, obstacles);

      if (glm::length(boid.m_velocity) < minSpeed)
         boid.m_velocity = util::setMag(boid.m_velocity, minSpeed);

      boid.keepInsideBorders();
   }
}

glm::vec2 Flock::avoidObstacles(
      const Boid& boid, const std::vector<Obstacle>& obstacles) const
{
   const float maxSpeed = config::MAX_SPEED * config::speedScalar;
   const float boidCollisionRadius = (config::B_HEIGHT * 0.5f) + config::B_WIDTH;

   glm::vec2 avg = glm::vec2(0.0);
   int nNeighbords = 0;

   for (const auto& obstacle : obstacles)
   {
         const float safetyRadius = obstacle.m_radius + boidCollisionRadius + 0.02f;
         const float lookAhead = 6.0f;
         const glm::vec2 futureCenter = boid.m_center + (boid.m_velocity * lookAhead);
         float d = fabs(glm::length(obstacle.m_center - boid.m_center));
         float dFuture = fabs(glm::length(obstacle.m_center - futureCenter));

         if (d < safetyRadius || dFuture < safetyRadius)
         {
            glm::vec2 oppositeDir = boid.m_center - obstacle.m_center;

            if (d > kEpsilon)
               oppositeDir /= d;
            else if (!util::isZeroVector(boid.m_velocity))
               oppositeDir = glm::normalize(-boid.m_velocity);
            else
               oppositeDir = glm::vec2(1.0f, 0.0f);

            avg += oppositeDir;
            nNeighbords++;
         }
   }

   if (nNeighbords == 0)
      return avg;

   glm::vec2 avgVelocity = avg / float(nNeighbords);

   avgVelocity = util::setMag(avgVelocity, maxSpeed);

   glm::vec2 steering = util::getSteeringVector(
         avgVelocity,
         boid.m_velocity
   );

   // Medium-strength steering feels more organic than hard rebound.
   return steering * 3.0f;
}

void Flock::resolveObstacleCollision(
      Boid& boid, const std::vector<Obstacle>& obstacles) const
{
   const float maxSpeed = config::MAX_SPEED * config::speedScalar;
   const float boidCollisionRadius = (config::B_HEIGHT * 0.5f) + config::B_WIDTH;
   const glm::vec2 previousCenter = boid.m_center - boid.m_velocity;

   for (const auto& obstacle : obstacles)
   {
      const float minimumDistance = obstacle.m_radius + boidCollisionRadius;
      const glm::vec2 closestOnPath = closestPointOnSegment(previousCenter, boid.m_center, obstacle.m_center);
      glm::vec2 offset = boid.m_center - obstacle.m_center;
      float distance = glm::length(offset);
      const float pathDistance = glm::length(closestOnPath - obstacle.m_center);

      if (distance >= minimumDistance && pathDistance >= minimumDistance)
         continue;

      glm::vec2 pushDir;
      if (pathDistance < minimumDistance)
      {
         glm::vec2 pathPush = closestOnPath - obstacle.m_center;
         const float pathPushLen = glm::length(pathPush);

         if (pathPushLen > kEpsilon)
            pushDir = pathPush / pathPushLen;
         else if (distance > kEpsilon)
            pushDir = offset / distance;
         else if (!util::isZeroVector(boid.m_velocity))
            pushDir = glm::normalize(-boid.m_velocity);
         else
            pushDir = glm::vec2(1.0f, 0.0f);
      }
      else if (distance > kEpsilon)
      {
         pushDir = offset / distance;
      }
      else if (!util::isZeroVector(boid.m_velocity))
         pushDir = glm::normalize(-boid.m_velocity);
      else
         pushDir = glm::vec2(1.0f, 0.0f);

      boid.m_center = obstacle.m_center + pushDir * (minimumDistance + kObstacleNudge);

      const float retainedSpeed = std::max(glm::length(boid.m_velocity), config::MIN_SPEED * config::speedScalar);

      // Keep tangential component and only remove into-obstacle normal component.
      const float normalVel = glm::dot(boid.m_velocity, pushDir);
      if (normalVel < 0.0f)
         boid.m_velocity -= pushDir * normalVel;

      if (util::isZeroVector(boid.m_velocity))
         boid.m_velocity = pushDir * retainedSpeed;
      else
         boid.m_velocity = util::setMag(boid.m_velocity, std::min(retainedSpeed, maxSpeed));

      boid.rotate();
   }
}

glm::vec2* Flock::getBoidPosition(const int i)
{
   return &m_boids[i].m_pos[0];
}

glm::vec3* Flock::getColor()
{
   return &m_color[0];
}

void Flock::setColor(const glm::vec3& color)
{
   m_color[0] = m_color[1] = m_color[2] = color;
}

// Calculates the average velocity.
glm::vec2 Flock::alignment(const Boid& boid, const int activeBoids) const
{
   const float maxSpeed = config::MAX_SPEED * config::speedScalar;
   glm::vec2 avgVelocity = getAverageVector(boid, ALIGNMENT, activeBoids);

   if (util::isZeroVector(avgVelocity))
      return avgVelocity;

   avgVelocity = util::setMag(avgVelocity, maxSpeed);

   glm::vec2 steering = util::getSteeringVector(
         avgVelocity, boid.m_velocity
   );

   return steering;
}

// Center of mass -> avgPosition
// Calculates the average position.
glm::vec2 Flock::cohesion(const Boid& boid, const int activeBoids) const
{
   const float maxSpeed = config::MAX_SPEED * config::speedScalar;
   glm::vec2 avgPosition = getAverageVector(boid, COHESION, activeBoids);

   if (avgPosition == glm::vec2(0.0))
      return avgPosition;

   glm::vec2 toCenterOfMass = avgPosition - boid.m_center;
   toCenterOfMass = util::setMag(toCenterOfMass, maxSpeed);

   glm::vec2 steering = util::getSteeringVector(
         toCenterOfMass, boid.m_velocity
   );
   
   return steering;
}

// Keeps distance between the boids of the same specie.
glm::vec2 Flock::separation(const Boid& boid, const int activeBoids) const
{
   const float maxSpeed = config::MAX_SPEED * config::speedScalar;
   glm::vec2 avgVelocity = getAverageVector(boid, SEPARATION, activeBoids);

   if (avgVelocity == glm::vec2(0.0))
      return avgVelocity;

   avgVelocity = util::setMag(avgVelocity, maxSpeed);

   glm::vec2 steering = util::getSteeringVector(avgVelocity, boid.m_velocity);

   return steering;
}

glm::vec2 Flock::getAverageVector(const Boid& boid, const int type, const int activeBoids) const
{
   glm::vec2 avg = glm::vec2(0.0);
   int nNeighbords = 0;
   const int boidsToCheck = std::clamp(activeBoids, 0, (int)m_boids.size());

   for (int i = 0; i < boidsToCheck; i++)
   {
      const auto& other = m_boids[i];
      if (boid.m_id == other.m_id)
         continue;
      
      float range = std::abs(util::getAngle(boid.m_velocity, other.m_velocity));
      float d = glm::length(boid.m_center - other.m_center);

      if (type == ALIGNMENT)
      {
         if (d < config::ALIGNMENT_DIST && range < config::ALIGNMENT_RANGE) {
            avg += other.m_velocity;
            nNeighbords++;
         }

      } 

      if (type == COHESION)
      {
         if (d < config::COHESION_DIST && range < config::COHESION_RANGE)
         {
            avg += other.m_center;
            nNeighbords++;
         }
      }

      // ~Separation~
      if (type == SEPARATION)
      {
         if (d < config::SEPARATION_DIST && range < config::SEPARATION_RANGE)
         {
            glm::vec2 oppositeDir = boid.m_center - other.m_center;
            if (d <= kEpsilon)
               continue;
            oppositeDir /= d;

            avg += oppositeDir;
            nNeighbords++;
         }
      }

   }

   if (nNeighbords == 0)
      return avg;
   else 
      return (avg / float(nNeighbords));
}

glm::vec3 Flock::pickRandColor() const
{
   const int paletteSize = static_cast<int>(sizeof(kSpeciesPalette) / sizeof(kSpeciesPalette[0]));
   const glm::vec3 color = kSpeciesPalette[s_speciesColorIndex % paletteSize];
   s_speciesColorIndex++;
   return color;
}

Flock::~Flock() = default;
