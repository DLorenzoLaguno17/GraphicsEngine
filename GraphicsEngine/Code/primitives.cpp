#include "engine.h"
#include "primitives.h"

/*void DrawQuad(App* app)
{
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
}*/