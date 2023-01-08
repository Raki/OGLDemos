
#include "CommonHeaders.h"
#include "GlslProgram.h"
#include "GLUtility.h"
#include "Utility.h"
#include "Colors.h"
#include "Materials.h"
#include "Camera.h"

#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/util/memory.hpp>
#include <osmium/visitor.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/geom/mercator_projection.hpp>
#include <osmium/geom/tile.hpp>
#include <osmium/geom/wkt.hpp>
#include <osmium/geom/factory.hpp>

#include <poly2tri/poly2tri.h>

using namespace GLUtility;

#pragma region vars

const int WIN_WIDTH = 1920;
const int WIN_HEIGHT = 1080;


GLFWwindow* window;
auto closeWindow = false;

std::shared_ptr<Camera> camera;
float fov=45;
glm::mat4 projectionMat = glm::perspective(fov, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 10000.0f);
glm::mat4 globalModelMat = glm::mat4(1);

std::shared_ptr<GlslProgram> basicProgram;
std::shared_ptr<Mesh> streetMap;
std::shared_ptr<FrameBuffer> layer1;

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

float gRotation = 90;
glm::vec3 gOrigin = glm::vec3(0);

struct NodeGLInfo
{
    glm::dvec3 pos;
    unsigned int ind;
};

std::map<int64_t, NodeGLInfo> osmNodes;
std::vector<glm::vec3> nodePositions;
std::vector<unsigned int> nodeIndices;
std::vector<DrawRange> waysData;
const double DMAX = std::numeric_limits<double>::max();
glm::dvec2 mercMax=glm::dvec3(- DMAX);
glm::dvec2 mercMin= glm::dvec3(DMAX);


struct CountHandler : public osmium::handler::Handler {

    std::uint64_t nodes = 0;
    std::uint64_t ways = 0;
    std::uint64_t closedWays = 0;
    std::uint64_t relations = 0;
    std::map<std::string, size_t> natMap;

    // This callback is called by osmium::apply for each node in the data.
    void node(const osmium::Node& node) noexcept {
        auto location = node.location();
        const osmium::geom::Coordinates c = osmium::geom::lonlat_to_mercator(location);
        if (c.x > mercMax.x)
            mercMax.x = c.x;
        else if (c.x < mercMin.x)
            mercMin.x = c.x;

        if (c.y > mercMax.y)
            mercMax.y = c.y;
        else if (c.y < mercMin.y)
            mercMin.y = c.y;
        
        nodePositions.push_back(glm::vec3(c.x, c.y, 0));
        NodeGLInfo info;
        info.pos = glm::dvec3(c.x, c.y, 0);
        info.ind = nodePositions.size() - 1;
        osmNodes[node.id()] = info;

        ++nodes;
    }

    // This callback is called by osmium::apply for each way in the data.
    void way(const osmium::Way& way) noexcept {
        
        DrawRange dRange;
        dRange.color = Color::getRandomColor();

        static auto xRange = mercMax.x - mercMin.x;
        static auto yRange = mercMax.y - mercMin.y;
        
        auto isBuilding = false;

        for (const auto& tag : way.tags())
        {
            const auto& key = tag.key();
            const auto& val = tag.value();
            auto keyStr = std::string(key);
            auto keyVal = std::string(val);
            if (keyStr.compare("natural") == 0)
            {
                //grassland
                //tree_row
                //water
                if (keyVal.compare("grassland") == 0||
                    keyVal.compare("grass") == 0)
                {
                    // 189,226,200
                    dRange.color = glm::vec3(189. / 255., 226. / 255., 200. / 255.);
                }
                else if (keyVal.compare("tree_row") == 0)
                {
                    //55,125,34
                    dRange.color = glm::vec3(55. / 255., 125. / 255., 34. / 255.);
                }
                else if (keyVal.compare("water") == 0)
                {
                    //156,192,249
                    dRange.color = glm::vec3(156. / 255., 192. / 255., 249. / 255.);
                }
            }
            else if (keyStr.compare("landuse") == 0)
            {
                natMap[keyVal] += 1;
                //comm
                //con
                //reli
                //retail
                //grass
                //recreat
                if (keyVal.compare("grass") == 0)
                {
                    dRange.color = glm::vec3(189. / 255., 226. / 255., 200. / 255.);
                }
                else if(keyVal.compare("commercial") == 0)
                    dRange.color = glm::vec3(125./255.);
                else if (keyVal.compare("construction") == 0)
                    dRange.color = glm::vec3(125. / 255.);
                else if (keyVal.compare("retail") == 0)
                    dRange.color = glm::vec3(200./255.);
                else if (keyVal.compare("religious") == 0)
                    dRange.color = glm::vec3(240. / 255., 134. / 255., 80. / 255.);
            }
            else if (keyStr.compare("highway") == 0)
            {
                //dRange.color = glm::vec3(250. / 255.);
            }
        }

        if (way.tags().has_tag("building","yes"))
        {
            isBuilding = true;
        }
        
        if (way.is_closed())
        {
            std::vector<p2t::Point*> points;
            dRange.offset = nodeIndices.size();
            /*if (!isBuilding)
            {
                for (auto node_ref : way.nodes())
                {
                    auto  coordinate = osmNodes[node_ref.ref()].pos;
                    auto pos = glm::vec3((-xRange / 2) + (coordinate.x - mercMin.x), (-yRange / 2) + (coordinate.y - mercMin.y), 0);
                    nodePositions.at(osmNodes[node_ref.ref()].ind) = pos;
                    nodeIndices.push_back(osmNodes[node_ref.ref()].ind);
                }
                dRange.drawCommand = GL_LINE_STRIP;
            }
            else*/
            {
                for (auto node_ref : way.nodes())
                {
                    auto  coordinate = osmNodes[node_ref.ref()].pos;
                    glm::dvec3 pos = glm::dvec3((-xRange / 2) + (coordinate.x - mercMin.x), (-yRange / 2) + (coordinate.y - mercMin.y), 0);
                    p2t::Point* pt = new p2t::Point();
                    pt->x = pos.x;
                    pt->y = pos.y;
                    points.push_back(pt);
                }
                points.pop_back();
                if (points.size()>0)
                {
                    dRange.drawCommand = GL_TRIANGLES;
                    p2t::CDT* cdt = new p2t::CDT(points);
                    vector<p2t::Triangle*> triangles;
                    cdt->Triangulate();
                    triangles = cdt->GetTriangles();

                    for (auto& triangle : triangles)
                    {
                        for (size_t p = 0; p <3; p++)
                        {
                            auto pt = triangle->GetPoint(p);
                            nodePositions.push_back(glm::vec3(pt->x, pt->y, 0));
                            nodeIndices.push_back(nodePositions.size() - 1);
                        }
                    }

                    delete cdt;
                }
            }
            
            dRange.drawCount = nodeIndices.size() - dRange.offset;
            

            waysData.push_back(dRange);
            closedWays++;
            
        }

        ++ways;
    }

    // This callback is called by osmium::apply for each relation in the data.
    void relation(const osmium::Relation& relation) noexcept {

        ++relations;
    }

}; // struct CountHandler

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
void readOSMFile();

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
    else if (key == GLFW_KEY_RIGHT /*&& action == GLFW_PRESS*/)
    {
        gOrigin.x += 10;
        globalModelMat = glm::translate(glm::mat4(1), gOrigin);
    }
    else if (key == GLFW_KEY_LEFT /*&& action == GLFW_PRESS*/)
    {
        gOrigin.x -= 10;
        globalModelMat = glm::translate(glm::mat4(1), gOrigin);
    }
    else if (key == GLFW_KEY_UP /*&& action == GLFW_PRESS*/)
    {
        gOrigin.y += 10;
        globalModelMat = glm::translate(glm::mat4(1), gOrigin);
    }
    else if (key == GLFW_KEY_DOWN /*&& action == GLFW_PRESS*/)
    {
        gOrigin.y -= 10;
        globalModelMat = glm::translate(glm::mat4(1), gOrigin);
    }
    else if (key == GLFW_KEY_Z /*&& action == GLFW_PRESS*/)
    {
        fov -= 0.05f;
        projectionMat = glm::perspective(fov, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 10000.0f);
        
    }
    else if (key == GLFW_KEY_X /*&& action == GLFW_PRESS*/)
    {
        fov += 0.05f;
        projectionMat = glm::perspective(fov, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 10000.0f);
        
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
    //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

    basicProgram = std::make_shared<GlslProgram>(Utility::readFileContents("shaders/vBasic.glsl"), Utility::readFileContents("shaders/fBasic.glsl"));
    
    setupScene();
    setupCamera();
   
    layer1 = getFboMSA(nullptr, 4);

    auto diffTex = GLUtility::makeTexture("img/container_diffuse.png");
    auto specTex = GLUtility::makeTexture("img/container_specular.png");
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

void setupCamera()
{
    auto eye = glm::vec3(0, 0, 500);
    auto center = glm::vec3(0, 0, 0);
    auto up = glm::vec3(0, 1, 0);
    camera = std::make_shared<Camera>(eye,center,up);
    camera->viewMat = glm::lookAt(camera->eye, camera->center, camera->up);
}

void setupScene()
{
    readOSMFile();
    streetMap = std::make_shared<Mesh>(nodePositions, nodeIndices);
    globalModelMat = glm::translate(glm::mat4(1), gOrigin);
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
}

void renderFrame()
{
   
    glBindFramebuffer(GL_FRAMEBUFFER, layer1->fbo);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    basicProgram->bind();
    basicProgram->setMat4f("view", camera->viewMat);
    basicProgram->setMat4f("proj", projectionMat);
    basicProgram->setVec3f("color", Color::red);

    glm::mat4 mv;
    
    for (auto& dRange : waysData)
    {
        mv = globalModelMat;
        basicProgram->setMat4f("model", mv);
        basicProgram->setVec3f("color", dRange.color);
        basicProgram->bindAllUniforms();
        streetMap->draw(dRange);
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
        ImGui::Begin("Open Street Map Sample");                     

        ImGui::Text("Use UP & DOWN arrows for zoom-In&Out");
        ImGui::SliderFloat("Rotation", &gRotation, 0, 360);
        ImGui::Text("Use Lef mouse click to pick triangle");

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();
    }
    
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void readOSMFile()
{
    try {
        // The Reader is initialized here with an osmium::io::File, but could
        // also be directly initialized with a file name.
        const osmium::io::File input_file{ "osm/sample.osm"};
        osmium::io::Reader reader{ input_file };

        // Create an instance of our own CountHandler and push the data from the
        // input file through it.
        CountHandler handler;
        osmium::apply(reader, handler);

        // You do not have to close the Reader explicitly, but because the
        // destructor can't throw, you will not see any errors otherwise.
        osmium::io::Header header = reader.header();
        std::cout << "HEADER:\n  generator=" << header.get("generator") << "\n";

        for (const auto& bbox : header.boxes()) {
            std::cout << "  bbox=" << bbox << "\n";
        }

        reader.close();

        std::cout << "Nodes: " << handler.nodes << "\n";
        std::cout << "Ways: " << handler.ways << "\n";
        std::cout << "Closed Ways: " << handler.closedWays << "\n";
        std::cout << "Relations: " << handler.relations << "\n";

        // Because of the huge amount of OSM data, some Osmium-based programs
        // (though not this one) can use huge amounts of data. So checking actual
        // memore usage is often useful and can be done easily with this class.
        // (Currently only works on Linux, not macOS and Windows.)
        const osmium::MemoryUsage memory;

        std::cout << "\nMemory used: " << memory.peak() << " MBytes\n";
    }
    catch (const std::exception& e) {
        // All exceptions used by the Osmium library derive from std::exception.
        std::cerr << e.what() << '\n';
        
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

