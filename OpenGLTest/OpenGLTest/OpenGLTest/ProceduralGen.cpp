#include "ProceduralGen.h"

ProcedualGen::ProcedualGen(FlyCamera* camera, GLFWwindow* window, AntTweakBar* gui)
{
	m_gui = gui;
	m_camera = camera;
	m_regenereate = false;
	m_window = window;

	GenerateGrid(100, 100);
	GenerateOpenGLBuffers();
	CreateShaders();
	GeneratePerlin();
}

ProcedualGen::~ProcedualGen()
{

}

void ProcedualGen::CreateShaders()
{
	//create shaders
	const char* vsSource = "#version 410\n \
   						   layout(location=0) in vec4 position; \
						   layout(location=1) in vec2 texcoord;\
						   layout(location=2) in vec4 colour; \
						   \
						   out vec2 frag_texcoord;\
						   out vec4 vColour; \
						   \
						   uniform mat4 ProjectionView; \
						   uniform sampler2D m_perlin_texture;\
						   uniform sampler2D m_grass_texture;\
						   uniform sampler2D m_water_texture;\
						   uniform sampler2D m_rocks_texture;\
						   \
						   void main()\
						   {\
								vec4 pos = position;\
								pos.y += texture(m_perlin_texture, texcoord).r * 50;\
								frag_texcoord = texcoord;\
								gl_Position = ProjectionView * pos;\
						    }";

	const char* fsSource = "#version 410\n \
						   	in vec2 frag_texcoord;\
							in vec4 vColour; \
							out vec4 out_color;\
							uniform sampler2D m_perlin_texture;\
							uniform sampler2D m_grass_texture;\
							uniform sampler2D m_water_texture;\
							uniform sampler2D m_rocks_texture;\
							void main()\
							{\
								float height = texture(m_perlin_texture, frag_texcoord).r;\
								out_color = texture(m_perlin_texture, frag_texcoord).rrrr;\
								out_color.a = 1;\
								if(height <= 0.45)\
								{\
									out_color = texture(m_perlin_texture, frag_texcoord).rrrr*texture(m_water_texture, frag_texcoord*2);\
								}\
								else if(height >= 0.45 && height <= 0.5)\
								{\
									out_color = texture(m_perlin_texture, frag_texcoord).rrrr*texture(m_rocks_texture, frag_texcoord*2);\
								}\
								else\
								{\
									out_color = texture(m_perlin_texture, frag_texcoord).rrrr*texture(m_grass_texture, frag_texcoord*2);\
								}\
							}";

	int success = GL_FALSE;
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	
	glShaderSource(vertexShader, 1, (const char**)&vsSource, 0);
	glCompileShader(vertexShader);
	glShaderSource(fragmentShader, 1, (const char**)&fsSource, 0);
	glCompileShader(fragmentShader);
	
	m_program = glCreateProgram();
	glAttachShader(m_program, vertexShader);
	glAttachShader(m_program, fragmentShader);
	glLinkProgram(m_program);
	
	glGetProgramiv(m_program, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) 
	{
		int infoLogLength = 0;
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength];
		glGetProgramInfoLog(m_program, infoLogLength, 0, infoLog);
		printf("Error: Failed to link shader program!\n");
		printf("%s\n", infoLog);
		delete[] infoLog;
	}
	
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
}

void ProcedualGen::GenerateOpenGLBuffers()
{
	data = stbi_load("./Content/Images/Grass.jpg", &imageWidth, &imageHeight, &imageFormat, STBI_default);

	glGenTextures(1, &m_grass_texture);
	glBindTexture(GL_TEXTURE_2D, m_grass_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);

	data = stbi_load("./Content/Images/Water.jpg", &imageWidth, &imageHeight, &imageFormat, STBI_default);

	glGenTextures(1, &m_water_texture);
	glBindTexture(GL_TEXTURE_2D, m_water_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);

	data = stbi_load("./Content/Images/Rock.jpg", &imageWidth, &imageHeight, &imageFormat, STBI_default);

	glGenTextures(1, &m_rocks_texture);
	glBindTexture(GL_TEXTURE_2D, m_rocks_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void ProcedualGen::GenerateGrid(unsigned int rows, unsigned int cols)
{
	Vertex2* aoVertices = new Vertex2[rows * cols];
	for (unsigned int r = 0; r < rows; ++r)
	{
		for (unsigned int c = 0; c < cols; ++c)
		{
			aoVertices[r * cols + c].position = vec4((float)c, 0, (float)r, 1);
			aoVertices[r * cols + c].texcoord = glm::vec2((float)c / cols, (float)r / rows);

			vec3 colour = vec3(sinf((c / (float)(cols - 1)) *
				(r / (float)(rows - 1))));
			aoVertices[r * cols + c].colour = vec4(colour, 1);
		}
	}

	unsigned int* auiIndices = new unsigned int[(rows - 1) * (cols - 1) * 6];

	unsigned int index = 0;
	for (unsigned int r = 0; r < (rows - 1); ++r)
	{
		for (unsigned int c = 0; c < (cols - 1); ++c)
		{
			auiIndices[index++] = r * cols + c;
			auiIndices[index++] = (r + 1) * cols + c;
			auiIndices[index++] = (r + 1) * cols + (c + 1);

			auiIndices[index++] = r * cols + c;
			auiIndices[index++] = (r + 1) * cols + (c + 1);
			auiIndices[index++] = r * cols + (c + 1);
		}
	}

	m_indexcount = index;

	// Create VAO, VBO and IBO
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_IBO);

	// Bind buffers
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);

	// send data to the buffers	
	glBufferData(GL_ARRAY_BUFFER, (rows * cols) * sizeof(Vertex2), aoVertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexcount * sizeof(unsigned int), auiIndices, GL_STATIC_DRAW);

	// describe how the vertices are setup so they can be sent to the shader
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (void*)(sizeof(vec4)));

	// m_VAO hold all our ARRAY_BUFFER and ARRAY_ELEMENT_BUFFER settings
	// so just rebind it later before using glDrawElements
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete[] auiIndices;
	delete[] aoVertices;
}

void ProcedualGen::GeneratePerlin()
{
	int dims = 257;
	float *perlin_data = new float[dims * dims];
	float scale = (1.0f / dims) * m_gui->m_scaleMultiplier;
	int octaves = m_gui->m_octaves;

	for (int x = 0; x < dims; x++)
	{
		for (int y = 0; y < dims; y++)
		{
			float amplitude = m_gui->m_amplitude;
			float persistence = m_gui->m_persistence;
			perlin_data[y* dims + x] = 0;

			for (int o = 0; o < octaves; o++)
			{
				float freq = powf(2, (float)o);
				float perlin_sample = glm::perlin(glm::vec2((float)x, (float)y) * scale * freq) * 0.5f + 0.5f;

				perlin_data[y * dims + x] += perlin_sample * amplitude;
				amplitude *= persistence;
			}
		}
	}

	glGenTextures(1, &m_perlin_texture);
	glBindTexture(GL_TEXTURE_2D, m_perlin_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, dims, dims, 0, GL_RED, GL_FLOAT, perlin_data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void ProcedualGen::Update(float dt)
{
	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS && m_regenereate == false)
	{
		m_regenereate = true;
		GeneratePerlin();
	}

	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_RELEASE)
		m_regenereate = false;
}

void ProcedualGen::Draw()
{

	glUseProgram(m_program);
	unsigned int projectionViewUniform = glGetUniformLocation(m_program, "ProjectionView");
	glUniformMatrix4fv(projectionViewUniform, 1, false, &m_camera->GetProjectionView()[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_perlin_texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_grass_texture);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_water_texture);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_rocks_texture);

	int loc = glGetUniformLocation(m_program, "m_perlin_texture");
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(m_program, "m_grass_texture");
	glUniform1i(loc, 1);

	loc = glGetUniformLocation(m_program, "m_water_texture");
	glUniform1i(loc, 2);

	loc = glGetUniformLocation(m_program, "m_rocks_texture");
	glUniform1i(loc, 3);

	glBindVertexArray(m_VAO);
	unsigned int indexCount = ((100 - 1) * (100 - 1) * 6);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

}