#version 330 core
layout (location = 0) in vec3 position;
layout (location = 0) in vec3 vertexPositionInModel;
layout (location = 1) in vec3 lineInstanceColor;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 modelOffset;
layout (location = 2) in int selection;
layout (location = 2) in vec3 normal;
layout (location = 3) in int blockType;

out vec2 TexCoord;
out vec3 lineColor;
out vec4 ModelColor;
out vec3 Normal;
out vec3 FragPos;
flat out int BlockType;
flat out int IsLookedAt;
flat out int Selection;

uniform mat4 meshModel;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform bool isApp;
uniform bool isLine;
uniform bool isMesh;
uniform bool isLookedAt;
uniform bool isDynamicObject;
uniform bool isModel;
uniform int lookedAtBlockType;
uniform int appNumber;

void main()
{
  // model in this case is used per call to glDrawArraysInstanced
  if(isApp) {
    gl_Position = projection * view * model * vec4(vertexPositionInModel, 1.0);
    BlockType = appNumber;
    TexCoord = texCoord;
  } else if(isLine) {
    gl_Position = projection * view *  vec4(position, 1.0);
    lineColor = lineInstanceColor;
  } else if(isDynamicObject) {
    gl_Position = projection * view * vec4(position, 1.0);
  } else if(isMesh) {
    gl_Position = projection * view * meshModel * vec4(position, 1.0);
    BlockType = blockType;
    if (selection > 0) {
      Selection = selection;
    }
    TexCoord = texCoord;
  } else if(isModel) {
    gl_Position = projection * view * model * vec4(position, 1.0);
    FragPos = vec3(model * vec4(position, 1.0));
    TexCoord = texCoord;
    Normal = normalMatrix * normal;
  }

  IsLookedAt = 0;
  if(isLookedAt) {
    IsLookedAt = 1;
    BlockType = lookedAtBlockType;
  }
}
