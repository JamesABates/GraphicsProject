#include "ProceduralGen.h"

ProcedualGen::ProcedualGen(FlyCamera* camera)
{
	m_camera = camera;

	GenerateGrid(100, 100);
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
							layout(location = 1) in vec2 texcoord; \
							uniform mat4 ProjectionView; \
							out vec2 frag_texcoord; \
							uniform sampler2D m_perlin_texture; \
							void main() \
							{ \
								vec4 pos = position; \
								pos.y += texture(m_perlin_texture, texcoord).r * 5; \
								frag_texcoord = texcoord; \
								gl_Position = ProjectionView * pos; \
							}";
	
	const char* fsSource = "#version 410\n \
							in vec2 frag_texcoord; \
							out vec4 out_color; \
							uniform sampler2D m_perlin_texture; \
							void main() \
							{ \
									out_color = texture(m_perlin_texture, frag_texcoord).rrrr; \
									out_color.a = 1; \
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
		glGetShaderiv(m_program, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength];
	
		glGetShaderInfoLog(m_program, infoLogLength, 0, infoLog);
		printf("Error: Failed to link shader program!\n");
		printf("%s\n", infoLog);
	
		delete[] infoLog;
	}
	
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
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
	float scale = (1.0f / dims) * 3;
	int octaves = 6;

	for (int x = 0; x < dims; x++)
	{
		for (int y = 0; y < dims; y++)
		{
			float amplitude = 1;
			float persistence = 0.3;
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

void ProcedualGen::Draw()
{

	glUseProgram(m_program);
	unsigned int projectionViewUniform = glGetUniformLocation(m_program, "ProjectionView");
	glUniformMatrix4fv(projectionViewUniform, 1, false, &m_camera->GetProjectionView()[0][0]);
	glBindVertexArray(m_VAO);
	unsigned int indexCount = ((100 - 1) * (100 - 1) * 6);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

}