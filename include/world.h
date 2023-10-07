#ifndef __WORLD_H__
#define __WORLD_H__

#include "app.h"
#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <octree/octree.h>
#include <unordered_map>
#include <vector>

class Renderer;

struct Position {
  int x;
  int y;
  int z;
  bool valid;
  glm::vec3 normal;
};

struct Cube {
  glm::vec3 position;
  int blockType;
  bool operator==(const Cube& cmp);
};

struct App {
  X11App* app;
  glm::vec3 position;
};

enum Action {
  PLACE_CUBE,
  REMOVE_CUBE
};

class World {
  Renderer *renderer = NULL;
  Camera *camera = NULL;

  int gotItCount = 0;
  int CHUNK_SIZE = 128;
  int cubeCount = 0;
  std::unordered_map<glm::vec3, int> appCubes;
  std::vector<X11App*> apps;
  Octree<Cube> cubes = Octree<Cube>(128, Cube{glm::vec3(0, 0, 0), -1});
  glm::vec3 cameraToVoxelSpace(glm::vec3 cameraPosition);
  Cube *getCube(float x, float y, float z);
  const std::vector<Cube> getCubes();
  void refreshRenderer();

public:
  const float CUBE_SIZE = 0.1;
  World(Camera *camera, bool debug = false);
  ~World();
  void attachRenderer(Renderer *renderer);

  X11App *getLookedAtApp();
  Position getLookedAtCube();
  const std::vector<glm::vec3> getAppCubes();

  void addCube(int x, int y, int z, int blockType);
  void removeCube(int x, int y, int z);
  void addApp(glm::vec3, X11App* app);
  int getIndexOfApp(X11App* app);
  int size();

  void action(Action);
};

#endif
