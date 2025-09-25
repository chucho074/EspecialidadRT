#pragma once
#include "Prerequisites.h"
#include "MathHelpers.h"

struct SimpleVertex {
  Vector3 position;
  Vector3 color;
  Vector3 normal;
  Vector3 tangent;
  float u, v;
};

struct MeshData {
  int topology;

  int baseVertex;
  int numVertices;

  int baseIndex;
  int numIndices;

  String meshName;
};

class Model {
public:
  Model() = default;
  ~Model() = default;

  bool
  loadFromFile(const Path& inPath);

  Vector<MeshData> m_meshes;


  Vector<SimpleVertex> m_vertices;
  Vector<unsigned int> m_indices;
};