
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

struct MeshView
{
    std::shared_ptr<glm::mat4> tMat = std::make_shared<glm::mat4>(1);
    std::shared_ptr<glm::mat4> parentMat = std::make_shared<glm::mat4>(1);
    glm::vec3 color, rOrigin;
    size_t mIndex;
    bool vert0 = false, vert1 = false, vert2 = false;
    //Each grp will have max of 6 entries
    //where each entry represents index of meshview 
    //and side(0,1,2) of the meshview
    std::vector<glm::ivec2> v0Grp, v1Grp, v2Grp;
    float srcTh, dstTh, speed;
    bool visible = true;
};

struct CheapMap
{
    std::vector<glm::vec3> entries;
    std::vector<size_t> counts;
    size_t push(glm::vec3 v)
    {
        auto ec = entryCount(v);
        if (ec == std::numeric_limits<size_t>::max())
        {
            entries.push_back(v);
            counts.push_back(1);
            return 1;
        }
        else
        {
            counts.at(ec) += 1;
            return counts.at(ec);
        }
    }

    size_t getCount(glm::vec3 v)
    {
        auto ec = entryCount(v);
        if (ec != std::numeric_limits<size_t>::max())
        {
            return counts.at(ec);
        }

        return 0;
    }

    size_t entryCount(glm::vec3 v)
    {
        auto itr = std::find(entries.begin(), entries.end(), v);
        if (itr != entries.end())
        {
            auto ind = itr - entries.begin();
            return ind;
        }
        return std::numeric_limits<size_t>::max();
    }
};
CheapMap cheapMap;
struct AnimEntity
{
    float srcTh, dstTh, speed;
    size_t index;
};

struct Edge
{
    glm::vec3 eo;
    glm::vec3 e1;
};

GLFWwindow* window;
auto closeWindow = false;
auto startAnim = true;
auto testFlag = false;


std::shared_ptr<Camera> camera;
float fov = 45;
glm::mat4 projectionMat = glm::perspective(fov, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 100.0f);
glm::mat4 globalModelMat = glm::mat4(1);



std::shared_ptr<GlslProgram> basicProgram, dirLightProgram, geomNormProgram, dirLvColorProgram;
std::vector<std::shared_ptr<Mesh>> scenObjects, defaultMatObjs, geomShaderObjs;
std::vector<std::shared_ptr<MeshView>> meshGroup;
std::vector<AnimEntity> animQueue;
std::shared_ptr<Mesh> lBox, rootMesh, majorMesh;
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

float gRotation = 0;
float tLength = 0.1;
float camSpeed = 0.1;

std::array<glm::vec3, 3> verts;
std::vector<std::shared_ptr<MeshView>> queue;

#pragma endregion 


#pragma region prototypes

void createWindow();
void initGL();
void cleanGL();
void initImgui();
void setupCamera();
void setupScene();
std::shared_ptr<FrameBuffer> getFboMSA(std::shared_ptr<FrameBuffer> refFbo, int samples);
void updateFrame();
void renderFrame();
void renderImgui();


std::shared_ptr<ObjContainer> loadObjModel(std::string filePath, std::string defaultDiffuse = "assets/textures/default.jpg");


#pragma endregion


#pragma region functions
glm::vec3 roundTo4(glm::vec3 v)
{
    glm::vec3 resV;
    resV.x = (int)(v.x * 10000 + .5);
    resV.x = (float)resV.x / 10000;
    resV.y = (int)(v.y * 10000 + .5);
    resV.y = (float)resV.y / 10000;
    resV.z = (int)(v.z * 10000 + .5);
    resV.z = (float)resV.z / 10000;

    return resV;
}
void createOrUpdateBox(glm::vec3 pos, size_t cCount)
{
    if (cCount == 1)
    {
        auto lBox = GLUtility::getCubeVec3(0.2f, 0.2f, 0.2f);
        lBox->color = glm::vec4(0.1, 0.1, 0.1, 1);
        lBox->tMatrix = glm::translate(glm::mat4(1), pos);
        defaultMatObjs.push_back(lBox);
    }
    else
    {
        auto index = cheapMap.entryCount(roundTo4(pos));
        if (index < defaultMatObjs.size())
            defaultMatObjs.at(index + 1)->color = glm::vec4(cCount * 0.1, cCount * 0.1, (cCount == 10) ? 0 : cCount * 0.1, 1);
    }
}
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
        testFlag = true;
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
    dirLightProgram = std::make_shared<GlslProgram>(
        Utility::readFileContents("shaders/vMatSpotLight.glsl"),
        Utility::readFileContents("shaders/fSpotLight.glsl")
        );
    dirLvColorProgram = std::make_shared<GlslProgram>(
        Utility::readFileContents("shaders/vSpotLight_vColor.glsl"),
        Utility::readFileContents("shaders/fSpotLight_vColor.glsl")
        );
    /*geomNormProgram = std::make_shared<GlslProgram>(
        Utility::readFileContents("shaders/gs/vNorm.glsl"),
        Utility::readFileContents("shaders/gs/gNorm.glsl"),
        Utility::readFileContents("shaders/gs/fNorm.glsl")
        );*/

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

    auto eye = glm::vec3(0, 10, 10);
    auto center = glm::vec3(0, 0, 0);
    auto up = glm::vec3(0, 1, 0);
    camera = std::make_shared<Camera>(eye, center, up);

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
    light.cutOff = 80.0f;
    light.outerCutOff = 90.0f;


    /*
    2.000000,0.000000,0.000000
    -1.000000,0.000000,1.732051
    2.000000,0.000000,3.464102
    */


    size_t ind = 0;
    for (float th = 360; th > 0; th -= 120)
    {
        float x = 2 * cos(glm::radians(th));
        float z = 2 * sin(glm::radians(th));
        verts[ind] = glm::vec3(x, 0, z);
        ind++;
    }
    /*verts[0] = glm::vec3(2.000000, 0.000000, 0.000000);
    verts[1] = glm::vec3(- 1.000000, 0.000000, 1.732051);
    verts[2] = glm::vec3(2.000000, 0.000000, 3.464102);*/


    rootMesh = GLUtility::getTri(verts);
    rootMesh->drawCommand = GL_TRIANGLES;
    rootMesh->color = glm::vec4(Color::orange, 1.);

    auto rootView = std::make_shared<MeshView>();// , mView2, mView3, mView4;
    rootView->mIndex = 0;
    rootView->rOrigin = glm::vec3(0);// glm::vec3(glm::translate(glm::mat4(1), verts[0]) * glm::rotate(glm::mat4(1), glm::radians(60.0f), GLUtility::Y_AXIS) * glm::translate(glm::mat4(1), -verts[0]) * glm::vec4(0, 0, 0, 1));
    rootView->srcTh = 45;
    rootView->dstTh = 45;
    rootView->speed = 0.05;
    rootView->color = Color::orange;
    rootView->visible = false;


    queue.push_back(rootView);


    auto normal = GLUtility::getNormal(verts[0], verts[1], verts[2]);
    normal = glm::normalize(normal);
    auto tMatrix = glm::translate(glm::mat4(1), rootView->rOrigin) * glm::rotate(glm::mat4(1), glm::radians(rootView->srcTh), GLUtility::Y_AXIS) * glm::translate(glm::mat4(1), -rootView->rOrigin);
    std::vector<GLUtility::VDPosNormColr> vData;
    for (size_t i = 0; i < 3; i++)
        vData.push_back({ glm::vec3(tMatrix * glm::vec4(verts[i],1)),normal,Color::orange });

    std::vector<unsigned int> iData{ 0,1,2 };
    majorMesh = std::make_shared<Mesh>(vData, iData);



    lBox = GLUtility::getCubeVec3(0.2f, 0.2f, 0.2f);
    lBox->color = glm::vec4(1.0);
    lBox->tMatrix = glm::translate(glm::mat4(1), light.position);
    lBox->pickColor = glm::vec4(1.0);

    auto xAxis = GLUtility::getCube(100, 0.1f, 0.1f);
    xAxis->color = glm::vec4(Color::red, 1.0);
    auto yAxis = GLUtility::getCube(0.1, 100, 0.1f);
    yAxis->color = glm::vec4(Color::green, 1.0);
    auto zAxis = GLUtility::getCube(0.1, 0.1f, 100);
    zAxis->color = glm::vec4(Color::blue, 1.0);



    //geomShaderObjs.push_back(triMesh);
    defaultMatObjs.push_back(lBox);
    //defaultMatObjs.push_back(tBox);
    scenObjects.push_back(xAxis);
    scenObjects.push_back(yAxis);
    scenObjects.push_back(zAxis);
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
    auto t = glfwGetTime() * 20;
    auto theta = (int)t % 360;

    tLength = glm::radians((float)t);

    lBox->tMatrix = glm::translate(glm::mat4(1), light.position);

    if (startAnim)
        if (animQueue.size() > 0)
        {
            if (animQueue.at(0).srcTh < animQueue.at(0).dstTh)
            {
                animQueue.at(0).srcTh += animQueue.at(0).speed;
                meshGroup.at(animQueue.at(0).index)->srcTh = animQueue.at(0).srcTh;
            }
            else
            {
                auto obj = meshGroup.at(animQueue.at(0).index);
                obj->visible = false;
                auto tMatrix = glm::translate(glm::mat4(1), obj->rOrigin) * glm::rotate(glm::mat4(1), glm::radians(obj->srcTh), GLUtility::Y_AXIS) * glm::translate(glm::mat4(1), -obj->rOrigin) * (*obj->parentMat.get());
                auto rVert0 = glm::vec3(tMatrix * glm::vec4(verts[0], 1));
                auto rVert1 = glm::vec3(tMatrix * glm::vec4(verts[1], 1));
                auto rVert2 = glm::vec3(tMatrix * glm::vec4(verts[2], 1));

                auto normal = GLUtility::getNormal(rVert0, rVert1, rVert2);
                normal = glm::normalize(normal);
                auto& vData = majorMesh->vdPosNrmClr;
                vData.push_back({ rVert0,normal,obj->color });
                vData.push_back({ rVert1,normal,obj->color });
                vData.push_back({ rVert2,normal,obj->color });

                auto& iData = majorMesh->iData;
                iData.push_back(iData.size());
                iData.push_back(iData.size());
                iData.push_back(iData.size());

                majorMesh->drawCount = iData.size();

                glBindBuffer(GL_ARRAY_BUFFER, majorMesh->vbo);
                glBufferData(GL_ARRAY_BUFFER, vData.size() * sizeof(VDPosNormColr), nullptr, GL_STREAM_DRAW);
                glBufferData(GL_ARRAY_BUFFER, vData.size() * sizeof(VDPosNormColr), vData.data(), GL_STREAM_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, majorMesh->ibo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, iData.size() * sizeof(GL_UNSIGNED_INT), nullptr, GL_STREAM_DRAW);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, iData.size() * sizeof(GL_UNSIGNED_INT), iData.data(), GL_STREAM_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

                animQueue.erase(animQueue.begin());
            }
        }
        else
        {
            static size_t cnt = 0, ptCnt = 0;
            if (queue.size() > 0/*&& cnt<50*/)
            {
                if (cnt == 6)
                    cnt += 0;
                cnt += 1;
                auto rootView = queue.at(0);
                auto tMatrix = glm::translate(glm::mat4(1), rootView->rOrigin) * glm::rotate(glm::mat4(1), glm::radians(rootView->dstTh), GLUtility::Y_AXIS) * glm::translate(glm::mat4(1), -rootView->rOrigin) * (*rootView->parentMat.get());
                auto rVert0 = glm::vec3(tMatrix * glm::vec4(verts[0], 1));
                auto rVert1 = glm::vec3(tMatrix * glm::vec4(verts[1], 1));
                auto rVert2 = glm::vec3(tMatrix * glm::vec4(verts[2], 1));

                auto tc0 = cheapMap.getCount(roundTo4(rVert0));
                auto tc1 = cheapMap.getCount(roundTo4(rVert1));
                auto tc2 = cheapMap.getCount(roundTo4(rVert2));
                queue.erase(queue.begin());

                meshGroup.push_back(rootView);

                if (!rootView->vert0)
                {
                    rootView->vert0 = true;
                    auto testViewRed = std::make_shared<MeshView>();
                    testViewRed->vert1 = true;
                    testViewRed->parentMat = std::make_shared<glm::mat4>(tMatrix);
                    testViewRed->rOrigin = glm::vec3(tMatrix * glm::vec4(verts[0], 1));
                    testViewRed->srcTh = 0;
                    testViewRed->dstTh = 60;
                    testViewRed->mIndex = meshGroup.size();
                    auto cInd = meshGroup.size() % Color::totalColors;
                    testViewRed->color = Color::colrArr[cInd];;
                    auto redtMatrix = glm::translate(glm::mat4(1), testViewRed->rOrigin) * glm::rotate(glm::mat4(1), glm::radians(testViewRed->dstTh), GLUtility::Y_AXIS) * glm::translate(glm::mat4(1), -testViewRed->rOrigin) * (*testViewRed->parentMat.get());
                    auto redVert0 = glm::vec3(redtMatrix * glm::vec4(verts[0], 1));
                    auto redVert1 = glm::vec3(redtMatrix * glm::vec4(verts[1], 1));

                    meshGroup.push_back(testViewRed);
                    queue.push_back(testViewRed);

                    //side 2 of rootView
                    rootView->v0Grp.push_back(glm::ivec2(meshGroup.size() - 1, 1));
                    rootView->v2Grp.push_back(glm::ivec2(meshGroup.size() - 1, 1));

                    //side 1 of testViewRed
                    testViewRed->v0Grp.push_back(glm::ivec2(rootView->mIndex, 2));
                    testViewRed->v1Grp.push_back(glm::ivec2(rootView->mIndex, 2));


                    AnimEntity aeRed;
                    aeRed.srcTh = 0;
                    aeRed.dstTh = 60;
                    aeRed.speed = 2;
                    aeRed.index = meshGroup.size() - 1;
                    animQueue.push_back(aeRed);
                }

                if (!rootView->vert1)
                {
                    rootView->vert1 = true;
                    auto testViewGreen = std::make_shared<MeshView>();
                    testViewGreen->vert2 = true;
                    testViewGreen->parentMat = std::make_shared<glm::mat4>(tMatrix);;
                    testViewGreen->rOrigin = glm::vec3(tMatrix * glm::vec4(verts[1], 1));
                    testViewGreen->srcTh = 0;
                    testViewGreen->dstTh = 60;
                    auto cInd = meshGroup.size() % Color::totalColors;
                    testViewGreen->color = Color::colrArr[cInd];// Color::green;
                    testViewGreen->mIndex = meshGroup.size();
                    auto greentMatrix = glm::translate(glm::mat4(1), testViewGreen->rOrigin) * glm::rotate(glm::mat4(1), glm::radians(testViewGreen->dstTh), GLUtility::Y_AXIS) * glm::translate(glm::mat4(1), -testViewGreen->rOrigin) * (*testViewGreen->parentMat.get());
                    auto greenVert1 = glm::vec3(greentMatrix * glm::vec4(verts[1], 1));
                    auto greenVert2 = glm::vec3(greentMatrix * glm::vec4(verts[2], 1));
                    meshGroup.push_back(testViewGreen);
                    queue.push_back(testViewGreen);

                    //side 0 of rootView
                    rootView->v0Grp.push_back(glm::ivec2(meshGroup.size() - 1, 1));
                    rootView->v1Grp.push_back(glm::ivec2(meshGroup.size() - 1, 1));
                    
                    //side 1 of testViewGreen
                    testViewGreen->v1Grp.push_back(glm::ivec2(rootView->mIndex, 0));
                    testViewGreen->v2Grp.push_back(glm::ivec2(rootView->mIndex, 0));

                    AnimEntity aeGreen;
                    aeGreen.srcTh = 0;
                    aeGreen.dstTh = 60;
                    aeGreen.speed = 2;
                    aeGreen.index = meshGroup.size() - 1;
                    animQueue.push_back(aeGreen);
                }

                if (!rootView->vert2
                    && cheapMap.getCount(roundTo4(rVert2)) < 10
                    && cheapMap.getCount(roundTo4(rVert1)) < 10)
                {
                    rootView->vert2 = true;
                    auto testViewBlue = std::make_shared<MeshView>();
                    testViewBlue->vert0 = true;
                    testViewBlue->parentMat = std::make_shared<glm::mat4>(tMatrix);;
                    testViewBlue->rOrigin = glm::vec3(tMatrix * glm::vec4(verts[2], 1));
                    testViewBlue->srcTh = 0;
                    testViewBlue->dstTh = 60;
                    auto cInd = meshGroup.size() % Color::totalColors;
                    testViewBlue->color = Color::colrArr[cInd];// Color::blue;
                    testViewBlue->mIndex = meshGroup.size();
                    auto bluetMatrix = glm::translate(glm::mat4(1), testViewBlue->rOrigin) * glm::rotate(glm::mat4(1), glm::radians(testViewBlue->dstTh), GLUtility::Y_AXIS) * glm::translate(glm::mat4(1), -testViewBlue->rOrigin) * (*testViewBlue->parentMat.get());
                    auto blueVert0 = glm::vec3(bluetMatrix * glm::vec4(verts[0], 1));
                    auto blueVert2 = glm::vec3(bluetMatrix * glm::vec4(verts[2], 1));
                    meshGroup.push_back(testViewBlue);
                    queue.push_back(testViewBlue);

                    //side 1 of rootView
                    rootView->v1Grp.push_back(glm::ivec2(meshGroup.size() - 1, 2));
                    rootView->v2Grp.push_back(glm::ivec2(meshGroup.size() - 1, 2));

                    //side 2 of testViewBlue
                    testViewBlue->v0Grp.push_back(glm::ivec2(rootView->mIndex, 1));
                    testViewBlue->v2Grp.push_back(glm::ivec2(rootView->mIndex, 1));

                    AnimEntity aeBlue;
                    aeBlue.srcTh = 0;
                    aeBlue.dstTh = 60;
                    aeBlue.speed = 2;
                    aeBlue.index = meshGroup.size() - 1;
                    animQueue.push_back(aeBlue);
                }
            }
        }

}

void renderFrame()
{

    glBindFramebuffer(GL_FRAMEBUFFER, layer1->fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    dirLightProgram->bind();

    dirLightProgram->setMat4f("view", camera->viewMat);
    dirLightProgram->setMat4f("proj", projectionMat);

    dirLightProgram->setVec3f("viewPos", camera->eye);


    dirLightProgram->setVec3f("material.specular", glm::vec3(0.5));

    dirLightProgram->setFloat("material.shininess", 64.0f);

    auto lightColor = Color::white;
    glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
    glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
    dirLightProgram->setVec3f("light.position", light.position);
    dirLightProgram->setVec3f("light.ambient", light.ambient);
    dirLightProgram->setVec3f("light.diffuse", light.diffuse);
    dirLightProgram->setVec3f("light.specular", light.specular);

    dirLightProgram->setFloat("light.constant", light.constant);
    dirLightProgram->setFloat("light.linear", light.linear);
    dirLightProgram->setFloat("light.quadratic", light.quadratic);

    dirLightProgram->setVec3f("light.direction", glm::vec3(0, -1, 0));
    dirLightProgram->setFloat("light.cutOff", glm::cos(glm::radians(light.cutOff)));
    dirLightProgram->setFloat("light.outerCutOff", glm::cos(glm::radians(light.outerCutOff)));

    dirLightProgram->setFloat("nDist", tLength);

    for (const auto& obj : scenObjects)
    {
        auto mv = obj->tMatrix * globalModelMat;
        auto nrmlMat = glm::transpose(glm::inverse(mv));
        dirLightProgram->setVec3f("material.diffuse", obj->color);
        dirLightProgram->setMat4f("model", mv);
        dirLightProgram->setMat4f("nrmlMat", nrmlMat);
        dirLightProgram->bindAllUniforms();
        obj->draw();
    }

    glBindVertexArray(rootMesh->vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rootMesh->ibo);
    for (const auto& obj : meshGroup)
    {
        if (!obj->visible)
            continue;

        auto tMatrix = glm::translate(glm::mat4(1), obj->rOrigin) * glm::rotate(glm::mat4(1), glm::radians(obj->srcTh), GLUtility::Y_AXIS) * glm::translate(glm::mat4(1), -obj->rOrigin);
        auto mv = tMatrix * (*obj->parentMat.get()) * globalModelMat;

        auto nrmlMat = glm::transpose(glm::inverse(mv));
        dirLightProgram->setVec3f("material.diffuse", obj->color);
        dirLightProgram->setMat4f("model", mv);
        dirLightProgram->setMat4f("nrmlMat", nrmlMat);
        dirLightProgram->bindAllUniforms();

        glDrawElements(rootMesh->drawCommand, rootMesh->drawCount, GL_UNSIGNED_INT, (void*)0);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    dirLightProgram->unbind();

    glm::mat4 mv;

    dirLvColorProgram->bind();

    dirLvColorProgram->setMat4f("view", camera->viewMat);
    dirLvColorProgram->setMat4f("proj", projectionMat);
    dirLvColorProgram->setVec3f("viewPos", camera->eye);


    dirLvColorProgram->setVec3f("material.specular", glm::vec3(0.5));

    dirLvColorProgram->setFloat("material.shininess", 64.0f);

    lightColor = Color::white;
    diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
    ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
    dirLvColorProgram->setVec3f("light.position", light.position);
    dirLvColorProgram->setVec3f("light.ambient", light.ambient);
    dirLvColorProgram->setVec3f("light.diffuse", light.diffuse);
    dirLvColorProgram->setVec3f("light.specular", light.specular);

    dirLvColorProgram->setFloat("light.constant", light.constant);
    dirLvColorProgram->setFloat("light.linear", light.linear);
    dirLvColorProgram->setFloat("light.quadratic", light.quadratic);

    dirLvColorProgram->setVec3f("light.direction", glm::vec3(0, -1, 0));
    dirLvColorProgram->setFloat("light.cutOff", glm::cos(glm::radians(light.cutOff)));
    dirLvColorProgram->setFloat("light.outerCutOff", glm::cos(glm::radians(light.outerCutOff)));

    auto tMatrix = glm::mat4(1);
    mv = tMatrix * globalModelMat;

    auto nrmlMat = glm::transpose(glm::inverse(mv));
    dirLvColorProgram->setMat4f("model", mv);
    dirLvColorProgram->setMat4f("nrmlMat", nrmlMat);
    dirLvColorProgram->bindAllUniforms();
    majorMesh->draw();

    dirLvColorProgram->unbind();


    basicProgram->bind();

    basicProgram->setMat4f("view", camera->viewMat);
    basicProgram->setMat4f("proj", projectionMat);
    basicProgram->setVec3f("color", Color::red);



    for (auto& obj : defaultMatObjs)
    {
        mv = obj->tMatrix * globalModelMat;
        basicProgram->setMat4f("model", mv);
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
        ImGui::Begin("Dynamic tile demo");

        ImGui::Checkbox("Start animation", &startAnim);
        ImGui::Text("Use UP & DOWN arrows for zoom-In&Out");
        ImGui::SliderFloat("Norm Distance", &tLength, 0, 1);
        ImGui::SliderFloat("Scene Y Rotation", &gRotation, 0, 360);
        //ImGui::Text("Use Lef mouse click to pick triangle");

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();
    }


    {
        ImGui::Begin("Light params");
        ImGui::SliderFloat3("Position", &light.position[0], 0, 15);
        ImGui::SliderFloat3("Ambient", &light.ambient[0], 0, 1);
        ImGui::SliderFloat3("Diffuse", &light.diffuse[0], 0, 1);

        ImGui::SliderFloat("CutOff", &light.cutOff, 0, 90);
        ImGui::SliderFloat("outerCutOff", &light.outerCutOff, 0, 90);

        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

std::shared_ptr<ObjContainer> loadObjModel(std::string filePath, std::string defaultDiffuse)
{
    auto filename = filePath;// "assets/cube/Cube.obj";
    //get base path
    auto pos = filePath.find_last_of('/');
    std::string basepath = "";
    if (pos != std::string::npos)
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

    glm::vec3 bbMin = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 bbMax = glm::vec3(-std::numeric_limits<float>::max());

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
            for (size_t id = i; id < i + 3; id++)
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

        if (materials.size() > 0)
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
    objContainer->tMatrix = glm::translate(glm::mat4(1), glm::vec3(-mid.x, 0, -mid.z)) * glm::scale(glm::mat4(1), glm::vec3(2));
    objContainer->bBox = GLUtility::getBoudingBox(bbMin, bbMax);
    objContainer->bBox->tMatrix = objContainer->tMatrix;
    objContainer->bbMin = bbMin;
    objContainer->bbMax = bbMax;

    return objContainer;
    //objModels.push_back(objContainer);
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

