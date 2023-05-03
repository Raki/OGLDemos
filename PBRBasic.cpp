#include <iomanip>
#include "CommonHeaders.h"
#include "GlslProgram.h"
#include "GLUtility.h"
#include "Utility.h"
#include "Colors.h"
#include "Materials.h"
#include "Camera.h"
#include "mcut/mcut.h"
#include <fmt/format.h>
using namespace GLUtility;

#define OUTPUT_DIR "."
#define my_assert(cond)                             \
    if (!(cond)) {                                  \
        fprintf(stderr, "MCUT error: %s\n", #cond); \
        std::exit(1);                               \
    }

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

struct InputMesh {
    // variables for mesh data in a format suited for MCUT
    std::vector<uint32_t> faceSizesArray; // vertices per face
    std::vector<uint32_t> faceIndicesArray; // face indices
    std::vector<double> vertexCoordsArray; // vertex coords
    std::vector<double> vcBaseArray;
    uint32_t numVertices;
    uint32_t    numFaces;
    void clean()
    {
        faceSizesArray.clear(); // vertices per face
        faceIndicesArray.clear(); // face indices
        vertexCoordsArray.clear(); // vertex coords
    }
};

struct CSGResult
{
    std::shared_ptr<InputMesh> resMesh;
    std::vector<glm::vec3> vCSGArr;
    std::vector<unsigned int> iCSGArr;
};

GLFWwindow* window;
auto closeWindow = false;
auto capturePos = false;

std::shared_ptr<Camera> camera;
float fov=45;
const float zNear=0.1f;
const float zFar=1000.0f;
glm::mat4 projectionMat = glm::perspective(fov, (float)WIN_WIDTH / (float)WIN_HEIGHT, zNear,zFar);
glm::mat4 globalModelMat = glm::mat4(1);



std::shared_ptr<GlslProgram> basicProgram, dirLightProgram,cMapProgram;
std::vector<std::shared_ptr<Mesh>> defaultMatObjs,diffuseMatObjs,pvColrObjs;
std::shared_ptr<Mesh> lBox,groupObj,leftObj;
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


vector<DrawRange> ranges;
map<GLuint, vector<DrawRange>> rangesPerTex;
vector<std::shared_ptr<ObjContainer>> objModels;

LightInfo light;
glm::mat4 uvRotMat = glm::mat4(1);

float gRotation = 90;
float xRotation = 0;
float tLength = 0.1;
float camSpeed = 0.1;
const int rows = 5;
const int cols = 5;
auto srcMesh = std::make_shared<InputMesh>();
auto cutMesh = std::make_shared<InputMesh>();
auto brushMesh = std::make_shared<InputMesh>();

const char* bOplist[] = { "A_NOT_B", "B_NOT_A", "UNION", "INTERSECTION" };
int cOP = 2;
#pragma endregion 


#pragma region prototypes

void createWindow();
void initGL();
void cleanGL();
void initImgui();
void setupCamera();
void setupScene();
void setupCSGMeshV4();
std::shared_ptr<FrameBuffer> getFboMSA(std::shared_ptr<FrameBuffer> refFbo,int samples);
void updateFrame();
void renderFrame();
void renderImgui();



std::shared_ptr<ObjContainer> loadObjModel(std::string filePath,std::string defaultDiffuse="assets/textures/default.jpg");
void loadObj(std::string filePath,std::vector<double> &vData,std::vector<unsigned int> &iData);

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
        camera->eye.x += camSpeed;
        camera->center.x += camSpeed;
        camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
    }
    else if (key == GLFW_KEY_LEFT
        || key == GLFW_KEY_A/*&& action == GLFW_PRESS*/)
    {
        camera->eye.x -= camSpeed;
        camera->center.x -= camSpeed;
        camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
    }
    else if (key == GLFW_KEY_UP
        || key == GLFW_KEY_W/*&& action == GLFW_PRESS*/)
    {
        camera->eye.z -= camSpeed;
        camera->center.z -= camSpeed;
        camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
    }
    else if (key == GLFW_KEY_DOWN
        || key == GLFW_KEY_S/*&& action == GLFW_PRESS*/)
    {
        camera->eye.z += camSpeed;
        camera->center.z += camSpeed;
        camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
    }
    else if (key == GLFW_KEY_Z /*&& action == GLFW_PRESS*/)
    {
        camera->eye.y += camSpeed;
        if ((mods == GLFW_MOD_CONTROL))
            camera->center.y += camSpeed;
        camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
    }
    else if (key == GLFW_KEY_X/*&& action == GLFW_PRESS*/)
    {
        camera->eye.y -= camSpeed;
        if ((mods == GLFW_MOD_CONTROL))
            camera->center.y -= camSpeed;
        camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);

    }
    else if (key == GLFW_KEY_Q /*&& action == GLFW_PRESS*/)
    {
        gRotation -= 5.f;
    }
    else if (key == GLFW_KEY_E /*&& action == GLFW_PRESS*/)
    {
        gRotation += 5.f;
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double wx, wy; //window coords
        glfwGetCursorPos(window, &wx, &wy);

        float x = (2.0f * static_cast<float>(wx)) / WIN_WIDTH - 1.0f;
        float y = 1.0f - (2.0f * static_cast<float>(wy)) / WIN_HEIGHT;
        float z = 1.0f;
        glm::vec3 ray_nds = glm::vec3(x, y, z);
        glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
        glm::vec4 ray_eye = glm::inverse(projectionMat) * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

        glm::vec3 ray_wor = glm::vec3(glm::inverse(camera->viewMat) * ray_eye);
        // don't forget to normalise the vector at some point
        ray_wor = glm::normalize(ray_wor);

        glm::vec3 rayOrigin = camera->eye;
        glm::vec3 rayEnd = rayOrigin + (20.0f * ray_wor);



        std::vector<glm::vec3> tri;
        std::vector<glm::vec3> ray;

        ray.push_back(rayOrigin);
        ray.push_back(rayEnd);

        float dist = 10000;
        int hitCount = 0;
        glm::vec3 hitPoint;
        for (size_t o=0;o<pvColrObjs.size();o++)
        {
            std::shared_ptr<Mesh> obj = pvColrObjs.at(o);
            if (obj->drawCommand != GL_TRIANGLES)
                continue;
            const auto& boxVData = obj->vdPosNrmClr;
            const auto& boxIData = obj->iData;
            
            for (auto i = 0; i < boxIData.size(); i += 3)
            {
                auto v1 = (boxVData.size() > 0) ? boxVData.at(boxIData.at(i)).pos : obj->vData.at(boxIData.at(i)).pos;
                auto v2 = (boxVData.size() > 0) ? boxVData.at(boxIData.at(i + 1)).pos : obj->vData.at(boxIData.at(i + 1)).pos;
                auto v3 = (boxVData.size() > 0) ? boxVData.at(boxIData.at(i + 2)).pos : obj->vData.at(boxIData.at(i + 2)).pos;
                v1 = glm::vec3(obj->trans*obj->rot*obj->scle * glm::vec4(v1, 1));
                v2 = glm::vec3(obj->trans*obj->rot*obj->scle * glm::vec4(v2, 1));
                v3 = glm::vec3(obj->trans*obj->rot*obj->scle * glm::vec4(v3, 1));

                tri.push_back(v1);
                tri.push_back(v2);
                tri.push_back(v3);
                glm::vec3 intr;
                if (GLUtility::intersects3D_RayTrinagle(ray, tri, intr) == 1)
                {
                    obj->color = glm::vec4(Color::purple, 1.0);
                    std::cout << intr.x << " " << intr.y << " " << intr.z << cendl;
                    auto d = glm::distance(camera->eye, intr);
                    if (d < dist)
                    {
                        dist = d;
                        hitPoint = intr;
                    }
                    hitCount += 1;
                }
                else
                {
                    // blueBox->color = glm::vec4(Color::red, 1.0);
                }
                tri.clear();
            }
            if (hitCount == 0)
            {
                obj->color = obj->pickColor;
            }
        }//end of for loop
        if (hitCount > 0)
        {
            fmt::print("hit point {},{},{}", hitPoint.x, hitPoint.y, hitPoint.z);
            {
                auto v = glm::vec4(0, 0, 0, 1);
                v = leftObj->tMatrix * v;
                auto dir = glm::normalize( hitPoint - glm::vec3(v));
                auto v1 = dir;
                auto v2 = glm::vec3(0, 1, 0);
                auto v3 = glm::normalize(glm::cross(v1,v2));
                v2 = glm::normalize(glm::cross(v1, v3));
                auto m = glm::mat4(glm::vec4(v1, 0), glm::vec4(v2, 0), glm::vec4(v3, 0), glm::vec4(0, 0, 0, 1));
                leftObj->tMatrix = leftObj->tMatrix * m;
            }
        }
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
    glPointSize(3.0f);
    
    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

    auto vsStr = Utility::readFileContents("shaders/vLights.glsl");
    vsStr = Utility::replaceStrWith("#version 460", "#version 460 \n#define PER_VERTEX_COLOR",vsStr);
    auto fsStr = Utility::readFileContents("shaders/fLights.glsl");
    fsStr = Utility::replaceStrWith("#version 460", "#version 460 \n#define PER_VERTEX_COLOR", fsStr);
    basicProgram = std::make_shared<GlslProgram>(vsStr, fsStr);

    assert(basicProgram->programID);

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

    auto eye = glm::vec3(0, 2.5, 4);
    auto center = glm::vec3(0, 1, 0);
    auto up = glm::vec3(0, 1, 0);
    camera = std::make_shared<Camera>(eye,center,up);
    
    camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
}

void setupScene()
{
    light.direction = glm::vec3(0, -1, -1);
    light.position = glm::vec3(0, 5, 0);
    light.ambient = glm::vec3(0.2f);
    light.diffuse = glm::vec3(0.5f);
    light.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    light.constant = 1.0f;
    light.linear = 0.045f;
    light.quadratic = 0.0075f;
    light.cutOff = 40.0f;
    light.outerCutOff = 60.0f;

    lBox = GLUtility::getCubeVec3(0.2f, 0.2f, 0.2f);
    lBox->color = glm::vec4(1.0);
    lBox->tMatrix = glm::translate(glm::mat4(1), light.position);
    lBox->pickColor = glm::vec4(1.0);
    defaultMatObjs.push_back(lBox);

    std::vector<GLUtility::VDPosNormColr> vData;
    std::vector<unsigned int> iData;

    auto m = glm::mat4(1);
    //x axis
    GLUtility::fillCube(8, 0.1f, 0.1f, Color::red, m, vData, iData);
    //y axis
    GLUtility::fillCube(0.1, 8.f, 0.1f, Color::green, m, vData, iData);
    //z axis
    GLUtility::fillCube(0.1, 0.1f, 8.f, Color::blue, m, vData, iData);

    groupObj = std::make_shared<GLUtility::Mesh>(vData, iData);
    groupObj->trans = glm::mat4(1);
    pvColrObjs.push_back(groupObj);

    {
        std::vector<GLUtility::VDPosNormColr> vData;
        std::vector<unsigned int> iData;
        GLUtility::fillCube(1.f, 1.f, 1.f, glm::vec3(0.5f, 0.5f, 1.0f), m, vData, iData);
        auto mesh = std::make_shared<GLUtility::Mesh>(vData, iData);
        mesh->trans = glm::translate(glm::mat4(1), glm::vec3(-1, 0, 0));
        mesh->rot = glm::rotate(glm::mat4(1),glm::radians(30.f),GLUtility::Y_AXIS);
        mesh->scle = glm::scale(glm::mat4(1), glm::vec3(0.75f));
        mesh->tMatrix = glm::mat4(1);
        leftObj = mesh;
        pvColrObjs.push_back(mesh);
    }

    {
        std::vector<GLUtility::VDPosNormColr> vData;
        std::vector<unsigned int> iData;
        GLUtility::fillCube(1.f, 3.f, 3.f, glm::vec3(0.5f, 1.f, 1.0f), m, vData, iData);
        auto mesh = std::make_shared<GLUtility::Mesh>(vData, iData);
        mesh->trans = glm::translate(glm::mat4(1), glm::vec3(1, 0, 0));
        mesh->rot = glm::rotate(glm::mat4(1), glm::radians(-30.f), GLUtility::Y_AXIS);
        mesh->scle = glm::scale(glm::mat4(1), glm::vec3(0.75f, 0.3f, 0.75f));
        mesh->tMatrix = glm::mat4(1);
        pvColrObjs.push_back(mesh);
    }
   
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
    
    lBox->tMatrix = glm::translate(glm::mat4(1), light.position);
    camera->orbitY(glm::radians(gRotation));
    globalModelMat = glm::rotate(glm::mat4(1),glm::radians(xRotation),GLUtility::X_AXIS);
}

void renderFrame()
{
   
    glBindFramebuffer(GL_FRAMEBUFFER, layer1->fbo);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    
    
    basicProgram->bind();
    
    basicProgram->setMat4f("view", camera->viewMat);
    basicProgram->setMat4f("proj", projectionMat);
    basicProgram->setVec3f("color", Color::red);

    basicProgram->setVec3f("lightPos", light.position);
    basicProgram->setVec3f("lightColr", light.diffuse);
    basicProgram->setFloat("lReach", 3.f);

    basicProgram->activeSubrountine("getColr", GL_FRAGMENT_SHADER);
    glm::mat4 mv;
    
    for (auto& obj : defaultMatObjs)
    {
        mv = obj->tMatrix * globalModelMat;
        auto normlMat = glm::transpose(glm::inverse(mv));
        basicProgram->setMat4f("model", mv);
        basicProgram->setMat4f("nrmlMat", normlMat);
        basicProgram->setVec3f("color", glm::vec3(obj->color));
        basicProgram->bindAllUniforms();
        obj->draw();
    }

    basicProgram->activeSubrountine("getDiffuseColr", GL_FRAGMENT_SHADER);
    for (auto& obj : diffuseMatObjs)
    {
        mv = obj->tMatrix * globalModelMat;
        auto normlMat = glm::transpose(glm::inverse(mv));
        basicProgram->setMat4f("model", mv);
        basicProgram->setMat4f("nrmlMat", normlMat);
        basicProgram->setVec3f("color", glm::vec3(obj->color));
        basicProgram->bindAllUniforms();
        obj->draw();
    }

    for (auto& obj : pvColrObjs)
    {
        basicProgram->activeSubrountine("getDiffuseColrVertex", GL_FRAGMENT_SHADER);
        //decltype(groupObj) obj = groupObj;
        auto m = obj->trans * obj->rot * obj->scle;
        mv =  m* globalModelMat;
        auto normlMat = glm::transpose(glm::inverse(mv));
        basicProgram->setMat4f("model", mv);
        basicProgram->setMat4f("nrmlMat", normlMat);
        basicProgram->setVec3f("color", glm::vec3(obj->color));
        basicProgram->bindAllUniforms();
        obj->draw();
    }
    basicProgram->unbind();
    

   

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
        ImGui::Begin("Lights Demo");                     

        ImGui::Text("Use UP & DOWN arrows for zoom-In&Out");
        ImGui::SliderFloat("Norm Distance", &tLength, 0, 1);
        ImGui::SliderFloat("Scene Y Rotation", &gRotation, 0, 360);
        ImGui::SliderFloat("Scene X Rotation", &xRotation, -180, 180);
        //ImGui::Text("Use Lef mouse click to pick triangle");
        
        
        ImGui::Combo("Boolean op", &cOP, bOplist,IM_ARRAYSIZE(bOplist), 2);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();
    }
    

    {
        ImGui::Begin("Light params");
        ImGui::SliderFloat3("Position", &light.position[0], -8.0f, 8.0f);
        ImGui::SliderFloat3("Diffuse", &light.diffuse[0], 0, 1);
        
        ImGui::SliderFloat("Cam speed", &camSpeed, 0, 5);
        //ImGui::SliderFloat("outerCutOff", &light.outerCutOff, 0, 90);

        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


std::shared_ptr<ObjContainer> loadObjModel(std::string filePath,std::string defaultDiffuse)
{
    auto filename = filePath;// "assets/cube/Cube.obj";
    //get base path
    auto pos = filePath.find_last_of('/');
    std::string basepath = "";
    if(pos!=std::string::npos)
    {
        basepath = filePath.substr(0, pos);
    }
    
    auto triangulate = false;
    std::cout << "Loading " << filename << std::endl;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(), basepath.c_str());

    auto objContainer = std::make_shared<ObjContainer>();
    vector<std::shared_ptr<ObjShape>> objParts;
    vector<VertexData> vData;
    vector<unsigned int> iData;

    glm::vec3 bbMin=glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 bbMax=glm::vec3(-std::numeric_limits<float>::max());

    struct MGroup
    {
        int bInd = -1;//begin index
        int eInd = -1;//end index
        int mId = -1;//mat id
    };
    vector<MGroup> groups;
    map<string, GLuint> texList;
    MGroup mg;
    groups.push_back(mg);
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) 
    {
        // Loop over faces(polygon)
        int pMid = -1;
        auto part = std::make_shared<ObjShape>();
        auto shape = shapes.at(s);
        
        const size_t numIndices = shape.mesh.indices.size();
        for (size_t i = 0; i < numIndices; i += 3)
        {
            for (size_t id=i;id<i+3;id++)
            {
                auto idx = shape.mesh.indices.at(id);
                //vertices
                VertexData vd;

                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                bbMin.x = std::min(vx, bbMin.x);
                bbMin.y = std::min(vy, bbMin.y);
                bbMin.z = std::min(vz, bbMin.z);

                bbMax.x = std::max(vx, bbMax.x);
                bbMax.y = std::max(vy, bbMax.y);
                bbMax.z = std::max(vz, bbMax.z);

                vd.pos = glm::vec3(vx, vy, vz);

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    vd.norm = glm::vec3(nx, ny, nz);
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    vd.uv = glm::vec2(tx, ty);
                }
                vData.push_back(vd);
                iData.push_back((unsigned int)iData.size());
            }

            if (materials.size() > 0)
            {
                auto materialID = shape.mesh.material_ids.at(i / 3);
                assert(materialID >= 0);

                size_t fv = size_t(shapes[s].mesh.num_face_vertices[i / 3]);
                if (pMid == -1)
                {
                    groups.at(groups.size() - 1).bInd = (int)(iData.size() - fv);
                    groups.at(groups.size() - 1).mId = materialID;
                }
                else
                {
                    if (pMid != materialID)
                    {
                        //groups.at(groups.size() - 1).eInd = iData.size();
                        MGroup mg;
                        mg.mId = materialID;
                        mg.bInd = (int)(iData.size() - fv);
                        groups.push_back(mg);
                    }
                    else
                    {
                        groups.at(groups.size() - 1).eInd = (int)iData.size();
                    }
                    if ((i / 3) == shape.mesh.num_face_vertices.size() - 1)
                    {
                        groups.at(groups.size() - 1).eInd = (int)iData.size();
                    }
                }
                pMid = materialID;
            }
            else
            {
                groups.at(groups.size() - 1).bInd = (int)(0);
                groups.at(groups.size() - 1).mId = 0;
                groups.at(groups.size() - 1).eInd = (int)iData.size();
            }
        }
        

        std::map<unsigned int, unsigned int> imap;
        decltype(rangesPerTex) rangerPerTexTemp;
        decltype(rangesPerTex) rangerPerTexLocal;

        for (auto gp : groups)
        {
            auto diff = (gp.eInd - gp.bInd);
            DrawRange dr;
            dr.offset = gp.bInd;
            dr.drawCount = diff;
            ranges.push_back(dr);
            if (imap.find(gp.mId) == imap.end())
            {
                imap[gp.mId] = 1;
                vector<DrawRange> rngs;
                rngs.push_back(dr);
                rangerPerTexTemp[gp.mId] = (rngs);
            }
            else
            {
                rangerPerTexTemp[gp.mId].push_back(dr);
                imap[gp.mId] += 1;
            }

        }

        if (materials.size()>0)
        {
            for (auto i : rangerPerTexTemp)
            {
                auto diffuseTex = materials.at(i.first).diffuse_texname;
                GLuint dTexID;
                if (diffuseTex.size() > 0)
                {

                    if (texList.find(diffuseTex) == texList.end())
                    {
                        if (basepath.length() > 0)
                            dTexID = GLUtility::makeTexture(basepath + std::string("/") + diffuseTex);
                        else
                            dTexID = GLUtility::makeTexture(diffuseTex);
                        texList[diffuseTex] = dTexID;
                    }
                    else
                        dTexID = texList[diffuseTex];
                    rangerPerTexLocal[dTexID] = i.second;
                }

                auto specTex = materials.at(i.first).diffuse_texname;
                //ToDo : remove duplicate code
                if (specTex.size() > 0)
                {
                    GLuint tex;
                    if (texList.find(diffuseTex) == texList.end())
                    {
                        tex = GLUtility::makeTexture(basepath + std::string("/") + specTex);
                        texList[specTex] = tex;
                    }
                    else
                        tex = texList[specTex];
                    for (size_t r = 0; r < rangerPerTexLocal[dTexID].size(); r++)
                    {
                        rangerPerTexLocal[dTexID].at(r).sTexID = tex;
                    }
                }
            }
        }
        else
        {
            GLuint dTexID;
            for (auto i : rangerPerTexTemp)
            {
                if (texList.find(defaultDiffuse) == texList.end())
                {
                    if (basepath.length() > 0)
                        dTexID = GLUtility::makeTexture(defaultDiffuse);
                    else
                        dTexID = GLUtility::makeTexture(defaultDiffuse);
                    texList[defaultDiffuse] = dTexID;
                }
                else
                    dTexID = texList[defaultDiffuse];
                rangerPerTexLocal[dTexID] = i.second;
            }

        }
        auto mesh = std::make_shared<Mesh>(vData, iData);
        part->mesh = mesh;
        part->rangesPerTex = rangerPerTexLocal;

        objParts.push_back(part);
    }

    auto mid = (bbMin + bbMax) / 2.0f;
    objContainer->objParts = objParts;
    //always follow Trans * Rotation * Scale
    objContainer->tMatrix = glm::translate(glm::mat4(1), glm::vec3(-mid.x,0,-mid.z))*glm::scale(glm::mat4(1),glm::vec3(2));
    objContainer->bBox = GLUtility::getBoudingBox(bbMin, bbMax);
    objContainer->bBox->tMatrix = objContainer->tMatrix;
    objContainer->bbMin = bbMin;
    objContainer->bbMax = bbMax;
    
    return objContainer;
    //objModels.push_back(objContainer);
}

void loadObj(std::string filePath, std::vector<double>& vData, std::vector<unsigned int>& iData)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str(), nullptr);

    if (!ret)
    {
        fmt::print("handle it");
    }
    attrib.vertices;
    
    for (const auto& v : attrib.vertices)
    {
        vData.push_back(static_cast<double>(v));
    }
    //vData.assign(attrib.vertices.begin(), attrib.vertices.end());
    
    
    /*glm::vec3 bbMin = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 bbMax = glm::vec3(-std::numeric_limits<float>::max());*/

    for (size_t s = 0; s < shapes.size(); s++)
    {
        // Loop over faces(polygon)
        int pMid = -1;
        auto part = std::make_shared<ObjShape>();
        auto shape = shapes.at(s);

        const size_t numIndices = shape.mesh.indices.size();
        for (const auto& index : shape.mesh.indices)
        {
            iData.push_back(index.vertex_index);
        }
        //iData.assign(shape.mesh.indices.begin(), shape.mesh.indices.end());
        break;
        //for (size_t i = 0; i < numIndices; i += 3)
        //{
        //    for (size_t id = i; id < i + 3; id++)
        //    {
        //        auto idx = shape.mesh.indices.at(id);
        //        //vertices

        //        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        //        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        //        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

        //        bbMin.x = std::min(vx, bbMin.x);
        //        bbMin.y = std::min(vy, bbMin.y);
        //        bbMin.z = std::min(vz, bbMin.z);

        //        bbMax.x = std::max(vx, bbMax.x);
        //        bbMax.y = std::max(vy, bbMax.y);
        //        bbMax.z = std::max(vz, bbMax.z);

        //        auto pos = glm::vec3(vx, vy, vz);

        //        vData.push_back(pos);
        //        iData.push_back((unsigned int)iData.size());
        //    }
        //}
    }
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

