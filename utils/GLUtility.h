
#ifndef GLUTILITY_H
#define GLUTILITY_H


#include "CommonHeaders.h"


namespace GLUtility
{
	const glm::vec3 Y_AXIS = glm::vec3(0, 1, 0);
	const glm::vec3 X_AXIS = glm::vec3(1, 0, 0);
	const glm::vec3 Z_AXIS = glm::vec3(0, 0, 1);

	struct DrawRange
	{
		unsigned int offset;
		unsigned int drawCount;
		GLuint texID = 0;
		GLuint sTexID = 0;
	};

	struct VertexData
	{
		glm::vec3 pos;
		glm::vec3 norm;
		glm::vec2 uv;
	};

	struct VDPosUV
	{
		glm::vec3 pos;
		glm::vec2 uv;
	};

	struct Texture2D
	{
		GLuint texture;
		uint16_t width;
		uint16_t height;

		~Texture2D()
		{
			if (texture > 0)
			{
				glDeleteTextures(1, &texture);
				cout << "Deleting texture  \n";
				texture = 0;
			}
		}
	};

	struct FrameBuffer
	{
		GLuint color;//Warning:This is texture..!
		GLuint depth;
		GLuint fbo;
		uint16_t width;
		uint16_t height;

		~FrameBuffer()
		{
			cout << "Deleting : ";
			if (fbo > 0)
			{
				glDeleteFramebuffers(1, &fbo);
				cout << "fbo \t";
				fbo = 0;
			}
			if (depth > 0)
			{
				glDeleteRenderbuffers(1, &depth);
				cout << "depth attachment\t";
				depth = 0;
			}
			if (color > 0)
			{
				glDeleteTextures(1, &color);
				cout << "color attachment  \n";
				color = 0;
			}
				
		}
	};

	struct Texture
	{
		unsigned int id;
		string type;
	};

	struct Mesh
	{
		vector<glm::vec3> vDataVec3;
		vector<VertexData> vData;
		vector<unsigned int> iData;
		vector<Texture> textures;

		glm::mat4 tMatrix;
		unsigned int drawCount;
		string name;
		glm::vec3 position;
		glm::vec4 color;
		GLenum drawCommand=GL_TRIANGLES;

		GLuint vbo, ibo, vao;

		Mesh(vector<VertexData> vData, vector<unsigned int> iData);
		Mesh(vector<glm::vec3> vData, vector<unsigned int> iData);
		Mesh(vector<VertexData> vData, vector<unsigned int> iData,vector<Texture> textures);
		void updateIbo();
		void draw();
		void draw(vector<DrawRange> ranges);
		void drawInstanced(int iCount);
		~Mesh();
	};

	/*
	* Mesh Structure for openGL ES
	*/
	struct MeshGLES
	{
		vector<VDPosUV> vData;
		vector<unsigned short> iData;
		vector<Texture> textures;

		glm::mat4 tMatrix;
		unsigned int drawCount;
		string name;
		glm::vec3 position;
		GLenum drawCommand = GL_TRIANGLES;

		GLuint vbo, ibo, vao;

		MeshGLES(vector<VDPosUV> vData, vector<unsigned short> iData);
		//MeshGLES(vector<VertexData> vData, vector<unsigned short> iData, vector<Texture> textures);
		void updateIbo();
		void draw();
		~MeshGLES();
	};

	


	GLuint makeVetexBufferObject(vector<VertexData> data);
	GLuint makeVetexBufferObject(vector<glm::vec3> data);
	GLuint makeIndexBufferObject(vector<unsigned int> data);
	GLuint makeVertexArrayObject(GLuint vbo, GLuint ibo);
	GLuint makeVertexArrayObjectVec3(GLuint vbo, GLuint ibo);
	GLuint makeTexture(string fileName);
	GLuint makeTexture(string fileName,glm::vec2 &dim);
	GLuint makeCubeMap(vector<string> faces);
	std::shared_ptr<FrameBuffer> makeFbo(int width,int height,int samples);
	
	

	void checkFrambufferStatus(GLuint fbo);
	std::shared_ptr<Mesh> getSimpleTri();
	std::shared_ptr<Mesh> getCube(float width, float height, float depth);
	std::shared_ptr<Mesh> getCubeVec3(float width, float height, float depth);
	std::shared_ptr<Mesh> getBoudingBox(glm::vec3 bbMin,glm::vec3 bbMax);
	/*
	* No normal info
	*/
	std::shared_ptr<Mesh> getRect(float width, float height, bool stroke=true);
	std::shared_ptr<Mesh> get2DRect(float width, float height);
	std::shared_ptr<Mesh> getPolygon(vector<glm::vec3> points);

	/*
	* Returns a openGLES 2.0 compatible fullscreen quad mesh
	*/
	std::shared_ptr<MeshGLES> getfsQuadES();

	/*
	* Returns a fullscreen quad mesh
	*/
	std::shared_ptr<Mesh> getfsQuad();


	/*
	* @param width of Rect
	* @param height of Rect
	* @param gridX indicates no of grids in width
	* @param girdY indicates no of grids in height
	*/
	std::shared_ptr<Mesh> getRect(float width, float height, int gridX,int gridY);

	/*
	* @param xRad x radius of HemiSphere
	* @param yRad y radius of HemiSphere
	* @param gridX indicates no of grids in xRad
	* @param girdY indicates no of grids in yRad
	*/
	std::shared_ptr<Mesh> getHemiSphere(float xRad, float yRad, int gridX, int gridY);

	/*
	* Creates Mesh to view nrmls of given mesh
	* @param ipMesh mesh from which target mesh will be created
	*/
	std::shared_ptr<Mesh> getNrmlMesh(std::shared_ptr<Mesh> ipMesh, float nrmlLen=2.0f);


	/*
	* Function to find Ray-Tri intersection
	* @param R vector of Ray points
	* @param T vector of Tri points
	* @pram intersectionPt vec3 of point of intersection
	* @return value 0 no intersect, 1 intersects
	* 
	* 
	*/
	int intersects3D_RayTrinagle(vector<glm::vec3> R, vector<glm::vec3> T,glm::vec3& intersectionPt);

	glm::vec3 getNormal(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);
}

#endif // !GLUTILITY_H