#include "world.h"
#include "renderer.h"
#include <vector>
#include <glm/glm.hpp>
#include <octree/octree.h>
#include <iostream>

using namespace std;



World::World(){}
World::~World() {}

const std::vector<Cube> World::getCubes() {
  std::vector<Cube> rv(cubes.size());
  for(int x=0; x<CHUNK_SIZE; x++) {
    for(int y=0; y<CHUNK_SIZE; y++) {
      for(int z=0; z<CHUNK_SIZE; z++) {
        Cube cube = cubes(x,y,z);
        if(cube.blockType != -1) {
          rv[cube.order] = cube;
        }
      }
    }
  }
  return rv;
}

const std::vector<glm::vec3> World::getAppCubes() {
  std::vector<glm::vec3> appCubeKeys(appCubes.size());
  for(auto kv: appCubes) {
    appCubeKeys[kv.second] = kv.first;
  }
  return appCubeKeys;
}

void World::addCube(int x, int y, int z, int blockType) {
  int orderIndex = cubeCount++;
  glm::vec3 pos(x, y, z);
  Cube cube{pos, blockType, orderIndex};
  cubes(x,y,z) = cube;
  if(renderer != NULL) {
    renderer->addCube(orderIndex, cube);
  }
}

void World::addAppCube(glm::vec3 cube) {
  int index = appCubes.size();
  appCubes.insert(std::pair<glm::vec3, int>(cube, index));
  if(renderer != NULL) {
    renderer->addAppCube(index);
  }
}

void World::attachRenderer(Renderer* renderer){
  this->renderer = renderer;
}

Cube* World::getVoxel(float x, float y, float z) {
  Cube* rv = &cubes(x,y,z);
  if(rv->blockType != -1) {
    return rv;
  }
  return NULL;
}

int World::size() {
  return cubeCount;
}


glm::vec3 World::cameraToVoxelSpace(glm::vec3 cameraPosition) {
  glm::vec3 rv = cameraPosition / glm::vec3(0.1,0.1,0.1);
  cout << rv.x << ", " << rv.y << ", " << rv.z << endl;
  return rv;
}


Position World::rayCast(Camera* camera) {
  Position rv;
  glm::vec3 voxelSpace = cameraToVoxelSpace(camera->position);
  int x = (int)voxelSpace.x;
  int y = (int)voxelSpace.y;
  int z = (int)voxelSpace.z;

  int stepX = ( camera->front.x > 0) ? 1 : -1;
  int stepY = ( camera->front.y > 0) ? 1 : -1;
  int stepZ = ( camera->front.z > 0) ? 1 : -1;

  cout << stepX << "," << stepY << "," << stepZ << endl;
  return rv;
}
