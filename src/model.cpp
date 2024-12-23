#include "model.h"
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>
#include <sstream>
#include "components/BoundingSphere.h"
#include "glm/trigonometric.hpp"
#include "persister.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "glm/ext/matrix_transform.hpp"
#include <glm/gtc/quaternion.hpp>

using namespace std;

void
Model::Draw(Shader& shader)
{
  for (unsigned int i = 0; i < meshes.size(); i++)
    meshes[i].Draw(shader);
}

void
Model::loadModel(string path)
{
  Assimp::Importer import;
  const aiScene* scene =
    import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
    return;
  }
  directory = path.substr(0, path.find_last_of('/'));

  processNode(scene->mRootNode, scene);
}

void
Model::processNode(aiNode* node, const aiScene* scene)
{
  // process all the node's meshes (if any)
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene));
  }
  // then do the same for each of its children
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}

Mesh
Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  vector<MeshTexture> textures;

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    // process vertex positions, normals and texture coordinates
    glm::vec3 vector;
    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    vertex.Position = vector;

    vector.x = mesh->mNormals[i].x;
    vector.y = mesh->mNormals[i].y;
    vector.z = mesh->mNormals[i].z;
    vertex.Normal = vector;

    if (mesh->mTextureCoords[0]) {
      glm::vec2 vec;
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.TexCoords = vec;
    } else
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);

    vertices.push_back(vertex);
  }

  // process indices
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }

  if (mesh->mMaterialIndex >= 0) {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    vector<MeshTexture> diffuseMaps =
      loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    vector<MeshTexture> specularMaps = loadMaterialTextures(
      material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  }

  return Mesh(vertices, indices, textures);
}

vector<Vertex>
Model::getAllVertices()
{
  vector<Vertex> allVertices;

  for (const Mesh& mesh : meshes) {
    allVertices.insert(
      allVertices.end(), mesh.vertices.begin(), mesh.vertices.end());
  }

  return allVertices;
}

vector<MeshTexture>
Model::loadMaterialTextures(aiMaterial* mat,
                            aiTextureType type,
                            string typeName)
{
  vector<MeshTexture> textures;
  cout << "loading material textures" << endl;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    cout << "loading texture " << i << endl;
    aiString str;
    mat->GetTexture(type, i, &str);
    bool skip = false;
    for (unsigned int j = 0; j < textures_loaded.size(); j++) {
      if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
        textures.push_back(textures_loaded[j]);
        skip = true;
        break;
      }
    }
    if (!skip) { // if texture hasn't been loaded already, load it
      MeshTexture texture;
      texture.id = TextureFromFile(str.C_Str(), directory);
      texture.type = typeName;
      texture.path = str.C_Str();
      textures.push_back(texture);
      textures_loaded.push_back(texture); // add to loaded textures
    }
  }
  return textures;
}

BoundingSphere
Model::getBoundingSphere(float scale)
{
  auto vertices = getAllVertices();

  if (vertices.empty()) {
    // Handle the case of an empty mesh
    return BoundingSphere{ glm::vec3(0.0f), 0.0f };
  }

  // 1. Find bounding box (same as before):
  glm::vec3 minBounds(std::numeric_limits<float>::max());
  glm::vec3 maxBounds(-std::numeric_limits<float>::max());

  for (const Vertex& vertex : vertices) {
    glm::vec3 scaledPosition = vertex.Position * scale;
    minBounds = glm::min(minBounds, scaledPosition);
    maxBounds = glm::max(maxBounds, scaledPosition);
  }

  // 2. Calculate center:
  glm::vec3 center = (minBounds + maxBounds) * 0.5f;

  // 3. Find the radius:
  float radius = 0.0f;
  for (const Vertex& vertex : vertices) {
    glm::vec3 scaledPosition = vertex.Position * scale;
    float distance = glm::distance(scaledPosition, center);
    radius = std::max(radius, distance);
  }

  return BoundingSphere{ center, radius };
}

unsigned int
TextureFromFile(const char* path, const string& directory, bool gamma)
{
  string filename = string(path);
  filename = directory + '/' + filename;

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  stbi_set_flip_vertically_on_load(false);
  unsigned char* data =
    stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
  stbi_set_flip_vertically_on_load(true);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 format,
                 width,
                 height,
                 0,
                 format,
                 GL_UNSIGNED_BYTE,
                 data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(
      GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

Model::Model(string path)
  : path(path)
{
  loadModel(path);
}

void
Positionable::update()
{
  modelMatrix = glm::mat4(1.0f);
  modelMatrix = glm::translate(modelMatrix, pos);

  // Create individual rotation quaternions
  glm::quat finalRotation = glm::quat(glm::radians(rotate));

  modelMatrix = modelMatrix * glm::mat4_cast(finalRotation);
  modelMatrix = glm::translate(modelMatrix, origin * glm::vec3(-1));
  modelMatrix = glm::scale(modelMatrix, glm::vec3(scale, scale, scale));
  glm::mat4 inverseModelMatrix = glm::inverse(modelMatrix);
  glm::mat4 transposedInverse = glm::transpose(inverseModelMatrix);
  normalMatrix = glm::mat3(transposedInverse);
  damaged = false;
}

void
Positionable::damage()
{
  damaged = true;
}

Positionable::Positionable(glm::vec3 pos,
                           glm::vec3 origin,
                           glm::vec3 rotate,
                           float scale)
  : pos(pos)
  , origin(origin)
  , scale(scale)
  , rotate(rotate)
{
}

Positionable::Positionable(Positionable* p) {
  Positionable(p->pos, p->origin, p->rotate, p->scale);
}

void
PositionablePersister::createTablesIfNeeded()
{
  stringstream queryStream;
  queryStream << "CREATE TABLE IF NOT EXISTS " << entityName << " ("
              << "entity_id INTEGER PRIMARY KEY, "
              << "pos_x REAL, pos_y REAL, pos_z REAL, "
              << "origin_x REAL, origin_y REAL, origin_z REAL, scale REAL, "
              << "rot_x REAL, rot_y REAL, rot_z REAL, "
              << "FOREIGN KEY(entity_id) REFERENCES Entity(id))";
  registry->getDatabase().exec(queryStream.str());
}

void
PositionablePersister::save(entt::entity entity)
{
  auto& pos = registry->get<Positionable>(entity);
  auto& persistable = registry->get<Persistable>(entity);
  auto& db = registry->getDatabase();
  stringstream queryStream;
  queryStream << "INSERT OR REPLACE INTO " << entityName
              << " (entity_id, pos_x, pos_y, pos_z, "
              << "origin_x, origin_y, origin_z, "
              << "rot_x, rot_y, rot_z, scale)"
              << " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
  SQLite::Statement query(db, queryStream.str());
  query.bind(1, persistable.entityId);
  query.bind(2, pos.pos.x);
  query.bind(3, pos.pos.y);
  query.bind(4, pos.pos.z);
  query.bind(5, pos.origin.x);
  query.bind(6, pos.origin.y);
  query.bind(7, pos.origin.z);
  query.bind(8, pos.rotate.x);
  query.bind(9, pos.rotate.y);
  query.bind(10, pos.rotate.z);
  query.bind(11, pos.scale);
  query.exec();
}

void
PositionablePersister::saveAll()
{
  auto view = registry->view<Persistable, Positionable>();

  SQLite::Database& db = registry->getDatabase(); // Get database reference

  stringstream queryStream;
  queryStream << "INSERT OR REPLACE INTO " << entityName
              << " (entity_id, pos_x, pos_y, pos_z, "
              << "origin_x, origin_y, origin_z, "
              << "rot_x, rot_y, rot_z, scale)"
              << " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
  SQLite::Statement query(db, queryStream.str());
  // Use a transaction for efficiency
  db.exec("BEGIN TRANSACTION");

  for (auto [entity, persist, positionable] : view.each()) {
    query.bind(1, persist.entityId);
    query.bind(2, positionable.pos.x);
    query.bind(3, positionable.pos.y);
    query.bind(4, positionable.pos.z);
    query.bind(5, positionable.origin.x);
    query.bind(6, positionable.origin.y);
    query.bind(7, positionable.origin.z);
    query.bind(8, positionable.rotate.x);
    query.bind(9, positionable.rotate.y);
    query.bind(10, positionable.rotate.z);
    query.bind(11, positionable.scale);
    query.exec();
    query.reset(); // Important for reusing the prepared statement
  }

  db.exec("COMMIT");
}

void
PositionablePersister::load(entt::entity entity)
{
  auto peristable = registry->get<Persistable>(entity);
  std::stringstream queryStream;
  queryStream << "SELECT pos_x, pos_y, pos_z, "
              << "origin_x, origin_y, origin_z, "
              << "rot_x, rot_y, rot_z, scale FROM " << entityName
              << " WHERE entity_id = ?";
  SQLite::Statement query(registry->getDatabase(), queryStream.str());
  query.bind(1, peristable.entityId);

  if (query.executeStep()) {
    float x = query.getColumn(0).getDouble();
    float y = query.getColumn(1).getDouble();
    float z = query.getColumn(2).getDouble();
    float origin_x = query.getColumn(3).getDouble();
    float origin_y = query.getColumn(4).getDouble();
    float origin_z = query.getColumn(5).getDouble();
    float rotx = query.getColumn(6).getDouble();
    float roty = query.getColumn(7).getDouble();
    float rotz = query.getColumn(8).getDouble();
    float scale = query.getColumn(9).getDouble();

    registry->emplace<Positionable>(entity,
                                    glm::vec3(x, y, z),
                                    glm::vec3(origin_x, origin_y, origin_z),
                                    glm::vec3(rotx, roty, rotz),
                                    scale);
  }
}

void
PositionablePersister::loadAll()
{
  auto view = registry->view<Persistable>();
  SQLite::Database& db = registry->getDatabase();

  // Cache query data
  std::unordered_map<int, std::array<float, 10>> positionDataCache;

  std::stringstream queryStream;
  queryStream << "SELECT entity_id, pos_x, pos_y, pos_z, "
              << "origin_x, origin_y, origin_z, "
              << "rot_x, rot_y, rot_z, scale FROM " << entityName;
  SQLite::Statement query(db, queryStream.str());
  while (query.executeStep()) {
    int dbId = query.getColumn(0).getInt();
    float x = query.getColumn(1).getDouble();
    float y = query.getColumn(2).getDouble();
    float z = query.getColumn(3).getDouble();
    float origin_x = query.getColumn(4).getDouble();
    float origin_y = query.getColumn(5).getDouble();
    float origin_z = query.getColumn(6).getDouble();
    float rotx = query.getColumn(7).getDouble();
    float roty = query.getColumn(8).getDouble();
    float rotz = query.getColumn(9).getDouble();
    float scale = query.getColumn(10).getDouble();

    std::array<float, 10> data = {
      x, y, z, origin_x, origin_y, origin_z, rotx, roty, rotz, scale
    }; // Using temporaries
    positionDataCache[dbId] = data;
  }

  // Iterate and emplace
  for (auto [entity, persistable] : view.each()) {
    auto it = positionDataCache.find(persistable.entityId);
    if (it != positionDataCache.end()) {
      auto& [x, y, z, originX, originY, originZ, rotx, roty, rotz, scale] =
        it->second;
      registry->emplace<Positionable>(entity,
                                      glm::vec3(x, y, z),
                                      glm::vec3(originX, originY, originZ),
                                      glm::vec3(rotx, roty, rotz),
                                      scale);
    }
  }
}

void
PositionablePersister::depersistIfGone(entt::entity entity)
{
  depersistIfGoneTyped<Positionable>(entity);
}

void
ModelPersister::saveAll()
{
  auto view = registry->view<Persistable, Model>();
  SQLite::Database& db = registry->getDatabase();

  // Assuming you have a 'Model' table with 'entity_id' and 'path' columns
  std::stringstream queryStream;
  queryStream << "INSERT OR REPLACE INTO " << entityName
              << " (entity_id, path) VALUES (?, ?)";
  SQLite::Statement query(db, queryStream.str());

  db.exec("BEGIN TRANSACTION"); // Initiate transaction

  for (auto [entity, persist, model] : view.each()) {
    query.bind(1, persist.entityId);
    query.bind(2, model.path);
    query.exec();
    query.reset();
  }

  db.exec("COMMIT"); // Commit changes
}

void
ModelPersister::createTablesIfNeeded()
{
  SQLite::Database& db = registry->getDatabase();
  std::stringstream queryStream;
  queryStream << "CREATE TABLE IF NOT EXISTS " << entityName << " ("
              << "entity_id INTEGER PRIMARY KEY, "
              << "path TEXT, "
              << "FOREIGN KEY(entity_id) REFERENCES Entity(id)) ";
  db.exec(queryStream.str());
}

void
ModelPersister::loadAll()
{
  auto view = registry->view<Persistable>();
  SQLite::Database& db = registry->getDatabase();

  // Cache query data
  std::unordered_map<int, std::string> modelDataCache;
  std::stringstream queryStream;
  queryStream << "SELECT entity_id, path FROM " << entityName;
  SQLite::Statement query(db, queryStream.str());

  while (query.executeStep()) {
    int dbId = query.getColumn(0).getInt();
    std::string path = query.getColumn(1).getText();
    modelDataCache[dbId] = path;
  }

  // Iterate and load (assuming you have a mechanism to load Models from paths)
  view.each([&modelDataCache, this](auto entity, auto& persistable) {
    auto it = modelDataCache.find(persistable.entityId);
    if (it != modelDataCache.end()) {
      std::string& path = it->second;
      std::unique_ptr<Model> model = make_unique<Model>(path);
      if (model) {
        registry->emplace<Model>(entity, std::move(*model));
      }
    }
  });
}

void
ModelPersister::load(entt::entity entity)
{
  auto& persistable = registry->get<Persistable>(entity);
  SQLite::Database& db = registry->getDatabase();

  std::stringstream queryStream;
  queryStream << "SELECT path FROM " << entityName << " WHERE entity_id = ?";
  SQLite::Statement query(db, queryStream.str());
  query.bind(1, persistable.entityId);

  if (query.executeStep()) {
    std::string path = query.getColumn(0).getText();

    std::unique_ptr<Model> model = make_unique<Model>(path);
    if (model) {
      registry->emplace<Model>(entity, std::move(*model));
    } else {
      // Log or handle model loading error
    }
  }
}

void
ModelPersister::save(entt::entity entity)
{
  auto& model = registry->get<Model>(entity);
  auto& persistable = registry->get<Persistable>(entity);
  SQLite::Database& db = registry->getDatabase();

  std::stringstream queryStream;
  queryStream << "INSERT OR REPLACE INTO " << entityName
              << " (entity_id, path) VALUES (?, ?)";
  SQLite::Statement query(db, queryStream.str());
  query.bind(1, persistable.entityId);
  query.bind(2, model.path);
  query.exec();
}

void
ModelPersister::depersistIfGone(entt::entity entity)
{
  depersistIfGoneTyped<Model>(entity);
}
