
#include "CommonHeaders.h"
#include "GlslProgram.h"
#include "GLUtility.h"
#include "Utility.h"
#include "Colors.h"
#include "Materials.h"
#include "Camera.h"
using namespace GLUtility;

#pragma region vars

const int WIN_WIDTH = 1920;
const int WIN_HEIGHT = 1080;

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


struct LightInfo
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 direction;

};

GLFWwindow* window;
auto closeWindow = false;

std::shared_ptr<Camera> camera;
float fov=45;
glm::mat4 projectionMat = glm::perspective(fov, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 100.0f);


glm::mat4 globalModelMat = glm::mat4(1);



std::shared_ptr<GlslProgram> dirLightProgram;
std::vector<std::shared_ptr<Mesh>> scenObjects;
std::shared_ptr<Mesh> fsQuad;
std::shared_ptr<FrameBuffer> layer1,layer2;
std::shared_ptr<Texture2D> diffuseTex, specularTex;
glm::vec3 lightPosition = glm::vec3(5, 6, 0);

struct Framebuffer
{
    GLuint fbo;
    std::vector<GLuint> colorAttachments;
    GLuint depth;
    ~Framebuffer()
    {
        if (colorAttachments.size() > 0)
            glDeleteTextures((GLsizei)colorAttachments.size(), colorAttachments.data());

        colorAttachments.clear();

        if (depth > 0)
        {
            depth = 0;
            glDeleteRenderbuffers(1, &depth);
        }
    }
};
std::shared_ptr<Framebuffer> blitContainer;


LightInfo light;
glm::mat4 uvRotMat = glm::mat4(1);
bool captureFrameFlg= false;
bool rotateFlg = false;
bool viewBlendFrame = false;
bool usePbo = true;

float gRotation = 83;

float bluePos[3] = { 0.2f, 0, -1.0f };
float orangePos[3] = { 0, 0, 0 };

#pragma endregion 


#pragma region prototypes

void createWindow();
void initGL();
void cleanGL();
void initImgui();
void setupCamera();
void setupScene();
std::shared_ptr<FrameBuffer> getFboMSA(std::shared_ptr<FrameBuffer> refFbo,int samples);
void updateFrame();
void renderFrame();
void renderImgui();



#pragma endregion


#pragma region functions

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    else if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        captureFrameFlg = true;
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        rotateFlg = true;
    }
    else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        
    }
    else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        
    }
    else if (key == GLFW_KEY_UP /*&& action == GLFW_PRESS*/)
    {
        fov -= 0.05f;
        projectionMat = glm::perspective(fov, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 100.0f);
        
    }
    else if (key == GLFW_KEY_DOWN /*&& action == GLFW_PRESS*/)
    {
        fov += 0.05f;
        projectionMat = glm::perspective(fov, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 100.0f);
        
    }
    else if (key == GLFW_KEY_Z /*&& action == GLFW_PRESS*/)
    {
        fov -= 0.05f;
        projectionMat = glm::perspective(fov, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 100.0f);
        
    }
    else if (key == GLFW_KEY_X /*&& action == GLFW_PRESS*/)
    {
        fov += 0.05f;
        projectionMat = glm::perspective(fov, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 100.0f);
        
    }
    else if (key == GLFW_KEY_Q /*&& action == GLFW_PRESS*/)
    {
        gRotation -= 0.05f;
    }
    else if (key == GLFW_KEY_E /*&& action == GLFW_PRESS*/)
    {
        gRotation += 0.05f;
    }
}

void createWindow()
{
    if (GLFW_TRUE != glfwInit())
    {
        cout << "Failed to init glfw" << '\n';
    }
    else
        cout << "glfw init success" << '\n';

    glfwSetErrorCallback(error_callback);
    

    window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Step1", NULL, NULL);

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwShowWindow(window);

    auto ginit = glewInit();
    if (ginit != GLEW_OK)
    {
        cout << "Failed to init glew" << '\n';
        return;
    }
    else
    {
        auto glVersionBytes = (char*)glGetString(GL_VERSION);
        auto glVendorBytes = (char*)glGetString(GL_VENDOR);
        string glversion((char*)glVersionBytes);
        string glvendor((char*)glVendorBytes);

        cout << "glew init success" << '\n';
        cout << glversion << '\n';
        cout << glvendor << cendl;
        cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << cendl;
    }

}

void initGL()
{
    glEnable(GL_DEPTH_TEST);
    
    

    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

    dirLightProgram = std::make_shared<GlslProgram>(Utility::readFileContents("shaders/vMatSpotLight.glsl"), Utility::readFileContents("shaders/fMatSpotLight.glsl"));
    
    
    setupScene();
    setupCamera();
   
    layer1 = getFboMSA(nullptr, 4);

    auto diffTex = GLUtility::makeTexture("img/container_diffuse.png");
    auto specTex = GLUtility::makeTexture("img/container_specular.png");
    diffuseTex = std::make_shared<Texture2D>();
    diffuseTex->texture = diffTex;
    specularTex = std::make_shared<Texture2D>();
    specularTex->texture = specTex;
}

void cleanGL()
{
}

void initImgui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    //"#version 130"
}

void setupCamera() {

    auto eye = glm::vec3(-5, 5, 10);
    auto center = glm::vec3(0, 0, 0);
    auto up = glm::vec3(0, 1, 0);
    camera = std::make_shared<Camera>(eye,center,up);
    
    camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
}

void setupScene()
{
    light.direction = glm::vec3(0, -1, -1);
    light.ambient = glm::vec3(0.3f);
    light.diffuse = glm::vec3(0.5f);
    light.specular = glm::vec3(1.0f, 1.0f, 1.0f);

    fsQuad = GLUtility::getfsQuad();

    auto floor = GLUtility::get2DRect(5.0f, 5.0f);
    auto trans = glm::translate(glm::mat4(1),glm::vec3(0,-1,0));
    floor->tMatrix = trans*glm::rotate(glm::mat4(1), glm::radians(-90.0f), GLUtility::X_AXIS);
    floor->color = glm::vec4(Color::grey, 1.0);
    scenObjects.push_back(floor);

    auto orangeRect = get2DRect(2, 2);
    //orangeRect->tMatrix = glm::rotate(glm::mat4(1), glm::radians(25.0f), GLUtility::Y_AXIS);
    orangeRect->color = glm::vec4(Color::orange, 1.0);
    scenObjects.push_back(orangeRect);

    auto blueRect = get2DRect(2, 2);
    blueRect->tMatrix = glm::translate(glm::mat4(1), glm::vec3(0.2, 0, -1.0));
    blueRect->color = glm::vec4(Color::blue, 1.0);
    scenObjects.push_back(blueRect);


}


std::shared_ptr<FrameBuffer> getFboMSA(std::shared_ptr<FrameBuffer> refFbo, int samples)
{
    auto layer = std::make_shared<FrameBuffer>();
    auto width = WIN_WIDTH;
    auto height = WIN_HEIGHT;
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); //bind both read/write to the target framebuffer
    

    GLuint color, depth = 0;
    glGenTextures(1, &color);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, color);
    {
        glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_GENERATE_MIPMAP, GL_TRUE);
    }
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, width, height, GL_TRUE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, color, 0);

    if (refFbo == nullptr)
    {
        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    }
    else
    {
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, refFbo->depth);
    }

    const GLenum transparentDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, transparentDrawBuffers);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    checkFrambufferStatus(fbo);

    layer->fbo = fbo;
    layer->color = color;
    layer->depth = depth;
    return layer;
}


void updateFrame()
{
    auto t = glfwGetTime()*20;
    auto theta = (int)t % 360;
   
    scenObjects.at(2)->tMatrix = glm::translate(glm::mat4(1), glm::vec3(bluePos[0], bluePos[1], bluePos[2]));
    scenObjects.at(1)->tMatrix = glm::translate(glm::mat4(1), glm::vec3(orangePos[0], orangePos[1], orangePos[2]));
    camera->orbitY(gRotation);

}

void renderFrame()
{
   
    glBindFramebuffer(GL_FRAMEBUFFER, layer1->fbo);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    dirLightProgram->bind();

    dirLightProgram->setMat4f("view", camera->viewMat);
    dirLightProgram->setMat4f("proj", projectionMat);

    dirLightProgram->setVec3f("viewPos", camera->eye);

    dirLightProgram->setInt("material.diffuseSampler", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseTex->texture);
    dirLightProgram->setInt("material.specularSampler", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularTex->texture);

    dirLightProgram->setFloat("material.shininess", 64.0f);

    auto lightColor = Color::white;
    glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
    glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
    dirLightProgram->setVec3f("light.position", lightPosition);
    dirLightProgram->setVec3f("light.ambient", glm::vec3(0.2f));
    dirLightProgram->setVec3f("light.diffuse", glm::vec3(0.5f));
    dirLightProgram->setVec3f("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));

    dirLightProgram->setFloat("light.constant", 1.0f);
    dirLightProgram->setFloat("light.linear", 0.045f);
    dirLightProgram->setFloat("light.quadratic", 0.0075f);

    dirLightProgram->setVec3f("light.direction", glm::vec3(0, -1, 0));
    dirLightProgram->setFloat("light.cutOff", glm::cos(glm::radians(40.0f)));
    dirLightProgram->setFloat("light.outerCutOff", glm::cos(glm::radians(60.0f)));


    for (const auto &obj : scenObjects)
    {
        auto mv = obj->tMatrix * globalModelMat;
        auto nrmlMat = glm::transpose(glm::inverse(mv));

        dirLightProgram->setMat4f("model", mv);
        dirLightProgram->setMat4f("nrmlMat", nrmlMat);

        dirLightProgram->bindAllUniforms();

        obj->draw();
    }

    dirLightProgram->unbind();

    
    
    //blitting
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, layer1->fbo);
    glDrawBuffer(GL_BACK);
    glBlitFramebuffer(0, 0, WIN_WIDTH, WIN_HEIGHT, 0, 0, WIN_WIDTH, WIN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void renderImgui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Basic Example");                     

        ImGui::Text("Use UP & DOWN arrows for zoom-In&Out");
        ImGui::SliderAngle("GMat Y Rotation", &gRotation, -360, 360);
        ImGui::SliderFloat3("Blue",bluePos , -2, 2);
        ImGui::SliderFloat3("Orange", orangePos, -2, 2);
        

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();
    }
    

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}




#pragma endregion





int main()
{
    createWindow();

    initImgui();

    initGL();

    while (!glfwWindowShouldClose(window))
    {

        updateFrame();
        renderFrame();
        
        renderImgui();



        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

