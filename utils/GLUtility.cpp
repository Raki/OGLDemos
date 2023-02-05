#include "GLUtility.h"

#define SMALL_NUM   0.00000001 // anything that avoids division overflow
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace GLUtility
{

	GLuint makeVetexBufferObject(vector<VertexData> data)
	{
		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(VertexData), data.data(), GL_STATIC_DRAW);
		return buffer;
	}

	GLuint makeVetexBufferObject(vector<VDPosNormColr> data)
	{
		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(VDPosNormColr), data.data(), GL_STATIC_DRAW);
		return buffer;
	}

	GLuint makeVetexBufferObject(vector<glm::vec3> data)
	{
		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec3), data.data(), GL_STATIC_DRAW);
		return buffer;
	}

	GLuint makeIndexBufferObject(vector<unsigned int> data)
	{
		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), data.data(), GL_STATIC_DRAW);
		return buffer;
	}

	GLuint makeVertexArrayObjectVec3(GLuint vbo, GLuint ibo)
	{
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);


		return vao;
	}

	GLuint makeVertexArrayObject(GLuint vbo, GLuint ibo)
	{
		auto nrmoffset = sizeof(glm::vec3);
		auto uvoffset = sizeof(glm::vec3) * 2;
		auto vdSize = sizeof(VertexData);

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)sizeof(glm::vec3));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(glm::vec3) * 2));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);


		return vao;
	}

	std::shared_ptr<Mesh> getSimpleTri()
	{
		vector<VertexData> vData = {
			{glm::vec3(0,0,0),glm::vec3(0,0,1),glm::vec2(0,0)},
			{glm::vec3(0.5,0,0),glm::vec3(0,0,1),glm::vec2(1,0)},
			{glm::vec3(0.5,0.5,0),glm::vec3(0,0,1),glm::vec2(1,1)}
		};
		vector<unsigned int> iData = { 0,1,2 };

		auto triMesh = std::make_shared<Mesh>(vData,iData);
		triMesh->name = "Simple Triangle";

		return triMesh;
		
	}

	std::shared_ptr<Mesh> getTri(std::array<glm::vec3, 3> verts)
	{
		auto norm = getNormal(verts[0], verts[1], verts[2]);
		norm = glm::normalize(norm);
		vector<VertexData> vData = {
			{verts[0],norm,glm::vec2(0,0)},
			{verts[1],norm,glm::vec2(1,0)},
			{verts[2],norm,glm::vec2(1,1)}
		};
		vector<unsigned int> iData = { 0,1,2 };
		auto triMesh = std::make_shared<Mesh>(vData, iData);
		triMesh->name = "Simple Triangle";
		return triMesh;
	}

	//ToDo : check if file is missing
	GLuint makeTexture(string fileName)
	{
		int width, height, nrChannels;
		
		stbi_set_flip_vertically_on_load(1);
		unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &nrChannels, 0);
		
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D,texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);

		return texture;
	}

	GLuint makeTexture(string fileName, glm::vec2& dim)
	{
		int width, height, nrChannels;

		stbi_set_flip_vertically_on_load(1);
		unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &nrChannels, 0);
		dim.x = (float)width;
		dim.y = (float)height;

		GLenum format= GL_RGB;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);

		return texture;
	}

	GLuint makeCubeMap(vector<string> faces)
	{
		if(faces.size()<6)
			return GLuint();
		
		GLuint texID;
		glGenTextures(1, &texID);

		glBindTexture(GL_TEXTURE_CUBE_MAP,texID);

		int width, height, nrChannels;
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			//stbi_set_flip_vertically_on_load(1);
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
				);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
				stbi_image_free(data);
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		return texID;

	}

	unsigned char* getImageData(std::string filename, int& width, int& height, int& nChannels)
	{
		stbi_set_flip_vertically_on_load(1);
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nChannels, 0);
		return data;
	}

	void freeImageData(unsigned char* data)
	{
		if (data != NULL)
			stbi_image_free(data);
	}

	std::shared_ptr<FrameBuffer> makeFbo(int width, int height,int samples)
	{
		std::shared_ptr<FrameBuffer> fbObj = std::make_shared<FrameBuffer>();
		fbObj->width = width;
		fbObj->height = height;
		GLuint fbo;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo); //bind both read/write to the target framebuffer

		GLuint color,depth;
		glGenTextures(1, &color);
		glBindTexture(GL_TEXTURE_2D, color);
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			width, height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

		glGenRenderbuffers(1, &depth);
		glBindRenderbuffer(GL_RENDERBUFFER, depth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		checkFrambufferStatus(fbo);

		fbObj->fbo = fbo;
		fbObj->color = color;
		fbObj->depth = depth;


		return fbObj;
	}

	void checkFrambufferStatus(GLuint fbo)
	{
		GLenum status;
		status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		switch (status) {
		case GL_FRAMEBUFFER_COMPLETE:
			break;

		case GL_FRAMEBUFFER_UNSUPPORTED:
			/* choose different formats */
			break;

		default:
			/* programming error; will fail on all hardware */
			fputs("Framebuffer Error\n", stderr);

		}
	}

	std::shared_ptr<Mesh> getCube(float width, float height, float depth)
	{
		bool hasTex = true;
		glm::vec3 bbMin, bbMax;
		bbMax.x = width / 2;
		bbMax.y = height / 2;
		bbMax.z = depth / 2;
		bbMin.x = -width / 2;
		bbMin.y = -height / 2;
		bbMin.z = -depth / 2;


		vector<glm::vec3> vArr, nArr;
		vector<glm::vec2> uvArr;

		//top
		glm::vec3 t1 = glm::vec3(bbMin.x, bbMax.y, bbMin.z);
		glm::vec3 t2 = glm::vec3(bbMax.x, bbMax.y, bbMin.z);
		glm::vec3 t3 = glm::vec3(bbMax.x, bbMax.y, bbMax.z);
		glm::vec3 t4 = glm::vec3(bbMin.x, bbMax.y, bbMax.z);

		//bottom
		glm::vec3 b1 = glm::vec3(bbMin.x, bbMin.y, bbMin.z);
		glm::vec3 b2 = glm::vec3(bbMax.x, bbMin.y, bbMin.z);
		glm::vec3 b3 = glm::vec3(bbMax.x, bbMin.y, bbMax.z);
		glm::vec3 b4 = glm::vec3(bbMin.x, bbMin.y, bbMax.z);

		// front			back
		//		t4--t3			t2--t1
		//		|    |			|	|
		//		b4--b3			b2--b1
		// left			right
		//		t1--t4			t3--t2
		//		|    |			|	|
		//		b1--b4			b3--b2
		// top			bottom
		//		t1--t2			b4--b3
		//		|    |			|	|
		//		t4--t3			b1--b2
		//front
		vArr.push_back(b4);		vArr.push_back(b3);		vArr.push_back(t3);
		vArr.push_back(b4);		vArr.push_back(t3);		vArr.push_back(t4);
		{
			auto n = getNormal(b4, b3, t3);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			if (hasTex)
			{
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 1));
			}
			else
			{
				for (auto i = 0; i < 6; i++)uvArr.push_back(glm::vec2(-1, -1));
			}
		}


		//back
		vArr.push_back(b2);		vArr.push_back(b1);		vArr.push_back(t1);
		vArr.push_back(b2);		vArr.push_back(t1);		vArr.push_back(t2);
		{
			auto n = getNormal(b2, b1, t1);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			if (hasTex)
			{
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 1));
			}
			else
			{
				for (auto i = 0; i < 6; i++)uvArr.push_back(glm::vec2(-1, -1));
			}
		}

		//left
		vArr.push_back(b1);		vArr.push_back(b4);		vArr.push_back(t4);
		vArr.push_back(b1);		vArr.push_back(t4);		vArr.push_back(t1);
		{
			auto n = getNormal(b1, b4, t4);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			if (hasTex)
			{
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 1));
			}
			else
			{
				for (auto i = 0; i < 6; i++)uvArr.push_back(glm::vec2(-1, -1));
			}
		}

		//right
		vArr.push_back(b3);		vArr.push_back(b2);		vArr.push_back(t2);
		vArr.push_back(b3);		vArr.push_back(t2);		vArr.push_back(t3);
		{
			auto n = getNormal(b3, b2, t2);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			if (hasTex)
			{
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 1));
			}
			else
			{
				for (auto i = 0; i < 6; i++)uvArr.push_back(glm::vec2(-1, -1));
			}
		}

		//top
		vArr.push_back(t4);		vArr.push_back(t3);		vArr.push_back(t2);
		vArr.push_back(t4);		vArr.push_back(t2);		vArr.push_back(t1);
		{
			auto n = getNormal(t4, t3, t2);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			if (hasTex)
			{
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 1));
			}
			else
			{
				for (auto i = 0; i < 6; i++)uvArr.push_back(glm::vec2(-1, -1));
			}
		}

		//bottom
		vArr.push_back(b1);		vArr.push_back(b2);		vArr.push_back(b3);
		vArr.push_back(b1);		vArr.push_back(b3);		vArr.push_back(b4);
		{
			auto n = getNormal(b1, b2, b3);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			if (hasTex)
			{
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 1));
			}
			else
			{
				for (auto i = 0; i < 6; i++)uvArr.push_back(glm::vec2(-1, -1));
			}
		}

		vector<VertexData> interleavedArr;
		vector<unsigned int> iArr;

		auto totalVerts = vArr.size();
		for (auto i = 0; i < totalVerts; i++)
		{
			auto v = vArr.at(i);
			auto n = nArr.at(i);
			auto uv = uvArr.at(i);
			interleavedArr.push_back({ v,n,uv });
			iArr.push_back((unsigned int)iArr.size());
		}

		auto cubeMesh = std::make_shared<Mesh>(interleavedArr, iArr);
		cubeMesh->name = "Cube Mesh";

		return cubeMesh;
	}

	void fillCube(float width, float height, float depth,glm::vec3 color,glm::mat4 tMat, vector<VDPosNormColr>& vData, vector<unsigned int>& iData)
	{
		bool hasTex = true;
		glm::vec3 bbMin, bbMax;
		bbMax.x = width / 2;
		bbMax.y = height / 2;
		bbMax.z = depth / 2;
		bbMin.x = -width / 2;
		bbMin.y = -height / 2;
		bbMin.z = -depth / 2;


		vector<glm::vec3> vArr, nArr;
		vector<glm::vec2> uvArr;

		//top
		glm::vec3 t1 = glm::vec3(bbMin.x, bbMax.y, bbMin.z);
		glm::vec3 t2 = glm::vec3(bbMax.x, bbMax.y, bbMin.z);
		glm::vec3 t3 = glm::vec3(bbMax.x, bbMax.y, bbMax.z);
		glm::vec3 t4 = glm::vec3(bbMin.x, bbMax.y, bbMax.z);

		//bottom
		glm::vec3 b1 = glm::vec3(bbMin.x, bbMin.y, bbMin.z);
		glm::vec3 b2 = glm::vec3(bbMax.x, bbMin.y, bbMin.z);
		glm::vec3 b3 = glm::vec3(bbMax.x, bbMin.y, bbMax.z);
		glm::vec3 b4 = glm::vec3(bbMin.x, bbMin.y, bbMax.z);

		// front			back
		//		t4--t3			t2--t1
		//		|    |			|	|
		//		b4--b3			b2--b1
		// left			right
		//		t1--t4			t3--t2
		//		|    |			|	|
		//		b1--b4			b3--b2
		// top			bottom
		//		t1--t2			b4--b3
		//		|    |			|	|
		//		t4--t3			b1--b2
		//front
		vArr.push_back(b4);		vArr.push_back(b3);		vArr.push_back(t3);
		vArr.push_back(b4);		vArr.push_back(t3);		vArr.push_back(t4);
		{
			auto n = getNormal(b4, b3, t3);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			if (hasTex)
			{
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 1));
			}
			else
			{
				for (auto i = 0; i < 6; i++)uvArr.push_back(glm::vec2(-1, -1));
			}
		}


		//back
		vArr.push_back(b2);		vArr.push_back(b1);		vArr.push_back(t1);
		vArr.push_back(b2);		vArr.push_back(t1);		vArr.push_back(t2);
		{
			auto n = getNormal(b2, b1, t1);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			if (hasTex)
			{
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 1));
			}
			else
			{
				for (auto i = 0; i < 6; i++)uvArr.push_back(glm::vec2(-1, -1));
			}
		}

		//left
		vArr.push_back(b1);		vArr.push_back(b4);		vArr.push_back(t4);
		vArr.push_back(b1);		vArr.push_back(t4);		vArr.push_back(t1);
		{
			auto n = getNormal(b1, b4, t4);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			if (hasTex)
			{
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 1));
			}
			else
			{
				for (auto i = 0; i < 6; i++)uvArr.push_back(glm::vec2(-1, -1));
			}
		}

		//right
		vArr.push_back(b3);		vArr.push_back(b2);		vArr.push_back(t2);
		vArr.push_back(b3);		vArr.push_back(t2);		vArr.push_back(t3);
		{
			auto n = getNormal(b3, b2, t2);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			if (hasTex)
			{
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 1));
			}
			else
			{
				for (auto i = 0; i < 6; i++)uvArr.push_back(glm::vec2(-1, -1));
			}
		}

		//top
		vArr.push_back(t4);		vArr.push_back(t3);		vArr.push_back(t2);
		vArr.push_back(t4);		vArr.push_back(t2);		vArr.push_back(t1);
		{
			auto n = getNormal(t4, t3, t2);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			if (hasTex)
			{
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 1));
			}
			else
			{
				for (auto i = 0; i < 6; i++)uvArr.push_back(glm::vec2(-1, -1));
			}
		}

		//bottom
		vArr.push_back(b1);		vArr.push_back(b2);		vArr.push_back(b3);
		vArr.push_back(b1);		vArr.push_back(b3);		vArr.push_back(b4);
		{
			auto n = getNormal(b1, b2, b3);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			nArr.push_back(n);
			if (hasTex)
			{
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 0));
				uvArr.push_back(glm::vec2(1, 1));
				uvArr.push_back(glm::vec2(0, 1));
			}
			else
			{
				for (auto i = 0; i < 6; i++)uvArr.push_back(glm::vec2(-1, -1));
			}
		}

		auto totalVerts = vArr.size();
		for (auto i = 0; i < totalVerts; i++)
		{
			auto v = glm::vec3(tMat*glm::vec4(vArr.at(i),1.0f));
			auto n = glm::normalize(nArr.at(i));
			auto uv = uvArr.at(i);
			vData.push_back({ v,n,color });
			iData.push_back((unsigned int)iData.size());
		}
	}

	void fillCubeforCSG(float width, float height, float depth, glm::mat4 tMat, vector<glm::vec3>& vData, vector<unsigned int>& iData)
	{
		glm::vec3 bbMin, bbMax;
		bbMax.x = width / 2;
		bbMax.y = height / 2;
		bbMax.z = depth / 2;
		bbMin.x = -width / 2;
		bbMin.y = -height / 2;
		bbMin.z = -depth / 2;


		//top
		glm::vec3 t1 = glm::vec3(tMat*glm::vec4(bbMin.x, bbMax.y, bbMin.z,1.f)); //0
		glm::vec3 t2 = glm::vec3(tMat*glm::vec4(bbMax.x, bbMax.y, bbMin.z,1.f)); //1
		glm::vec3 t3 = glm::vec3(tMat*glm::vec4(bbMax.x, bbMax.y, bbMax.z,1.f)); //2
		glm::vec3 t4 = glm::vec3(tMat*glm::vec4(bbMin.x, bbMax.y, bbMax.z,1.f)); //3
											  
		//bottom							  
		glm::vec3 b1 = glm::vec3(tMat*glm::vec4(bbMin.x, bbMin.y, bbMin.z,1.f)); //4
		glm::vec3 b2 = glm::vec3(tMat*glm::vec4(bbMax.x, bbMin.y, bbMin.z,1.f)); //5
		glm::vec3 b3 = glm::vec3(tMat*glm::vec4(bbMax.x, bbMin.y, bbMax.z,1.f)); //6
		glm::vec3 b4 = glm::vec3(tMat*glm::vec4(bbMin.x, bbMin.y, bbMax.z,1.f)); //7

		// front			back
		//		t4--t3			t2--t1
		//		|    |			|	|
		//		b4--b3			b2--b1
		// left			right
		//		t1--t4			t3--t2
		//		|    |			|	|
		//		b1--b4			b3--b2
		// top			bottom
		//		t1--t2			b4--b3
		//		|    |			|	|
		//		t4--t3			b1--b2
		vData.push_back(t1); vData.push_back(t2); vData.push_back(t3); vData.push_back(t4);
		vData.push_back(b1); vData.push_back(b2); vData.push_back(b3); vData.push_back(b4);

		//b4,b3,t3
		iData.push_back(7); iData.push_back(6); iData.push_back(2);
		//b4,t3,t4
		iData.push_back(7); iData.push_back(2); iData.push_back(3);

		//b2,b1,t1
		iData.push_back(5); iData.push_back(4); iData.push_back(0);
		//b2,t1,t2
		iData.push_back(5); iData.push_back(0); iData.push_back(1);

		//b1,b4,t4
		iData.push_back(4); iData.push_back(7); iData.push_back(3);
		//b1,t4,t1
		iData.push_back(4); iData.push_back(3); iData.push_back(0);

		//b3,b2,t2
		iData.push_back(6); iData.push_back(5); iData.push_back(1);
		//b3,t2,t3
		iData.push_back(6); iData.push_back(1); iData.push_back(2);

		//t4,t3,t2
		iData.push_back(3); iData.push_back(2); iData.push_back(1);
		//t4,t2,t1
		iData.push_back(3); iData.push_back(1); iData.push_back(0);

		//b1,b2,b3
		iData.push_back(4); iData.push_back(5); iData.push_back(6);
		//b1,b3,b4
		iData.push_back(4); iData.push_back(6); iData.push_back(7);

	}

	void fillStarforCSG(float radius,float thickness, vector<glm::vec3>& vData, vector<unsigned int>& iData)
	{
		std::vector<glm::vec3> vArr1,vArr2;
		
		vArr2.push_back(glm::vec3(0,-thickness/2,0));
		for (float th=0;th<360;th+=120)
		{
			float x = radius * cos(glm::radians(th));
			float y = thickness / 2;
			float z = radius * sin(glm::radians(th));
			vArr1.insert(vArr1.begin(), glm::vec3(x, y, z));
			vArr2.push_back(glm::vec3(x, -y, z));
		}
		vArr1.insert(vArr1.begin(),glm::vec3(0, thickness / 2, 0));
		for (auto v : vArr1)
			vData.push_back(v);
		for (auto v : vArr2)
			vData.push_back(v);
		for (size_t i = 1; i < vArr1.size(); i++)
		{
			auto ind0 = 0;
			auto ind1 = i;
			auto ind2 = (i == vArr1.size() - 1) ? 1 : i + 1;
			iData.push_back(ind0); iData.push_back(ind1); iData.push_back(ind2);
		}

		for (size_t i = vArr1.size()+1; i < vArr1.size()+vArr2.size(); i++)
		{
			auto ind0 = vArr1.size();
			auto ind1 = i;
			auto ind2 = (i == vArr1.size() + vArr2.size() - 1) ? vArr1.size() + 1 : i + 1;
			iData.push_back(ind0); iData.push_back(ind1); iData.push_back(ind2);
		}

		for (size_t i = 1,j=vArr1.size()-1; i < vArr1.size(); i++,j--)
		{
			auto ind0 = j;
			auto ind1 = (i == vArr1.size() - 1)? vArr1.size() - 1 :j - 1;
			auto ind2 = (i) + (vArr1.size());
			auto ind3 = (i == vArr1.size() - 1)?vArr1.size()+1 : ((i) + vArr1.size()) + 1;
			iData.push_back(ind2); iData.push_back(ind3); iData.push_back(ind1);
			iData.push_back(ind2); iData.push_back(ind1); iData.push_back(ind0);
		}
		
	}

	void fillPyramidforCSG(float base, float height, vector<glm::vec3>& vData, vector<unsigned int>& iData)
	{
	}

	std::shared_ptr<Mesh> getCubeVec3(float width, float height, float depth)
	{
		bool hasTex = true;
		glm::vec3 bbMin, bbMax;
		bbMax.x = width / 2;
		bbMax.y = height / 2;
		bbMax.z = depth / 2;
		bbMin.x = -width / 2;
		bbMin.y = -height / 2;
		bbMin.z = -depth / 2;


		vector<glm::vec3> vArr, nArr;
		vector<glm::vec2> uvArr;

		//top
		glm::vec3 t1 = glm::vec3(bbMin.x, bbMax.y, bbMin.z);
		glm::vec3 t2 = glm::vec3(bbMax.x, bbMax.y, bbMin.z);
		glm::vec3 t3 = glm::vec3(bbMax.x, bbMax.y, bbMax.z);
		glm::vec3 t4 = glm::vec3(bbMin.x, bbMax.y, bbMax.z);

		//bottom
		glm::vec3 b1 = glm::vec3(bbMin.x, bbMin.y, bbMin.z);
		glm::vec3 b2 = glm::vec3(bbMax.x, bbMin.y, bbMin.z);
		glm::vec3 b3 = glm::vec3(bbMax.x, bbMin.y, bbMax.z);
		glm::vec3 b4 = glm::vec3(bbMin.x, bbMin.y, bbMax.z);

		// front			back
		//		t4--t3			t2--t1
		//		|    |			|	|
		//		b4--b3			b2--b1
		// left			right
		//		t1--t4			t3--t2
		//		|    |			|	|
		//		b1--b4			b3--b2
		// top			bottom
		//		t1--t2			b4--b3
		//		|    |			|	|
		//		t4--t3			b1--b2
		//front
		vArr.push_back(b4);		vArr.push_back(b3);		vArr.push_back(t3);
		vArr.push_back(b4);		vArr.push_back(t3);		vArr.push_back(t4);
	


		//back
		vArr.push_back(b2);		vArr.push_back(b1);		vArr.push_back(t1);
		vArr.push_back(b2);		vArr.push_back(t1);		vArr.push_back(t2);
		

		//left
		vArr.push_back(b1);		vArr.push_back(b4);		vArr.push_back(t4);
		vArr.push_back(b1);		vArr.push_back(t4);		vArr.push_back(t1);
		

		//right
		vArr.push_back(b3);		vArr.push_back(b2);		vArr.push_back(t2);
		vArr.push_back(b3);		vArr.push_back(t2);		vArr.push_back(t3);
		

		//top
		vArr.push_back(t4);		vArr.push_back(t3);		vArr.push_back(t2);
		vArr.push_back(t4);		vArr.push_back(t2);		vArr.push_back(t1);
		

		//bottom
		vArr.push_back(b1);		vArr.push_back(b2);		vArr.push_back(b3);
		vArr.push_back(b1);		vArr.push_back(b3);		vArr.push_back(b4);
		

		vector<glm::vec3> interleavedArr;
		vector<unsigned int> iArr;

		auto totalVerts = vArr.size();
		for (auto i = 0; i < totalVerts; i++)
		{
			auto v = vArr.at(i);
			interleavedArr.push_back(v);
			iArr.push_back((unsigned int)iArr.size());
		}

		auto cubeMesh = std::make_shared<Mesh>(interleavedArr, iArr);
		cubeMesh->name = "Cube Mesh";

		return cubeMesh;
	}

	std::shared_ptr<Mesh> getBoudingBox(glm::vec3 bbMin, glm::vec3 bbMax)
	{
		const auto dim = bbMax - bbMin;
		//bottom
		auto v0 = bbMin;
		auto v1 = bbMin + glm::vec3(dim.x, 0, 0);
		auto v2 = bbMin + glm::vec3(dim.x, 0, dim.z);
		auto v3 = bbMin + glm::vec3(0, 0, dim.z);
		//top
		auto v4 = v0 + glm::vec3(0,dim.y,0);
		auto v5 = v1 + glm::vec3(0,dim.y,0);
		auto v6 = v2 + glm::vec3(0,dim.y,0);
		auto v7 = v3 + glm::vec3(0,dim.y,0);

		std::vector<glm::vec3> vData{v0,v1,v2,v3,v4,v5,v6,v7};
		std::vector<unsigned int> iData{0,1,1,2,2,3,3,0,4,5,5,6,6,7,7,4,0,4,1,5,2,6,3,7};

		auto bbMesh = std::make_shared<Mesh>(vData, iData);
		bbMesh->name = "BoundingBox";
		bbMesh->drawCommand = GL_LINES;
		return bbMesh;
	}

	/*std::shared_ptr<Mesh> getMeshFromHeightMap(std::string heightMapPath)
	{
		int width, height, nrChannels;

		stbi_set_flip_vertically_on_load(1);
		unsigned char* data = stbi_load(heightMapPath.c_str(), &width, &height, &nrChannels, 0);
		float yScale = 64.0f / 256.0f, yShift = 16.0f;
		for (size_t h = 0; h < height; h++)
		{
			for (size_t w = 0; w < width; w++)
			{
				vector<char>pixelData(nrChannels);
				memcpy(pixelData.data(), &data[(width*h*nrChannels)+w], nrChannels * sizeof(char));
				glm::vec3 position = glm::vec3((- width / 2)+w,static_cast<int>(pixelData[0]*yScale-yShift), (-height / 2) + h);
			}
		}
		stbi_image_free(data);

	}*/

	std::shared_ptr<Mesh> getRect(float width, float height, bool stroke)
	{
		vector<VertexData> interleavedArr;
		vector<unsigned int> iArr;
		GLenum drawCommand= GL_TRIANGLES;
		
		interleavedArr.push_back({ glm::vec3(-width / 2,-height / 2,0),glm::vec3(0),glm::vec2(-1) });
		interleavedArr.push_back({ glm::vec3(width / 2,-height / 2,0),glm::vec3(0),glm::vec2(-1) });
		interleavedArr.push_back({ glm::vec3(width / 2,height / 2,0),glm::vec3(0),glm::vec2(-1) });
		interleavedArr.push_back({ glm::vec3(-width / 2,height / 2,0),glm::vec3(0),glm::vec2(-1) });
		auto normal = getNormal(interleavedArr.at(0).pos, interleavedArr.at(1).pos, interleavedArr.at(2).pos);
		interleavedArr.at(0).norm = normal;
		interleavedArr.at(1).norm = normal;
		interleavedArr.at(2).norm = normal;
		interleavedArr.at(3).norm = normal;
		if (!stroke)
		{
			iArr.push_back(0);
			iArr.push_back(1);
			iArr.push_back(2);

			iArr.push_back(0);
			iArr.push_back(2);
			iArr.push_back(3);
			drawCommand = GL_TRIANGLES;
		}
		else
		{
			iArr.push_back(0);
			iArr.push_back(1);
			iArr.push_back(2);
			iArr.push_back(3);
			drawCommand = GL_LINE_LOOP;
		}

		auto rectMesh = std::make_shared<Mesh>(interleavedArr, iArr);
		rectMesh->name = "Rect";
		rectMesh->drawCommand = drawCommand;
		return rectMesh;
	}

	std::shared_ptr<Mesh> get2DRect(float width, float height)
	{
		vector<VertexData> interleavedArr;
		vector<unsigned int> iArr;
		GLenum drawCommand = GL_TRIANGLES;

		interleavedArr.push_back({ glm::vec3(-width / 2,-height / 2,0),glm::vec3(0),glm::vec2(0,0) });
		interleavedArr.push_back({ glm::vec3(width / 2,-height / 2,0),glm::vec3(0),glm::vec2(1,0) });
		interleavedArr.push_back({ glm::vec3(width / 2,height / 2,0),glm::vec3(0),glm::vec2(1,1) });
		interleavedArr.push_back({ glm::vec3(-width / 2,height / 2,0),glm::vec3(0),glm::vec2(0,1) });
		auto normal = getNormal(interleavedArr.at(0).pos, interleavedArr.at(1).pos, interleavedArr.at(2).pos);
		interleavedArr.at(0).norm = normal;
		interleavedArr.at(1).norm = normal;
		interleavedArr.at(2).norm = normal;
		interleavedArr.at(3).norm = normal;
			
		iArr.push_back(0);
		iArr.push_back(1);
		iArr.push_back(2);

		iArr.push_back(0);
		iArr.push_back(2);
		iArr.push_back(3);
		drawCommand = GL_TRIANGLES;

		auto rectMesh = std::make_shared<Mesh>(interleavedArr, iArr);
		rectMesh->name = "Textured Rect";
		rectMesh->drawCommand = drawCommand;
		return rectMesh;
	}

	std::shared_ptr<Mesh> getPolygon(vector<glm::vec3> points)
	{
		vector<glm::vec3> interleavedArr;
		vector<unsigned int> iArr;

		auto totalVerts = points.size();
		for (auto i = 0; i < totalVerts; i++)
		{
			auto v = points.at(i);
			interleavedArr.push_back(v);
			iArr.push_back((unsigned int)iArr.size());
		}

		auto polyline = std::make_shared<Mesh>(interleavedArr, iArr);
		polyline->name = "polyline";
		polyline->drawCommand = GL_LINE_STRIP;

		return polyline;
	}

	std::shared_ptr<MeshGLES> getfsQuadES()
	{
		vector<VDPosUV> vData = {
			{glm::vec3(-1,-1,0),glm::vec2(0,0)},
			{glm::vec3(1,-1,0),glm::vec2(1,0)},
			{glm::vec3(1,1,0),glm::vec2(1,1)},
			{glm::vec3(-1,1,0),glm::vec2(0,1)}
		};
		vector<unsigned short> iData = { 0,1,2,0,2,3 };

		auto triMesh = std::make_shared<MeshGLES>(vData, iData);
		triMesh->name = "fs quad";

		return triMesh;
	}

	std::shared_ptr<Mesh> getfsQuad()
	{
		vector<VertexData> vData = {
			{glm::vec3(-1,-1,0),glm::vec3(0,0,1),glm::vec2(0,0)},
			{glm::vec3(1,-1,0),glm::vec3(0,0,1),glm::vec2(1,0)},
			{glm::vec3(1,1,0),glm::vec3(0,0,1),glm::vec2(1,1)},
			{glm::vec3(-1,1,0),glm::vec3(0,0,1),glm::vec2(0,1)}
		};
		vector<unsigned int> iData = { 0,1,2,0,2,3 };

		auto triMesh = std::make_shared<Mesh>(vData, iData);
		triMesh->name = "Full screen quad";

		return triMesh;
	}

	std::shared_ptr<Mesh> getRect(float width, float height, int gridX, int gridY)
	{

		if(height*width*gridX*gridY<=0)
			return std::shared_ptr<Mesh>();


		auto origin = glm::vec3(-width/2,-height/2,0);
		float stepW = width / gridX;
		float stepH = height / gridY;
		vector<VertexData> vData;
		vector<unsigned int> iData;
		// position and texcoords
		for (int y = 0;y <= gridY; y++)
		{
			for (int x = 0; x <= gridX; x++)
			{
				auto pos = glm::vec3(x * stepW, y * stepH, 0);
				pos += origin;
				auto uv = glm::vec2((x*stepW)/width,(y*stepH)/height);
				vData.push_back({ pos,glm::vec3(0),uv });
			}
		}
		
		// normals and indices
		for (int y = 0;y <=gridY-1; y++)
		{
			for (int x = 0; x <=gridX-1; x++)
			{
				// v3-v4
				// | /|
				// |/ |
				// v1-v2

				auto v1Ind = (y * (gridY+1)) + x;
				auto v2Ind = (y * (gridY+1)) + x+1;
				auto v3Ind = ((y+1) * (gridY+1))+ x;
				auto v4Ind = ((y+1) * (gridY+1))+ x+1;

				auto n1 = getNormal(vData.at(v1Ind).pos, vData.at(v4Ind).pos, vData.at(v3Ind).pos);
				vData.at(v1Ind).norm += n1;
				vData.at(v1Ind).norm /= 2.0f;
				vData.at(v4Ind).norm += n1;
				vData.at(v4Ind).norm /= 2.0f;
				vData.at(v3Ind).norm += n1;
				vData.at(v3Ind).norm /= 2.0f;

				auto n2 = getNormal(vData.at(v1Ind).pos, vData.at(v2Ind).pos, vData.at(v4Ind).pos);
				vData.at(v1Ind).norm += n2;
				vData.at(v1Ind).norm /= 2.0f;
				vData.at(v2Ind).norm += n2;
				vData.at(v2Ind).norm /= 2.0f;
				vData.at(v4Ind).norm += n2;
				vData.at(v4Ind).norm /= 2.0f;

				iData.push_back(v1Ind); iData.push_back(v4Ind); iData.push_back(v3Ind);
				iData.push_back(v1Ind); iData.push_back(v2Ind); iData.push_back(v4Ind);

			}
		}

		auto rectMesh = std::make_shared<Mesh>(vData, iData);
		rectMesh->name = "Rect Mesh";
		return rectMesh;
	}

	std::shared_ptr<Mesh> getHemiSphere(float xRad, float yRad, int gridX, int gridY)
	{
		if (xRad * yRad * gridX * gridY <= 0)
			return std::shared_ptr<Mesh>();

		const float XTHETA = 180.0f;
		const float YTHETA = 180.0f;

		float stepW = XTHETA / gridX;
		float stepH = YTHETA / gridY;
		vector<VertexData> vData;
		vector<unsigned int> iData;
		// position and texcoords
		for (int y = 0; y <= gridY; y++)
		{
			for (int x = 0; x <= gridX; x++)
			{
				float xCoord = xRad * cos(glm::radians(x * stepW));
				float yCoord = yRad * sin(glm::radians(x * stepW));
				float zCoord = 0;

				auto pos = glm::vec3(xCoord,yCoord,zCoord);

				pos = glm::vec3(glm::rotate(glm::mat4(1),glm::radians(y*stepH),glm::vec3(1,0,0))*glm::vec4(pos,1));

				auto uv = glm::vec2((x * stepW) / XTHETA, (y * stepH) / YTHETA);
				vData.push_back({ pos,glm::vec3(0),uv });
			}
		}

		// normals and indices
		for (int y = 0; y <= gridY - 1; y++)
		{
			for (int x = 0; x <= gridX - 1; x++)
			{
				// v3-v4
				// | /|
				// |/ |
				// v1-v2

				auto v1Ind = (y * (gridY + 1)) + x;
				auto v2Ind = (y * (gridY + 1)) + x + 1;
				auto v3Ind = ((y + 1) * (gridY + 1)) + x;
				auto v4Ind = ((y + 1) * (gridY + 1)) + x + 1;

				auto n1 = getNormal(vData.at(v1Ind).pos, vData.at(v4Ind).pos, vData.at(v3Ind).pos);
				vData.at(v1Ind).norm += n1;
				vData.at(v1Ind).norm /= 2.0f;
				vData.at(v4Ind).norm += n1;
				vData.at(v4Ind).norm /= 2.0f;
				vData.at(v3Ind).norm += n1;
				vData.at(v3Ind).norm /= 2.0f;

				auto n2 = getNormal(vData.at(v1Ind).pos, vData.at(v2Ind).pos, vData.at(v4Ind).pos);
				vData.at(v1Ind).norm += n2;
				vData.at(v1Ind).norm /= 2.0f;
				vData.at(v2Ind).norm += n2;
				vData.at(v2Ind).norm /= 2.0f;
				vData.at(v4Ind).norm += n2;
				vData.at(v4Ind).norm /= 2.0f;

				iData.push_back(v1Ind); iData.push_back(v4Ind); iData.push_back(v3Ind);
				iData.push_back(v1Ind); iData.push_back(v2Ind); iData.push_back(v4Ind);

			}
		}

		auto hSphereMesh = std::make_shared<Mesh>(vData, iData);
		hSphereMesh->name = "Rect Mesh";
		return hSphereMesh;
	}

	std::shared_ptr<Mesh> getNrmlMesh(std::shared_ptr<Mesh> ipMesh,float nrmlLen)
	{
		if(ipMesh->iData.size()==0)
			return std::shared_ptr<Mesh>();

		vector<VertexData> vData;
		vector<unsigned int> iData;

		for (size_t i=0;i< ipMesh->iData.size();i+=3)
		{
			auto i1 = ipMesh->iData.at(i);
			auto i2 = ipMesh->iData.at(i+1);
			auto i3 = ipMesh->iData.at(i+2);

			auto v1 = (ipMesh->vData.at(i1).pos + ipMesh->vData.at(i2).pos + ipMesh->vData.at(i3).pos)/3.0f;

			auto norm = glm::normalize(ipMesh->vData.at(i1).norm);
			v1 = glm::vec3(ipMesh->tMatrix*glm::vec4(v1,1.0f));
			auto v2 = v1 + (norm * nrmlLen);

			vData.push_back({ v1,norm,glm::vec2(0,1) });
			vData.push_back({ v2,norm,glm::vec2(0,1) });
			iData.push_back((unsigned int)iData.size());
			iData.push_back((unsigned int)iData.size());
		}

		auto nrmlsMesh = std::make_shared<Mesh>(vData, iData);
		nrmlsMesh->name = "Nrmls Mesh";
		nrmlsMesh->drawCommand = GL_LINES;
		return nrmlsMesh;
	}

	int intersects3D_RayTrinagle(vector<glm::vec3> R, vector<glm::vec3> T, glm::vec3& intersectionPt)
	{
		glm::vec3    u, v, n;              // triangle vectors
		glm::vec3    dir, w0, w;           // ray vectors
		float     r, a, b;              // params to calc ray-plane intersect

										// get triangle edge vectors and plane normal
		u = T.at(1) - T.at(0);
		v = T.at(2) - T.at(0);
		n = glm::cross(u, v);              // cross product

		if (n.x == 0
			&& n.y == 0
			&& n.z == 0)             // triangle is degenerate
			return -1;                  // do not deal with this case

		dir = R.at(1) - R.at(0);              // ray direction vector
		w0 = R.at(0) - T.at(0);
		a = -glm::dot(n, w0);
		b = glm::dot(n, dir);
		if (fabs(b) < SMALL_NUM)
		{     // ray is  parallel to triangle plane
			if (a == 0)                 // ray lies in triangle plane
				return 2;
			else return 0;              // ray disjoint from plane
		}

		// get intersect point of ray with triangle plane
		r = a / b;
		if (r < 0.0)                    // ray goes away from triangle
			return 0;                   // => no intersect
										// for a segment, also test if (r > 1.0) => no intersect

		intersectionPt = R.at(0) + r * dir;            // intersect point of ray and plane

													   // is I inside T?
		float    uu, uv, vv, wu, wv, D;
		uu = glm::dot(u, u);
		uv = glm::dot(u, v);
		vv = glm::dot(v, v);
		w = intersectionPt - T.at(0);
		wu = glm::dot(w, u);
		wv = glm::dot(w, v);
		D = uv * uv - uu * vv;

		// get and test parametric coords
		float s, t;
		s = (uv * wv - vv * wu) / D;
		if (s < 0.0 || s > 1.0)         // I is outside T
			return 0;
		t = (uv * wu - uu * wv) / D;
		if (t < 0.0 || (s + t) > 1.0)  // I is outside T
			return 0;

		return 1;                       // I is in T
	}

	glm::vec3 getNormal(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
	{
		auto p = v2 - v1;
		auto q = v3 - v1;
		return glm::cross(p, q);
	}

	Mesh::Mesh(vector<VertexData> vData, vector<unsigned int> iData)
	{
		this->vData = vData;
		this->iData = iData;
		vbo = makeVetexBufferObject(vData);
		ibo = makeIndexBufferObject(iData);
		vao = makeVertexArrayObject(vbo, ibo);
		drawCount = (unsigned int)iData.size();
		tMatrix = glm::mat4(1);
	}

	Mesh::Mesh(vector<VDPosNormColr> vData, vector<unsigned int> iData)
	{
		this->vdPosNrmClr = vData;
		this->iData = iData;
		vbo = makeVetexBufferObject(vData);
		ibo = makeIndexBufferObject(iData);
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VDPosNormColr), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VDPosNormColr), (void*)sizeof(glm::vec3));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VDPosNormColr), (void*)(sizeof(glm::vec3) * 2));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		drawCount = (unsigned int)iData.size();
		tMatrix = glm::mat4(1);
	}

	Mesh::Mesh(vector<glm::vec3> vData, vector<unsigned int> iData)
	{
		this->vDataVec3 = vData;
		this->iData = iData;
		vbo = makeVetexBufferObject(vData);
		ibo = makeIndexBufferObject(iData);
		vao = makeVertexArrayObjectVec3(vbo, ibo);
		drawCount = (unsigned int)iData.size();
		tMatrix = glm::mat4(1);
	}

	Mesh::Mesh(vector<VertexData> vData, vector<unsigned int> iData, vector<Texture> textures)
	{
		this->vData = vData;
		this->iData = iData;
		this->textures = textures;
		vbo = makeVetexBufferObject(vData);
		ibo = makeIndexBufferObject(iData);
		vao = makeVertexArrayObject(vbo, ibo);
		drawCount = (unsigned int)iData.size();
		tMatrix = glm::mat4(1);
	}

	void Mesh::updateIbo()
	{
		if (ibo > 0)
			glDeleteBuffers(1, &ibo);

		ibo = makeIndexBufferObject(iData);
		drawCount = (unsigned int)iData.size();
	}

	void Mesh::draw()
	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glDrawElements(drawCommand, drawCount, GL_UNSIGNED_INT, (void*)0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Mesh::draw(vector<DrawRange> ranges)
	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		for (auto range : ranges)
			glDrawElements(drawCommand, range.drawCount, GL_UNSIGNED_INT, (void*)(range.offset * sizeof(GLuint)));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Mesh::draw(DrawRange range)
	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glDrawElements(range.drawCommand, range.drawCount, GL_UNSIGNED_INT, (void*)(range.offset * sizeof(GLuint)));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Mesh::drawInstanced(int iCount)
	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glDrawElements(drawCommand, drawCount, GL_UNSIGNED_INT, (void*)0);
		glDrawElementsInstanced(drawCommand, drawCount, GL_UNSIGNED_INT, (void*)0, iCount);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	Mesh::~Mesh()
	{
		cout << "Cleaning mesh obj : " <<name <<'\n';
		if (vbo > 0)
		{
			glDeleteBuffers(1, &vbo);
			vbo = 0;
		}

		if (ibo > 0)
		{
			glDeleteBuffers(1, &ibo);
			ibo = 0;
		}

		if (vao > 0)
		{
			glDeleteVertexArrays(1, &vao);
		}
	}

	

	MeshGLES::MeshGLES(vector<VDPosUV> vData, vector<unsigned short> iData)
	{
		this->vData = vData;
		this->iData = iData;

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VDPosUV) * vData.size(), (void*)vData.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//ibo = makeIndexBufferObject(iData);
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * iData.size(), (void*)iData.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		drawCount = (unsigned int)iData.size();
		tMatrix = glm::mat4(1);
	}

	void MeshGLES::updateIbo()
	{
	}

	void MeshGLES::draw()
	{
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);
		glDrawElements(GL_TRIANGLES, drawCount, GL_UNSIGNED_SHORT, (void*)0);
	}

	MeshGLES::~MeshGLES()
	{
		
	}

}