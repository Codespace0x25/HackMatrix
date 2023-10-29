#include "api.h"
#include "app.h"
#include "camera.h"
#include "controls.h"
#include "renderer.h"
#include "wm.h"
#include "world.h"

#include <GLFW/glfw3.h>

void mouseCallback(GLFWwindow *window, double xpos, double ypos);

class Engine {

  World *world;
  Api *api;
  Renderer *renderer;
  Controls *controls;
  Camera *camera;
  WM *wm;
  GLFWwindow *window;

  friend void mouseCallback(GLFWwindow *window, double xpos, double ypos);

public:
  Engine(GLFWwindow* window, char** envp);
  ~Engine();
  void initialize();
  void wire();
  void loop();
  void registerCursorCallback();
};