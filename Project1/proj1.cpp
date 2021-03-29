#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <vector>
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"

using namespace std; // Uses the standard namespace

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Table plane with xbox"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];     // Handles for the vertex buffer objects
        GLuint nIndices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow;

    GLMesh gMeshCube;
    GLMesh gMeshTable;
    GLMesh gMeshwalls;
    // Shader program
    GLuint gProgramId;

    Camera camera(glm::vec3(0.f, 1.f, 3.f));
    float lastX = WINDOW_WIDTH / 2.0f;
    float lastY = WINDOW_HEIGHT / 2.0f;
    bool firstMouse = true;
    bool usePerspective = false;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UCreateMeshFromVerts(GLMesh& mesh, std::vector<GLfloat> const& verts, std::vector<GLushort> const& indices);
void URenderMesh(const GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Vertex Shader Program Source Code
const char* vertexShaderSource = "#version 440 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec4 colorFromVBO;\n"

"uniform mat4 transform = mat4(1.0);\n"

"out vec4 colorFromVS;\n"
"void main()\n"
"{\n"
"   gl_Position = transform * vec4(aPos.x , aPos.y, aPos.z, 1.0);\n"
"   colorFromVS = colorFromVBO;\n"
"}\n\0";

/*
0.5 0   0   0.25
0   0.5 0   0
0   0   0.5 0
0   0   0   1
*/

// Fragment Shader Program Source Code
const char* fragmentShaderSource = "#version 440 core\n"
"in vec4 colorFromVS;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = colorFromVS;\n"
"}\n\0";


// main function. Entry point to the OpenGL program
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    {
#define RED -1.0,-1.0,-1.0,-1.0
#define GREEN 0.1,1.0,0.3,1.0
        std::vector<GLfloat> verts = {
            -1, -1, -1, RED,    // 0
            -1, -1,  1, RED,    // 1
            -1,  1, -1, GREEN,  // 2
            -1,  1,  1, GREEN,  // 3
             1, -1, -1, RED,    // 4
             1, -1,  1, RED,    // 5
             1,  1, -1, GREEN,  // 6
             1,  1,  1, GREEN   // 7
        };
#undef RED
#undef GREEN

#define YELLOW 0.6,0.6,0.0,1.0
        std::vector<GLfloat> vertsTable = {
            -1, -1, -1, YELLOW,    // 0
            -1, -1,  1, YELLOW,    // 1
            -1,  1, -1, YELLOW,  // 2
            -1,  1,  1, YELLOW,  // 3
             1, -1, -1, YELLOW,    // 4
             1, -1,  1, YELLOW,    // 5
             1,  1, -1, YELLOW,  // 6
             1,  1,  1, YELLOW   // 7
        };
#undef YELLOW
#define RED -1.0,-1.0,-1.0,-1.0
        std::vector<GLfloat> vertswalls = {
            -1, -1, -1, RED,    // 0
            -1, -1,  1, RED,    // 1
            -1,  1, -1, RED,  // 2
            -1,  1,  1, RED,  // 3
             1, -1, -1, RED,    // 4
             1, -1,  1, RED,    // 5
             1,  1, -1, RED,  // 6
             1,  1,  1, RED   // 7
        };
#undef RED

        std::vector<GLushort> indices = {
            0, 2, 1, // left
            1, 2, 3,
            4, 6, 5, // right
            5, 6, 7,
            2, 6, 3, // top
            3, 6, 7,
            0, 4, 1, // bottom
            1, 4, 5,
            1, 5, 3, // front
            3, 5, 7,
            0, 4, 2, // back
            2, 4, 6
        };
      
        // Create the mesh
        UCreateMeshFromVerts(gMeshCube, verts, indices); // Calls the function to create the Vertex Buffer Object
        UCreateMeshFromVerts(gMeshTable, vertsTable, indices); // Calls the function to create the Vertex Buffer Object
        UCreateMeshFromVerts(gMeshwalls, vertswalls, indices); // Calls the function to create the Vertex Buffer Object
        
    }


    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMeshCube);

    // Release shader program
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    glfwSetCursorPosCallback(*window, mouse_callback);
    glfwSetScrollCallback(*window, scroll_callback);

    // glad: load all OpenGL function pointers
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        usePerspective = !usePerspective;
    }

    const float deltaTime = 1.f / 60.f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

float angle = 0.f;

// Functioned called to render a frame
void URender()
{
    // Clear the background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);
    //glEnable(GL_CULL_FACE);

    // Set the shader to be used
    glUseProgram(gProgramId);

    GLint location = glGetUniformLocation(gProgramId, "transform");
    GLsizei count = 1;
    GLboolean transpose = GL_FALSE;
   


    glm::mat4 projection;
    if (usePerspective) {
        projection = glm::perspective(70.f, 1.0f, 0.1f, 100.f);
    }
    else {
        projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.1f, 100.f);
    }
    
    //const glm::mat4 view = glm::lookAt(glm::vec3(0.f, 1.f, 3.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
    const glm::mat4 view = camera.GetViewMatrix();
    
 
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.70, 0.0, 0.5));
        model = glm::rotate(model, angle, glm::vec3(0.f, 1.f, 0.f));
        model = glm::scale(model, glm::vec3(0.20, 0.90, 0.4));

        glm::mat4 transform = projection * view * model;

        glUniformMatrix4fv(location, count, transpose, glm::value_ptr(transform));

        // Render mesh1
        URenderMesh(gMeshCube);
    }

    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.70, 0.0, 0.5));
        model = glm::rotate(model, angle, glm::vec3(0.f, 1.f, 0.f));
        model = glm::scale(model, glm::vec3(0.20, 0.90, 0.4));

        glm::mat4 transform = projection * view * model;

        glUniformMatrix4fv(location, count, transpose, glm::value_ptr(transform));

        // Render mesh1
        URenderMesh(gMeshCube);
    }
    {
        glm::mat4 tableModel = glm::mat4(1.0f);
        tableModel = glm::translate(tableModel, glm::vec3(-0.70, -0.51, 0));
        tableModel = glm::rotate(tableModel, angle, glm::vec3(0.f, 1.f, 0.f));

        glm::mat4 transform = projection * view * glm::scale(tableModel, glm::vec3(1.75, 0.10, 0.99));

        glUniformMatrix4fv(location, count, transpose, glm::value_ptr(transform));

        // Render mesh1
        URenderMesh(gMeshTable);

        {
            glm::mat4 model = tableModel;
            model = glm::translate(model, glm::vec3(1.75, -2.0, 0.99));
            model = glm::scale(model, glm::vec3(0.1, 2.0, 0.1));

            glm::mat4 transform = projection * view * model;

            glUniformMatrix4fv(location, count, transpose, glm::value_ptr(transform));

            // Render mesh1
            URenderMesh(gMeshTable);
        }
        {
            glm::mat4 model = tableModel;
            model = glm::translate(model, glm::vec3(-1.75, -2.0, 0.99));
            model = glm::scale(model, glm::vec3(0.1, 2.0, 0.1));

            glm::mat4 transform = projection * view * model;

            glUniformMatrix4fv(location, count, transpose, glm::value_ptr(transform));

            // Render mesh1
            URenderMesh(gMeshTable);
        }
        {
            glm::mat4 model = tableModel;
            model = glm::translate(model, glm::vec3(-1.75, -2.0, -0.99));
            model = glm::scale(model, glm::vec3(0.1, 2.0, 0.1));

            glm::mat4 transform = projection * view * model;

            glUniformMatrix4fv(location, count, transpose, glm::value_ptr(transform));

            // Render mesh1
            URenderMesh(gMeshTable);
        }
        {
            glm::mat4 model = tableModel;
            model = glm::translate(model, glm::vec3(1.75, -2.0, -0.99));
            model = glm::scale(model, glm::vec3(0.1, 2.0, 0.1));

            glm::mat4 transform = projection * view * model;

            glUniformMatrix4fv(location, count, transpose, glm::value_ptr(transform));

            // Render mesh1
            URenderMesh(gMeshTable);
        }
        
    }


    //angle += 0.0001;
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


void URenderMesh(const GLMesh& mesh) {
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
    glBindVertexArray(0);
}

void UCreateMeshFromVerts(GLMesh& mesh, std::vector<GLfloat> const& verts, std::vector<GLushort> const& indices) {
    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Creates the Vertex Attribute Pointer for the screen coordinates
    const GLuint floatsPerVertex = 3; // Number of coordinates per vertex
    const GLuint floatsPerColor = 4;  // (r, g, b, a)

    // Strides between vertex coordinates is 6 (x, y, r, g, b, a). A tightly packed stride is 0.
    const GLint stride = sizeof(GLfloat) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, verts.size() * stride, verts.data(), GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Creates a buffer object for the indices
    mesh.nIndices = indices.size();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

    // Creates the Vertex Attribute Pointer
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(GLfloat) * floatsPerVertex));
    glEnableVertexAttribArray(1);
}

void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;
   

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}


