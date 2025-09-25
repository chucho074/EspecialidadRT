#include "Model.h"

struct FaceVertex {
  int vertex_index = -1;
  int uv_index = -1;
  int normal_index = -1;

  bool
    operator==(const FaceVertex& vertex) const {
    return vertex_index == vertex.vertex_index
      && uv_index == vertex.uv_index
      && normal_index == vertex.normal_index;
  }
};

namespace std {
  template<>
  struct hash<FaceVertex> {
    size_t
      operator()(const FaceVertex& vertex) const {
      return hash<int>()(vertex.vertex_index)
        ^ hash<int>()(vertex.uv_index);
    }
  };
}

bool
Model::loadFromFile(const Path& inPath) {


  fstream objFile(inPath, ios::in | ios::ate);
  if(!objFile.is_open()) {
    return false;
  }

  auto fileSize = objFile.tellg();
  objFile.seekp(ios::beg);

  String fileData;
  fileData.resize(fileSize);
  objFile.read(&fileData[0], fileSize);

  Vector<String> lines = split(fileData, '\n');
  //Vector<SimpleVertex> vertices;
  //Vector<uint32> indices;

  Vector<Vector3> temp_pos;
  Vector<Vector2> temp_tc;
  Vector<Vector3> temp_norm;
  UMap<FaceVertex, unsigned int> uniqueVertices;

  int vt_index = 0;
  for(const auto& line : lines) {
    Vector<String> tokens = split(line, ' ');
    if(tokens.empty()) {
      continue;
    }

    if(tokens[0] == "#") {
      continue;
    }
    if(tokens[0] == "v") {
      Vector3 pos;
      pos.x = std::stof(tokens[1]);
      pos.y = std::stof(tokens[2]);
      pos.z = std::stof(tokens[3]);

      temp_pos.push_back(pos);
    }

    else if(tokens[0] == "vt") {
      Vector2 uv;
      uv.x = std::stof(tokens[1]);
      uv.y = std::stof(tokens[2]);
      temp_tc.push_back(uv);
    }
    else if(tokens[0] == "vn") {
      Vector3 normal;
      normal.x = std::stof(tokens[1]);
      normal.y = std::stof(tokens[2]);
      normal.z = std::stof(tokens[3]);
      temp_norm.push_back(normal);
    }

    else if(tokens[0] == "f") {
      Vector<unsigned int> faceIndex;

      //assert(tokens.size() == 4);
      for(size_t i = 1; i < tokens.size(); ++i) {
        Vector<String> fi = split(tokens[i], '/');

        FaceVertex fv;

        fv.vertex_index = std::stoi(fi[0]) - 1;
        fv.uv_index = std::stoi(fi[1]) - 1;
        fv.normal_index = std::stoi(fi[2]) - 1;

        if(uniqueVertices.find(fv) == uniqueVertices.end()) {
          uniqueVertices[fv] = static_cast<unsigned int>(m_vertices.size());

          SimpleVertex mvertex;
          mvertex.position = temp_pos[fv.vertex_index];
          mvertex.color = Vector3(1.f, 1.f, 1.f);
          mvertex.normal = temp_norm[fv.normal_index];
          mvertex.u = temp_tc[fv.uv_index].x;
          mvertex.v = 1.0 - temp_tc[fv.uv_index].y;

          m_vertices.push_back(mvertex);
        }

        faceIndex.push_back(uniqueVertices[fv]);
      }

      m_indices.push_back(faceIndex[0]);
      m_indices.push_back(faceIndex[1]);
      m_indices.push_back(faceIndex[2]);
    }
  }
  objFile.close();
  m_meshes.resize(1);
  auto& mesh = m_meshes[0];
  mesh.baseVertex = 0;
  mesh.numVertices = m_vertices.size();

  mesh.baseIndex = 0;
  mesh.numIndices = m_indices.size();

  mesh.topology = 4;

  return true;
}

