//
// Created by code on 9/22/25.
//

#ifndef MESHOBJECT_H
#define MESHOBJECT_H

#include <memory>

#include "GameObject.h"
#include "../Mesh.h"
#include "../Shader.h"


class MeshObject : public GameObject {
public:
  std::unique_ptr<Mesh> mesh;
  std::shared_ptr<Shader>& shader;

  MeshObject(
    std::unique_ptr<Mesh>&& m,
    std::shared_ptr<Shader>& shader);

  void Update(float dt) override;

  void Draw(Camera& camera) const override;
};



#endif //MESHOBJECT_H
