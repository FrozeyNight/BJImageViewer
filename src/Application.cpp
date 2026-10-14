#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "stb_image.h"
// a Render works like this: you give it a command and it renders that thing

//bool isWindowResizedByCode = false;
float zoom = 1.0f;
float rotation = 0.0f;
bool rotatedImage = false;

//void window_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // Setting the version of OpenGL and choosing the CORE profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // get image size from file
    int imageWidth = 0;
    int imageHeight = 0;
    stbi_load("Example1.png", &imageWidth, &imageHeight, nullptr, 0);

    float aspectRatio = (float)imageWidth / imageHeight;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(imageWidth, imageHeight, "BJImageViewer", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // makes the window render in the monitor's? framerate
    glfwSwapInterval(1);

    if(glewInit() != GLEW_OK){
        std::cout << "Error!" << std::endl;
    }

    {   

        // We probably need the width and height of the window, so we can always have the image centered
        // we may only need to adjust the translationA to put the image in the middle
        // we (only?) need to adjust the positions in real time, to do that we need to move the positions, VertexArray and VertexBuffer (I don't think layout) to the window loop. (Do I only need to make the vb and bind it? check)

        int windowWidth = 0;
        int windowHeight = 0;
        glfwGetWindowSize(window, &windowWidth, &windowHeight);

        //glViewport(0, 0, windowWidth, windowHeight);

        
        float positions[] = {
            float(((windowWidth / 2) - (imageWidth / 2))), (float)((windowHeight / 2) - (imageHeight / 2)), 0.0f, 0.0f,// 0
            float(((windowWidth / 2) + (imageWidth / 2))), (float)((windowHeight / 2) - (imageHeight / 2)), 1.0f, 0.0f,// 1
            float(((windowWidth / 2) + (imageWidth / 2))), (float)((windowHeight / 2) + (imageHeight / 2)), 1.0f, 1.0f,// 2
            float(((windowWidth / 2) - (imageWidth / 2))), (float)((windowHeight / 2) + (imageHeight / 2)), 0.0f, 1.0f,// 3
        };
        
        
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        GLCall(glEnable(GL_BLEND));
        // Blending (making it so transparency works on textures)
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        // Creating a Vertex Array
        unsigned int vao;
        GLCall(glGenVertexArrays(1, &vao));
        GLCall(glBindVertexArray(vao));

        VertexArray va;
        VertexBuffer vb(positions, 4 * 4 * sizeof(float));

        VertexBufferLayout layout;
        layout.Push<float>(2); // 2 because we're adding 2 triangles
        layout.Push<float>(2); // for texture coordinates
        va.AddBuffer(vb, layout);


        IndexBuffer ib(indices, 6);
        glm::mat4 projectionMatrix = glm::ortho(0.0f, (float)imageWidth, 0.0f, (float)imageHeight, -1.0f, 1.0f);
        glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

        //glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

        Shader shader("Basic.shader");
        shader.Bind();
        //shader.SetUniform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f);
        //shader.SetUniformMat4f("u_MVP", mvp);

        Texture texture("Example1.png");
        texture.Bind();
        shader.SetUniform1i("u_Texture", 0); // 0 means the slot in which the texture we want to access is

        va.Unbind();
        shader.Unbind();
        vb.Unbind();
        ib.Unbind();

        Renderer renderer;

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        // trigger an event when the window is resized
        //glfwSetWindowSizeCallback(window, window_size_callback);
        glfwSetScrollCallback(window, scroll_callback);
        

        glm::vec3 translationA(0, 0, 0);

        /*
        int windowWidthHalved = windowWidth / 2;
        int windowHeightHalved = windowHeight / 2;
        int imageWidthHalved = ImageWidth / 2;
        int imageHeightHalved = ImageHeight / 2;
        float left = windowWidthHalved - imageWidthHalved;
        float right = windowWidthHalved + imageWidthHalved;
        float bottom = windowHeightHalved - imageHeightHalved;
        float top = windowHeightHalved + imageHeightHalved;
        */

        int scaledImageWidth = 0;
        int scaledImageHeight = 0;

        float imageWidthScale = 0.0f;
        float imageHeightScale = 0.0f;

        //float rotation = 0.0f;

        //bool rotatedImage = false;
        int temporaryImageWidth = 0;
        int temporaryImageHeight = 0;
        float temporaryRotation = 0;
        float absoluteRotation = 0;

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            renderer.Clear();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            //glfwGetWindowSize(window, &windowWidth, &windowHeight);
            glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

            // to make the image possible to render after resizing the window we need to change the viewport, so OpenGL can
            //  render on the entire (new) window resolution

            glViewport(0, 0, windowWidth, windowHeight);

            // we also need to update the projection matrix, since the size of the window changed
            projectionMatrix = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, -1.0f, 1.0f);

            scaledImageWidth = imageWidth;
            scaledImageHeight = imageHeight;

            /* scaling the image if it's too small, disabled for testing rotation
            if(windowWidth < imageWidth || windowHeight < imageHeight){
                scaledImageWidth = (int)(windowHeight * aspectRatio + 0.5f);
                if(scaledImageWidth > windowWidth){
                    scaledImageHeight = (int)(windowWidth / aspectRatio + 0.5f);
                    scaledImageWidth = windowWidth;
                }
                else{
                    scaledImageHeight = windowHeight;
                }

                imageWidthScale = ((float)scaledImageWidth / (float)imageWidth);
                imageHeightScale = ((float)scaledImageHeight / (float)imageHeight);
            }
            else{
                imageWidthScale = 1.0f;
                imageHeightScale = 1.0f;
            }
            */

            // FOR QUICK TEST ONLY, DELETE AFTER
            imageWidthScale = 1.0f;
            imageHeightScale = 1.0f;

            translationA.x = ((windowWidth / 2) - (scaledImageWidth / 2));
            translationA.y = ((windowHeight / 2) - (scaledImageHeight / 2));
            /*
            windowWidthHalved = windowWidth / 2;
            windowHeightHalved = windowHeight / 2;
            imageWidthHalved = ImageWidth / 2;
            imageHeightHalved = ImageHeight / 2;
            left = windowWidthHalved - imageWidthHalved;
            right = windowWidthHalved + imageWidthHalved;
            bottom = windowHeightHalved - imageHeightHalved;
            top = windowHeightHalved + imageHeightHalved;

            float positions[] = {
                left, bottom, 0.0f, 0.0f,// 0
                right, bottom, 1.0f, 0.0f,// 1
                right, top, 1.0f, 1.0f,// 2
                left, top, 0.0f, 1.0f,// 3
            };
            
            glBindBuffer(GL_ARRAY_BUFFER, vb.GetID());
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);

            */

            // (remember we changed the vertex buffer to dynamic)

            /*
            // divide the width by the height to get the difference between them
            // when the window is smaller than the original image size AND both height and width of the window are greater than the scaled image size
            // use either the height or width (not sure which one) of the image and multiply that by the difference of original H and W
            // to get a very close approximation of the original aspect ratio
            // (you probably decide which one it is by how much bigger the window size to scaled image size is)
            // also we probably need to do this by updating vertices so back to original plan
            */

            temporaryImageWidth = scaledImageWidth * zoom;
            temporaryImageHeight = scaledImageHeight * zoom;
            absoluteRotation = glm::abs(rotation);
            temporaryRotation = absoluteRotation;

            //isWindowResizedByCode = false;

            int state = glfwGetKey(window, GLFW_KEY_RIGHT);
            if (state == GLFW_PRESS)
            {
                if(rotation > 360) rotation = 0.0f;
                rotation += 0.5f;
                rotatedImage = true;
            }
            state = glfwGetKey(window, GLFW_KEY_LEFT);
            if (state == GLFW_PRESS)
            {
                if(rotation < -360) rotation = 0.0f;
                rotation -= 0.5f;
                rotatedImage = true;
            }

            
            // Adjusting window size to accommodate rotated image
            if(rotatedImage){
                if(absoluteRotation >= 90 && absoluteRotation <= 180){
                    // the * zoom can be done only once, right now just for testing
                    temporaryImageWidth = scaledImageHeight * zoom;
                    temporaryImageHeight = scaledImageWidth * zoom;
                    temporaryRotation = absoluteRotation - 90;
                }
                else if(absoluteRotation > 180){
                    temporaryRotation = absoluteRotation - 180;
                    if(absoluteRotation > 270){
                        temporaryImageWidth = scaledImageHeight * zoom;
                        temporaryImageHeight = scaledImageWidth * zoom;
                        temporaryRotation = temporaryRotation - 90;
                    }
                }

                float degreeOfRotation = glm::radians(temporaryRotation);
                float remainderOfRotation = 90 - temporaryRotation;
                remainderOfRotation = glm::radians(remainderOfRotation);
                float c,f,d,e;
                c = temporaryImageHeight * glm::cos(remainderOfRotation);
                f = temporaryImageHeight * glm::sin(remainderOfRotation);
                d = temporaryImageWidth * glm::cos(degreeOfRotation);
                e = temporaryImageWidth * glm::sin(degreeOfRotation);

                int adjustedWindowWidth = c + d + 1;
                int adjustedWindowHeight = e + f + 1;

                if(adjustedWindowWidth > windowWidth || adjustedWindowHeight > windowHeight/* || abs(rotation) == 360 || rotation == 0*/){
                    int windowXPos = 0;
                    int windowYPos = 0;
                    int widthDifference = (adjustedWindowWidth - windowWidth) / 2;
                    int heightDifference = (adjustedWindowHeight - windowHeight)/ 2;
                    glfwGetWindowPos(window, &windowXPos, &windowYPos);;
                    glfwSetWindowPos(window, windowXPos - (widthDifference/2), windowYPos - (heightDifference/2));

                    glfwSetWindowSize(window, windowWidth + widthDifference, windowHeight + heightDifference);
                }
                    //glfwSetWindowSize(window, (int)(c + d + 1), (int)(e + f + 1));
                rotatedImage = false;
            }
            

            {
                glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), translationA);
                //modelMatrix = glm::scale(modelMatrix, glm::vec3(0.75, 0.75, 0));
                modelMatrix = glm::translate(modelMatrix, glm::vec3((scaledImageWidth / 2), (scaledImageHeight / 2), 0));
                modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation), glm::vec3(0.0, 0.0, 1.0));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(zoom, zoom, 0));
                modelMatrix = glm::translate(modelMatrix, glm::vec3(-(scaledImageWidth / 2), -(scaledImageHeight / 2), 0));
                //modelMatrix = glm::scale(modelMatrix, glm::vec3(imageWidthScale, imageHeightScale, 0));
                glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
                shader.Bind();
                shader.SetUniformMat4f("u_MVP", mvp);
                renderer.Draw(va, ib, shader);
            }

            //test += 0.1f;
            //rotatedImage = false;

            // ImGui debug window elements
            {
                ImGui::Begin("Debug Window"); // Create a window called "Hello, world!" and append into it.

                //ImGui::SliderFloat3("Translation A", &translationA.x, 0.0f, 960.0f); // Edit 1 float using a slider from 0.0f to 960.0f
                //ImGui::SliderFloat3("Translation B", &translationB.x, 0.0f, 960.0f); // Edit 1 float using a slider from 0.0f to 960.0f

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                /*
                if(ImGui::SliderFloat("Rotation", &rotation, -360.0f, 360.0f)){
                    rotatedImage = true;
                };
                */
                ImGui::Text("Image resolution: %i x %i", imageWidth, imageHeight);
                ImGui::Text("Window resolution: %i x %i", windowWidth, windowHeight);
                //ImGui::Text("Left Bottom: %.1f x %1.f", float(((windowWidth / 2) - (imageWidth / 2))), (float)((windowHeight / 2) - (imageHeight / 2)));
                //ImGui::Text("Right Top: %.1f x %1.f", float(((windowWidth / 2) + (imageWidth / 2))), (float)((windowHeight / 2) + (imageHeight / 2)));
                ImGui::Text("Scaled image res: %i x %i", scaledImageWidth, scaledImageHeight);
                ImGui::Text("Scaled image scale: %.3f x %.3f", imageWidthScale, imageHeightScale);
                
                ImGui::End();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }

    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    //ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

/*
void window_size_callback(GLFWwindow* window, int width, int height){
    if(isWindowResizedByCode) return;
    std::cout << "quick check" << std::endl;
}
*/

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(yoffset == -1 && zoom > 0.19f)
        zoom -= 0.1f;
    else if(yoffset == 1 && zoom < 20)
        zoom += 0.1f;
}