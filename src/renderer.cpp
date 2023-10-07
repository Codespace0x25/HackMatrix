#include "texture.h"
#include "renderer.h"
#include "shader.h"
#include "camera.h"
#include "app.h"
#include <vector>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float HEIGHT = 0.27;

float appVertices[] = {
  -0.5f, -HEIGHT, -0.5f, 0.0f, 0.0f,
  0.5f, -HEIGHT, -0.5f, 1.0f, 0.0f,
  0.5f, HEIGHT, -0.5f, 1.0f, 1.0f,
  0.5f, HEIGHT, -0.5f, 1.0f, 1.0f,
  -0.5f, HEIGHT, -0.5f, 0.0f, 1.0f,
  -0.5f, -HEIGHT, -0.5f, 0.0f, 0.0f,
};

float vertices[] = {
  -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
  0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
  0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
  0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
  -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
  -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
  0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
  0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
  0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
  -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
  -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
  -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
  -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
  -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
  -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
  -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
  0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
  0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
  0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
  0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
  0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
  0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
  -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
  0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
  0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
  0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
  -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
  -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
  -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
  0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
  0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
  0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
  -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
  -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
};

void Renderer::genGlResources() {
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &APP_VBO);
  glGenBuffers(1, &INSTANCE);
  glGenBuffers(1, &APP_INSTANCE);
  glGenVertexArrays(1, &VAO);
  glGenVertexArrays(1, &APP_VAO);
}

void Renderer::bindGlResourcesForInit() {
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
}

void Renderer::setupVertexAttributePointers() {
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // texture coord attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // instance coord attribute
  glBindBuffer(GL_ARRAY_BUFFER, INSTANCE);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (3 * sizeof(float)) + (1 * sizeof(int)), (void*)0);
  glEnableVertexAttribArray(2);
  glVertexAttribDivisor(2, 1);

  // instance texture attribute
  glBindBuffer(GL_ARRAY_BUFFER, INSTANCE);
  glVertexAttribIPointer(3, 1, GL_INT, (3 * sizeof(float)) + (1 * sizeof(int)), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(3);
  glVertexAttribDivisor(3, 1);

  glBindVertexArray(APP_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, APP_VBO);
  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // texture coord attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // instance coord attribute
  glBindBuffer(GL_ARRAY_BUFFER, APP_INSTANCE);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (3 * sizeof(float)) + (1 * sizeof(int)), (void*)0);
  glEnableVertexAttribArray(2);
  glVertexAttribDivisor(2, 1);

  // instance app number attribute
  glBindBuffer(GL_ARRAY_BUFFER, APP_INSTANCE);
  glVertexAttribIPointer(3, 1, GL_INT, (3 * sizeof(float)) + (1 * sizeof(int)),
                         (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(3);
  glVertexAttribDivisor(3, 1);
}

void Renderer::fillBuffers() {
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, APP_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(appVertices), appVertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, INSTANCE);
  glBufferData(GL_ARRAY_BUFFER, (sizeof(glm::vec3) + sizeof(int)) * 200000, (void*)0 , GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, APP_INSTANCE);
  glBufferData(GL_ARRAY_BUFFER, (sizeof(glm::vec3) + sizeof(int)) * 20, (void*)0 , GL_STATIC_DRAW);
}


Renderer::Renderer(Camera* camera, World* world) {
  this->camera = camera;
  this->world = world;
  glEnable(GL_DEPTH_TEST);
  genGlResources();
  bindGlResourcesForInit();
  fillBuffers();
  setupVertexAttributePointers();

  std::vector<std::string> images = {
    "images/bAndGrey.png",
    "images/purpleRoad.png",
    "images/bAndGreySpeckled.png",
    "images/grass.png",
    "images/pillar.png"
  };
  textures.insert(std::pair<string,Texture*>("container", new Texture(images, GL_TEXTURE0)));
  textures.insert(std::pair<string, Texture*>("face",
                                              new Texture("images/awesomeface.png", GL_TEXTURE1)));
  textures.insert(std::pair<string, Texture*>("app0", new Texture(GL_TEXTURE31)));
  textures.insert(std::pair<string, Texture *>("app1", new Texture(GL_TEXTURE30)));
  textures.insert(std::pair<string, Texture *>("app2", new Texture(GL_TEXTURE29)));

  shader = new Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
  shader->use(); // may need to move into loop to use changing uniforms
  shader->setInt("texture1", 0);
  shader->setInt("totalBlockTypes", images.size());
  shader->setInt("texture2", 1);
  shader->setInt("app0", 31);
  shader->setInt("app1", 30);
  shader->setInt("app2", 29);

  shader->setBool("selectedValid", false);
  shader->setInt("selectedX", 0);
  shader->setInt("selectedY", 0);
  shader->setInt("selectedZ", 0);

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //  normal
  glClearColor(178.0/256, 178.0/256, 178.0/256, 1.0f);

  //projection = glm::ortho(0.0f, 1920.0f, 0.0f, 1080.0f, 0.1f, 100.0f);


  view = glm::mat4(1.0f);
  // note that we’re translating the scene in the reverse direction
  view = glm::translate(view, glm::vec3(0.0f, 0.0f, 3.0f));

  projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

  // this can be used to rotate an entire chunk (call to glDrawArraysInstanced)
  model = glm::mat4(1.0f);
  model = glm::scale(model, glm::vec3(world->CUBE_SIZE));

  appModel = glm::mat4(1.0f);
  //model = glm::rotate(model, glm::radians(-55.0f),
  //                   glm::vec3(1.0f, 0.0f, 0.0f));

}

void Renderer::updateTransformMatrices() {
  unsigned int modelLoc = glGetUniformLocation(shader->ID,"model");
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


  unsigned int appModelLoc = glGetUniformLocation(shader->ID,"appModel");
  glUniformMatrix4fv(appModelLoc, 1, GL_FALSE, glm::value_ptr(appModel));

  unsigned int viewLoc = glGetUniformLocation(shader->ID,"view");
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

  unsigned int projectionLoc = glGetUniformLocation(shader->ID,"projection");
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void Renderer::addCube(int index, Cube cube) {
  glBindBuffer(GL_ARRAY_BUFFER, INSTANCE);
  glBufferSubData(GL_ARRAY_BUFFER,
                  (sizeof(glm::vec3)+sizeof(int))*index, sizeof(glm::vec3), &cube.position);
  glBufferSubData(GL_ARRAY_BUFFER,
                  sizeof(glm::vec3)*(index+1)+sizeof(int)*index, sizeof(int), &cube.blockType);
}

void Renderer::addAppCube(int index, glm::vec3 pos) {
  cout << "pos:" << pos.x << "," << pos.y << ", " << pos.z << endl;
  glBindBuffer(GL_ARRAY_BUFFER, APP_INSTANCE);
  glBufferSubData(GL_ARRAY_BUFFER, (sizeof(glm::vec3) + sizeof(int)) * index,
                  sizeof(glm::vec3), &pos);
  glBufferSubData(GL_ARRAY_BUFFER,
                  sizeof(glm::vec3)*(index+1)+sizeof(int)*index, sizeof(int), &index);
}

void Renderer::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  view = camera->getViewMatrix();

  // handleSelected
  Position selected = world->getLookedAtCube();
  if(selected.valid) {
    shader->setBool("selectedValid", true);
    glm::vec3 selectedVec = glm::vec3(selected.x, selected.y, selected.z);
    unsigned int selectedLoc = glGetUniformLocation(shader->ID,"selected");
    glUniform3fv(selectedLoc, 1, glm::value_ptr(selectedVec));
  } else {
    shader->setBool("selectedValid", false);
  }

  X11App* app = world->getLookedAtApp();

  updateTransformMatrices();
  shader->use(); // may need to move into loop to use changing uniforms
  shader->setFloat("time", glfwGetTime());
  shader->setBool("isApp", false);
  if(app != NULL) {
    shader->setBool("appSelected", app->isFocused);
  }
  glBindVertexArray(VAO);
  glDrawArraysInstanced(GL_TRIANGLES, 0, 36, world->size());
  shader->setBool("isApp", true);
  glBindVertexArray(APP_VAO);
  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, world->getAppCubes().size());
  if(app != NULL && app->isFocused) {
    int index = world->getIndexOfApp(app);
    if(index >= 0) {
      glBlitNamedFramebuffer(frameBuffers[index], 0,
                           // start x, start y (flip)
                           0, 1080,
                           // end x, end y (flip)
                           1920, 0, 0, 0, 1920, 1080, GL_COLOR_BUFFER_BIT,
                           GL_NEAREST);
    }
  }
}

Camera* Renderer::getCamera() {
  return camera;
}

void Renderer::registerApp(X11App* app, int index) {
  int textureN = 31 - index;
  int texId = GL_TEXTURE0 + textureN;
  glActiveTexture(texId);
  glBindTexture(GL_TEXTURE_2D, textures["app" + to_string(index)]->ID);
  app->appTexture();

  unsigned int framebufferId;
  glGenFramebuffers(1, &framebufferId);
  frameBuffers.push_back(framebufferId);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferId);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, textures["app" + to_string(index)]->ID,
                         0);
}

Renderer::~Renderer() {
  delete shader;
  for(auto& t: textures) {
    delete t.second;
  }
}
