//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "assimp.h"
#include "buffers.h"

#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>

GLuint CreateProgramFromSource(App* app, String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(vertexShaderDefine),
        (GLint) programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(fragmentShaderDefine),
        (GLint) programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
        std::string output = "\nFail with vertex shader:\n - " + (std::string)infoLogBuffer;
        app->info.push_back(output);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
        std::string output = "\nFail with fragment shader:\n - " + (std::string)infoLogBuffer;
        app->info.push_back(output);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(app, programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);

    // Fill input vertex shader
    GLint attributeCount;
    glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

    for (u32 i = 0; i < attributeCount; ++i)
    {
        const GLsizei bufSize = 16; // maximum name length
        GLchar attributeName[bufSize]; // variable name in GLSL
        GLsizei attributeNameLength; // name length

        GLint attributeSize; // size of the variable
        GLenum attributeType; // type of the variable (float, vec3 or mat4, etc)

        glGetActiveAttrib(program.handle, i, bufSize, &attributeNameLength, &attributeSize, &attributeType, attributeName);

        u8 attributeLocation = glGetAttribLocation(program.handle, attributeName);
        program.vertexInputLayout.attributes.push_back({ attributeLocation, (u8)attributeSize });
    }

    app->programs.push_back(program);
    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
        case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
        case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
        default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    // Try finding a vao for this submesh/program
    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
    {
        if (submesh.vaos[i].programHandle == program.handle)
            return submesh.vaos[i].handle;
    }

    // Create a new vao for 
    GLuint vaoHandle = 0;
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

    // We have to link all vertex inputs attributes to attributes in the vertex buffer
    for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
    {
        bool attributeWasLinked = false;

        for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
        {
            if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
            {
                const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset; // attribute offset + vertex offset
                const u32 stride = submesh.vertexBufferLayout.stride;
                glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                glEnableVertexAttribArray(index);

                attributeWasLinked = true;
                break;
            }
        }

        assert(attributeWasLinked); // The submesh should provide an attribute for each vertex inputs
    }

    glBindVertexArray(0);

    // Store it in the list of vaos for this submesh
    Vao vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);

    return vaoHandle;
}

void OnGLError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    ELOG("OpenGL debug message: %s", message);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API: ELOG(" - source: GL_DEBUG_SOURCE_API");                           break;  // Calls to OpenGL API
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: ELOG(" - source: GL_DEBUG_SOURCE_WINDOW_SYSTEM");       break;  // Calls to a window-system API
        case GL_DEBUG_SOURCE_SHADER_COMPILER: ELOG(" - source: GL_DEBUG_SOURCE_SHADER_COMPILER");   break;  // A compiler for a shading language
        case GL_DEBUG_SOURCE_THIRD_PARTY: ELOG(" - source: GL_DEBUG_SOURCE_THIRD_PARTY");           break;  // An application associated to OpenGL
        case GL_DEBUG_SOURCE_APPLICATION: ELOG(" - source: GL_DEBUG_SOURCE_APPLICATION");           break;  // Generated by the user of this application
        case GL_DEBUG_SOURCE_OTHER: ELOG(" - source: GL_DEBUG_SOURCE_OTHER");                       break;  // Some source that is not any of these
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR: ELOG(" - type: GL_DEBUG_TYPE_ERROR");                             break;  // An error, typically from the API
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ELOG(" - type: GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"); break;  // Some behaviour marked deprecated
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: ELOG(" - type: GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR");   break;  // Something has invoked undefined behaviour
        case GL_DEBUG_TYPE_PORTABILITY: ELOG(" - type: GL_DEBUG_TYPE_PORTABILITY");                 break;  // Some functionality the user relies upon
        case GL_DEBUG_TYPE_PERFORMANCE: ELOG(" - type: GL_DEBUG_TYPE_PERFORMANCE");                 break;  // Code has triggered possible performance problems
        case GL_DEBUG_TYPE_MARKER: ELOG(" - type: GL_DEBUG_TYPE_MARKER");                           break;  // Command stream annotation
        case GL_DEBUG_TYPE_PUSH_GROUP: ELOG(" - type: GL_DEBUG_TYPE_PUSH_GROUP");                   break;  // Group pushing
        case GL_DEBUG_TYPE_POP_GROUP: ELOG(" - type: GL_DEBUG_TYPE_POP_GROUP");                     break;  // Group pop
        case GL_DEBUG_TYPE_OTHER: ELOG(" - type: GL_DEBUG_TYPE_OTHER");                             break;  // Some type that is not any of these
    }

    switch (source)
    {
        case GL_DEBUG_SEVERITY_HIGH: ELOG(" - severity: GL_DEBUG_SEVERITY_HIGH");                   break;  // All OpenGL errors, shader cimpilation/linking
        case GL_DEBUG_SEVERITY_MEDIUM: ELOG(" - severity: GL_DEBUG_SEVERITY_MEDIUM");               break;  // Major performance warnings, shader compilation
        case GL_DEBUG_SEVERITY_LOW: ELOG(" - severity: GL_DEBUG_SEVERITY_LOW");                     break;  // Redundant state change performance warning
    }
}

void Init(App* app)
{
    app->mode = Mode::TexturedMesh;

    // Gather OpenGL information
    std::string aux;
    std::string version = (const char*)glGetString(GL_VERSION);
    aux = "OpenGL version: " + version;
    app->info.push_back(aux);

    std::string renderer = (const char*)glGetString(GL_RENDERER);
    aux = "OpenGL renderer: " + renderer;
    app->info.push_back(aux);

    std::string vendor = (const char*)glGetString(GL_VENDOR);
    aux = "OpenGL vendor: " + vendor;
    app->info.push_back(aux);

    std::string glsl = (const char*)glGetString(GL_VERSION);
    aux = "OpenGL GLSL: " + glsl;
    app->info.push_back(aux);

    GLint num_extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    glEnable(GL_DEPTH_TEST);

    // Get OpenGL errors
    if (GL_MAJOR_VERSION > 4 || (GL_MAJOR_VERSION == 4 && GL_MINOR_VERSION >= 3))
    {
        glDebugMessageCallback(OnGLError, app);
    }

    // Initialize resources
    const VertexV3V2 vertices[] = {
        { glm::vec3(-0.5, -0.5, 0.0), glm::vec2(0.0, 0.0) },
        { glm::vec3( 0.5, -0.5, 0.0), glm::vec2(1.0, 0.0) },
        { glm::vec3( 0.5,  0.5, 0.0), glm::vec2(1.0, 1.0) },
        { glm::vec3(-0.5,  0.5, 0.0), glm::vec2(0.0, 1.0) },
    };

    const u16 indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    // Uniform buffers
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBufferAlignment);
    app->cbuffer = CreateConstantBuffer(app->maxUniformBufferSize);

    // Vertex buffers 
    glGenBuffers(1, &app->embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Element/index buffers
    glGenBuffers(1, &app->embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Vaos
    glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)12);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBindVertexArray(0);

    // Programs
    app->texturedGeometryProgramIdx = LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
    app->texturedMeshProgramIdx = LoadProgram(app, "shaders.glsl", "SHOW_TEXTURED_MESH");

    // Load textures
    app->diceTexIdx = LoadTexture2D(app, "dice.png");
    app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");

    // Load models
    app->model = LoadModel(app, "Patrick/Patrick.obj");

    // Create entities
    Entity ent1 = Entity(glm::mat4(1.0), app->model, 0, 0);
    ent1.worldMatrix = glm::translate(ent1.worldMatrix, vec3(0.0, 1.0, -2.0));
    app->entities.push_back(ent1);

    Entity ent2 = Entity(glm::mat4(1.0), app->model, 0, 0);
    ent2.worldMatrix = glm::translate(ent2.worldMatrix, vec3(5.0, 1.0, -5.0));
    app->entities.push_back(ent2);

    Entity ent3 = Entity(glm::mat4(1.0), app->model, 0, 0);
    ent3.worldMatrix = glm::translate(ent3.worldMatrix, vec3(-5.0, 1.0, -5.0));
    app->entities.push_back(ent3);

    // Create lights
    Light light1 = Light(LightType::LightType_Directional, vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), vec3(0.0, 10.0, 0.0));
    app->lights.push_back(light1);

    Light light2 = Light(LightType::LightType_Point, vec3(0.0, 1.0, 0.0), vec3(2.0, 2.0, 0.0), vec3(0.0, 0.0, 0.0));
    app->lights.push_back(light2);

    Light light3 = Light(LightType::LightType_Point, vec3(1.0, 0.0, 0.0), vec3(-2.0, -2.0, 0.0), vec3(0.0, 0.0, 0.0));
    app->lights.push_back(light3);
}

void Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    
    for (int i = 0; i < app->info.size(); ++i)
        ImGui::Text(app->info[i].c_str());

    ImGui::End();
}

void Update(App* app)
{
    // Update programs regarding their timestamps
    for (u64 i = 0; i < app->programs.size(); ++i)
    {
        Program& program = app->programs[i];
        u64 currentTimestamp = GetFileLastWriteTimestamp(program.filepath.c_str());

        if (currentTimestamp > program.lastWriteTimestamp)
        {
            glDeleteProgram(program.handle);
            String programSource = ReadTextFile(program.filepath.c_str());
            const char* programName = program.programName.c_str();
            program.handle = CreateProgramFromSource(app, programSource, programName);
            program.lastWriteTimestamp = currentTimestamp;
        }
    }

    vec3 cameraPos = vec3(0.0f, 2.0f, 7.5f);

    glm::mat4 viewMatrix = glm::lookAt(
        cameraPos,              // the position of your camera, in world space
        vec3(0.0f, 1.0f, 0.0f), // where you want to look at, in world space
        glm::vec3(0, 1, 0)      // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
    );

    // Generates a really hard-to-read matrix, but a normal, standard 4x4 matrix nonetheless
    glm::mat4 projectionMatrix = glm::perspective(
        glm::radians(60.0f),    // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
        4.0f / 3.0f,            // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
        0.1f,                   // Near clipping plane. Keep as big as possible, or you'll get precision issues.
        100.0f                  // Far clipping plane. Keep as little as possible.
    );

    // Global parameters
    MapBuffer(app->cbuffer, GL_WRITE_ONLY);
    app->globalParamsOffset = app->cbuffer.head;

    PushVec3(app->cbuffer, cameraPos);
    PushUInt(app->cbuffer, app->lights.size());

    for (u32 i = 0; i < app->lights.size(); ++i)
    {
        AlignHead(app->cbuffer, sizeof(vec4));

        Light& light = app->lights[i];
        PushVec3(app->cbuffer, light.color);
        PushVec3(app->cbuffer, light.direction);
        PushVec3(app->cbuffer, light.position);
        PushUInt(app->cbuffer, light.type);
    }

    app->globalParamsSize = app->cbuffer.head - app->globalParamsOffset;

    // Local parameters
    for (u32 i = 0; i < app->entities.size(); ++i)
    {
        AlignHead(app->cbuffer, app->uniformBufferAlignment);

        Entity&     entity = app->entities[i];
        glm::mat4   world = entity.worldMatrix;
        glm::mat4   worldViewProjection = projectionMatrix * viewMatrix * world;

        entity.localParamsOffset = app->cbuffer.head;
        PushMat4(app->cbuffer, world);
        PushMat4(app->cbuffer, worldViewProjection);
        entity.localParamsSize = app->cbuffer.head - entity.localParamsOffset;
    }

    UnmapBuffer(app->cbuffer);
}

void Render(App* app)
{
    if (app->enableDebugGroups) 
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Shaded model");

    // Clear the framebuffer
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the viewport
    glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    switch (app->mode)
    {
        case Mode::TexturedQuad:
        {
            // Bind the program
            Program& texturedGeometryProgram = app->programs[app->texturedGeometryProgramIdx];
            glUseProgram(texturedGeometryProgram.handle);

            // Bind the vao       
            glBindVertexArray(app->vao);

            // Set the blending state
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Bind the texture into unit 0
            glActiveTexture(GL_TEXTURE0);

            GLuint textureHandle = app->textures[app->diceTexIdx].handle;
            glBindTexture(GL_TEXTURE_2D, textureHandle);

            // Draw elements
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        }
        break;

        case Mode::TexturedMesh:
        {
            // Bind the program
            Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
            glUseProgram(texturedMeshProgram.handle);

            for (int i = 0; i < app->entities.size(); ++i)
            {
                Model& model = app->models[app->entities[i].modelIndex];
                Mesh& mesh = app->meshes[model.meshIdx];

                glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->cbuffer.handle, app->globalParamsOffset, app->globalParamsSize);
                glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->cbuffer.handle, app->entities[i].localParamsOffset, app->entities[i].localParamsSize);

                for (u32 j = 0; j < mesh.submeshes.size(); ++j)
                {
                    GLuint vao = FindVAO(mesh, j, texturedMeshProgram);
                    glBindVertexArray(vao);

                    u32 submeshMaterialIdx = model.materialIdx[j];
                    Material& submeshMaterial = app->materials[submeshMaterialIdx];

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                    //glUniform1i(app->programMeshTexture, 0);

                    // Draw elements
                    Submesh& submesh = mesh.submeshes[j];
                    glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                }
            }
        }
        break;
    }

    if (app->enableDebugGroups)
        glPopDebugGroup();

    glBindVertexArray(0);
    glUseProgram(0);
}