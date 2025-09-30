//
// Created by code on 9/22/25.
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Camera.h"

struct AABB {
  glm::vec3 min;
  glm::vec3 max;

  AABB(glm::vec3 _min, glm::vec3 _max) : min(_min), max(_max) {}
  AABB() {}

  bool intersects(const AABB& other) const {
    return (min.x <= other.max.x && max.x >= other.min.x) &&
           (min.y <= other.max.y && max.y >= other.min.y) &&
           (min.z <= other.max.z && max.z >= other.min.z);
  }

  // Optional: point check (if you want)
  bool contains(const glm::vec3& point) const {
    return (point.x >= min.x && point.x <= max.x) &&
           (point.y >= min.y && point.y <= max.y) &&
           (point.z >= min.z && point.z <= max.z);
  }

  // translate AABB by vector
  AABB& operator+=(const glm::vec3& rhs) {
    min += rhs;
    max += rhs;
    return *this;
  }

  AABB& operator-=(const glm::vec3& rhs) {
    min -= rhs;
    max -= rhs;
    return *this;
  }

  // uniform scaling
  AABB& operator*=(float s) {
    min *= s;
    max *= s;
    return *this;
  }

  // comparisons
  bool operator==(const AABB& rhs) const {
    return min == rhs.min && max == rhs.max;
  }

  bool operator!=(const AABB& rhs) const {
    return !(*this == rhs);
  }
};

// free functions (non-mutating versions)
inline AABB operator+(AABB lhs, const glm::vec3& rhs) {
  lhs += rhs;
  return lhs;
}

inline AABB operator-(AABB lhs, const glm::vec3& rhs) {
  lhs -= rhs;
  return lhs;
}

inline AABB operator*(AABB lhs, float s) {
  lhs *= s;
  return lhs;
}


class GameObject {
public:
  glm::vec3 position {0.0f, 0.0f, 0.0f};
  glm::vec3 rotation {0.0f, 0.0f, 0.0f};
  glm::vec3 scale    {1.0f, 1.0f, 1.0f};

  virtual ~GameObject() = default;

  virtual void Update(float dt);

  virtual void Draw(Camera& camera) const;

  glm::mat4 getModelMatrix() const;
  AABB getBoundingBox() const;


};



#endif //GAMEOBJECT_H
