
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

struct ObjShape
{
    std::shared_ptr<Mesh> mesh;
    map<GLuint, vector<DrawRange>> rangesPerTex;
};


struct ObjContainer
{
    vector<std::shared_ptr<ObjShape>> objParts;
    glm::mat4 tMatrix;
    std::shared_ptr<Mesh> bBox;
    glm::vec3 bbMin, bbMax;
};

struct LightInfo
{
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 direction;
    float linear;
    float constant;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

GLFWwindow* window;
auto closeWindow = false;
auto capturePos = false;

std::shared_ptr<Camera> camera;
float fov=45;
glm::mat4 projectionMat = glm::perspective(fov, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 5000.0f);
glm::mat4 globalModelMat = glm::mat4(1);



std::shared_ptr<GlslProgram> basicProgram, dirLightProgram,geomNormProgram,heightMapProgram;
std::vector<std::shared_ptr<Mesh>> defaultMatObjs,geomShaderObjs;
std::shared_ptr<Mesh> lBox,hMesh;
std::shared_ptr<FrameBuffer> layer1;
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

bool pMode = true;

int hMapWidth;
int hMapHeight;

#pragma endregion 


#pragma region prototypes

void createWindow();
void initGL();
void cleanGL();
void initImgui();
void setupCamera();
void setupScene();
std::shared_ptr<FrameBuffer> getFboMSA(std::shared_ptr<FrameBuffer> refFbo,int samples);
std::shared_ptr<Mesh> getMeshFromHeightMap(std::string heightMapPath);
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
        
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        
    }
    else if (key == GLFW_KEY_RIGHT
        || key == GLFW_KEY_D/*&& action == GLFW_PRESS*/)
    {
        camera->eye.x += 5;
        camera->center.x += 5;
        camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
    }
    else if (key == GLFW_KEY_LEFT
        || key == GLFW_KEY_A/*&& action == GLFW_PRESS*/)
    {
        camera->eye.x -= 5;
        camera->center.x -= 5;
        camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
    }
    else if (key == GLFW_KEY_UP
        || key == GLFW_KEY_W/*&& action == GLFW_PRESS*/)
    {
        camera->eye.z -= 5;
        camera->center.z -= 5;
        camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
    }
    else if (key == GLFW_KEY_DOWN
        || key == GLFW_KEY_S/*&& action == GLFW_PRESS*/)
    {
        camera->eye.z += 5;
        camera->center.z += 5;
        camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
    }
    else if (key == GLFW_KEY_Z /*&& action == GLFW_PRESS*/)
    {
        camera->eye.y += 5;
        if ((mods == GLFW_MOD_CONTROL))
            camera->center.y += 5;
        camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
    }
    else if (key == GLFW_KEY_X/*&& action == GLFW_PRESS*/)
    {
        camera->eye.y -= 5;
        if((mods == GLFW_MOD_CONTROL))
            camera->center.y -= 5;
        camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
        
    }
    else if (key == GLFW_KEY_Q /*&& action == GLFW_PRESS*/)
    {
        
    }
    else if (key == GLFW_KEY_E /*&& action == GLFW_PRESS*/)
    {
        
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        //capturePos = true;
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
    
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Step1", NULL, NULL);

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

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

    basicProgram = std::make_shared<GlslProgram>(Utility::readFileContents("shaders/vBasic.glsl"), Utility::readFileContents("shaders/fBasic.glsl"));
    heightMapProgram= std::make_shared<GlslProgram>(Utility::readFileContents("shaders/vHeightMap.glsl"), Utility::readFileContents("shaders/fHeightMap.glsl"));
    dirLightProgram = std::make_shared<GlslProgram>(
        Utility::readFileContents("shaders/vMatSpotLight.glsl"),
        Utility::readFileContents("shaders/fMatSpotLight.glsl")
        );
    geomNormProgram = std::make_shared<GlslProgram>(
        Utility::readFileContents("shaders/gs/vNorm.glsl"),
        Utility::readFileContents("shaders/gs/gNorm.glsl"),
        Utility::readFileContents("shaders/gs/fNorm.glsl")
        );

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

    auto eye = glm::vec3(0, 20, 100);
    auto center = glm::vec3(0, 0, 0);
    auto up = glm::vec3(0, 1, 0);
    camera = std::make_shared<Camera>(eye,center,up);
    
    camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
}

void setupScene()
{
    light.direction = glm::vec3(0, -1, -1);
    light.position = glm::vec3(0, 3, 0);
    light.ambient = glm::vec3(0.2f);
    light.diffuse = glm::vec3(0.5f);
    light.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    light.constant = 1.0f;
    light.linear = 0.045f;
    light.quadratic = 0.0075f;
    light.cutOff = 40.0f;
    light.outerCutOff = 60.0f;

    hMesh = ::getMeshFromHeightMap("img/iceland_heightmap.png");
    hMesh->color = glm::vec4(Color::orange, 1.0);
    //geomShaderObjs.push_back(triMesh);
    defaultMatObjs.push_back(lBox);
    
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

std::shared_ptr<Mesh> getMeshFromHeightMap(std::string heightMapPath)
{
    int width, height, nrChannels;

    unsigned char* data = GLUtility::getImageData(heightMapPath.c_str(), width, height, nrChannels);
    hMapWidth = width;
    hMapHeight = height;
    float yScale = 64.0f / 256.0f, yShift = 16.0f;
    std::vector<glm::vec3> vData;
    for (size_t h = 0; h < height; h++)
    {
        for (size_t w = 0; w < width; w++)
        {
            auto y = data + ((width * h) + w) * nrChannels;
            glm::vec3 position = glm::vec3((-static_cast<float>(width) / 2) + w, static_cast<int>(*y) * yScale - yShift, (-static_cast<float>(height) / 2) + h);
            vData.push_back(position);
        }
    }

    std::vector<unsigned int> iData;
    for (unsigned int h = 0; h < height-1; h++)
    {
        for (unsigned int w = 0; w < width; w++)
        {
            auto v0 = (h * width) + w;
            auto v1 = ((h+1) * width) + w;
            iData.push_back(v0); iData.push_back(v1); 
        }
    }

    GLUtility::freeImageData(data);

    auto mesh = std::make_shared<Mesh>(vData, iData);

    return mesh;
}

void updateFrame()
{
    auto t = glfwGetTime()*20;
    auto theta = (int)t % 360;
}

void renderFrame()
{
   
    glBindFramebuffer(GL_FRAMEBUFFER, layer1->fbo);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glm::mat4 mv;
    heightMapProgram->bind();
    
    mv = hMesh->tMatrix * globalModelMat;
    heightMapProgram->setMat4f("view", camera->viewMat);
    heightMapProgram->setMat4f("proj", projectionMat);
    heightMapProgram->setMat4f("model", mv);
    heightMapProgram->bindAllUniforms();

    glBindVertexArray(hMesh->vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hMesh->ibo);
    size_t lHeight = static_cast<size_t>(hMapHeight) - 1;
    size_t dCount = static_cast<size_t>(hMapWidth) * 2;
    for (size_t h=1;h<lHeight-1;h++)
    {
        size_t offSet = dCount*h * sizeof(unsigned int);
        glDrawElements(GL_TRIANGLE_STRIP, hMapWidth * 2, GL_UNSIGNED_INT, (void*)offSet);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    heightMapProgram->unbind();
    
   /* geomNormProgram->bind();
    geomNormProgram->setMat4f("view", camera->viewMat);
    geomNormProgram->setMat4f("proj", projectionMat);
    geomNormProgram->setVec3f("color", Color::red);
    for (const auto& obj : scenObjects)
    {
        auto mv = obj->tMatrix * globalModelMat;
        auto nrmlMat = glm::transpose(glm::inverse(mv));

        geomNormProgram->setMat4f("model", mv);
        geomNormProgram->setMat4f("nrmlMat", nrmlMat);

        geomNormProgram->bindAllUniforms();

        obj->draw();
    }
    geomNormProgram->unbind();*/


    //blitting
    auto err = glGetError();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, layer1->fbo);
    glDrawBuffer(GL_BACK);
    glBlitFramebuffer(0, 0, WIN_WIDTH, WIN_HEIGHT, 0, 0, WIN_WIDTH, WIN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBlitFramebuffer(0, 0, WIN_WIDTH, WIN_HEIGHT, 0, 0, WIN_WIDTH, WIN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    
}

void renderImgui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Height map Demo");                     

        auto prevM = pMode;
        ImGui::Checkbox("Polygon mode Fill", &pMode);
        if (prevM != pMode)
        {
            if (!pMode)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        ImGui::Text("Use A,W,S,D or Arrow keys to navigate in XZ plane");
        ImGui::Text("Use Z,X key navigate in Y axis");
        ImGui::Text("Use Ctrl+Z,X to change cam center");
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

