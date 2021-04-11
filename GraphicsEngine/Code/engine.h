//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include <glad/glad.h>

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

struct Vao
{
    GLuint handle;
    GLuint programHandle;
};

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct VertexV3V2
{
    glm::vec3 pos;
    glm::vec2 uv;
};

struct VertexBufferAttribute
{
    u8 location;
    u8 componentCount;
    u8 offset;
};

struct VertexBufferLayout
{
    u8 stride;
    std::vector<VertexBufferAttribute> attributes;
};

struct VertexShaderAttribute
{
    u8 location;
    u8 componentCount;
};

struct VertexShaderLayout
{
    std::vector<VertexShaderAttribute> attributes;
};

struct Material
{
    std::string name;
    vec3 albedo;
    vec3 emissive;
    f32 smoothness;
    u32 albedoTextureIdx;
    u32 emissiveTextureIdx;
    u32 specularTextureIdx;
    u32 normalsTextureIdx;
    u32 bumpTextureIdx;
};

struct Submesh
{
    u32 vertexOffset;
    u32 indexOffset;
    std::vector<float>  vertices;
    std::vector<u32>    indices;
    std::vector<Vao>    vaos;
    VertexBufferLayout  vertexBufferLayout;
};

struct Mesh
{
    GLuint vertexBufferHandle;
    GLuint indexBufferHandle;
    std::vector<Submesh> submeshes;
};

struct Model
{
    u32 meshIdx;
    std::vector<u32> materialIdx;
};

struct Program
{
    GLuint              handle;
    std::string         filepath;
    std::string         programName;
    u64                 lastWriteTimestamp;
    VertexShaderLayout  vertexInputLayout;
};

enum class Mode
{
    TexturedQuad,
    TexturedMesh,
    Count
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;
    bool enableDebugGroups = false;

    // Input
    Input input;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    ivec2 displaySize;

    std::vector<Texture>  textures;
    std::vector<Material> materials;
    std::vector<Mesh>     meshes;
    std::vector<Model>    models;
    std::vector<Program>  programs;

    // Texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    // Model indices
    u32 model;

    // Program indices
    u32 texturedGeometryProgramIdx;
    u32 texturedMeshProgramIdx;

    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;
    GLuint programMeshTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;

    // OpenGL information
    std::vector<std::string> info;
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

u32 LoadTexture2D(App* app, const char* filepath);