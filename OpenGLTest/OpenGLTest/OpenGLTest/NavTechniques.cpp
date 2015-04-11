#include "NavTechniques.h"

NavTechniques::NavTechniques(FlyCamera* camera)
{
	m_camera = camera;

	m_sponza = new FBXFile();
	m_sponza->load("./Content/FBX/SponzaSimple.fbx", FBXFile::UNITS_METER, true, true, true);

	m_navMesh = new FBXFile();
	m_navMesh->load("./Content/FBX/SponzaSimpleNavMesh.fbx", FBXFile::UNITS_METER);

	m_modelWorld = glm::mat4(1, 0, 0, 0,
							 0, 1, 0, 0,
							 0, 0, 1, 0,
							 0, 0, 0, 1);

	CreateOpenGLBuffers(m_sponza);
	CreateShaders();
}

NavTechniques::~NavTechniques()
{

}

void NavTechniques::CreateOpenGLBuffers(FBXFile* fbx)
{
	float vertexData[] = {
		-10, 0, -10, 1, 0, 0,
		10, 0, -10, 1, 1, 0,
		-10, 0, 10, 1, 1, 1,
		10, 0, 10, 1, 0, 1,
	};
	unsigned int indexData[] = {
		0, 1, 2,
		1, 2, 3,
	};
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	glGenBuffers(1, &m_vbo);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 6 * 4, vertexData, GL_STATIC_DRAW);

	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* 6, indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)* 6, 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)* 6, ((char*)0) + 16);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// create the GL VAO/VBO/IBO data for each mesh
	for (unsigned int i = 0; i < fbx->getMeshCount(); ++i)
	{
		FBXMeshNode* mesh = fbx->getMeshByIndex(i);
		// storage for the opengl data in 3 unsigned int
		unsigned int* glData = new unsigned int[3];

		glGenVertexArrays(1, &glData[0]);
		glBindVertexArray(glData[0]);

		glGenBuffers(1, &glData[1]);
		glGenBuffers(1, &glData[2]);

		glBindBuffer(GL_ARRAY_BUFFER, glData[1]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glData[2]);

		glBufferData(GL_ARRAY_BUFFER, mesh->m_vertices.size() * sizeof(FBXVertex), mesh->m_vertices.data(), GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->m_indices.size() * sizeof(unsigned int), mesh->m_indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), 0);

		glEnableVertexAttribArray(1); // normal
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), ((char*)0) + FBXVertex::NormalOffset);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		mesh->m_userData = glData;
	}
}

void NavTechniques::CreateShaders()
{
	const char* vsSource = "#version 410\n \
							layout(location = 0) in vec4 position;\
							layout(location = 1) in vec4 normal;\
							out vec4 worldPosition;\
							out vec4 worldNormal;\
							uniform mat4 projectionView;\
							uniform mat4 model;\
							uniform mat4 invTransposeModel;\
							void main() {\
								worldPosition = model * position;\
								worldNormal = invTransposeModel * normal;\
								gl_Position = projectionView * position;\
							}";

	const char* fsSource = "#version 410\n \
							in vec4 worldPosition;\
							in vec4 worldNormal;\
							layout(location = 0) out vec4 fragColour;\
							void main() {\
							vec3 colour = vec3(1);\
							if (mod(worldPosition.x, 1.0) < 0.05f ||\
								mod(worldPosition.y, 1.0) < 0.05f ||\
								mod(worldPosition.z, 1.0) < 0.05f)\
								colour = vec3(0);\
							float d = max(0, dot(normalize(vec3(1, 1, 1)),\
								normalize(worldNormal.xyz))) * 0.75f;\
							fragColour.rgb = colour * 0.25f + colour * d;\
							fragColour.a = 1;\
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

void NavTechniques::Update()
{

	for (auto& first : m_graph) 
	{
		for (auto& second : m_graph) 
		{
			// don't link to self
			if (&first == &second)
				continue;

			float A = first.position.x;
			float B = first.position.y;
			float C = first.position.z;

			float X = second.vertices->x;
			float Y = second.vertices->y;
			float Z = second.vertices->z;

			if ((A == X && B == Y) || (A == Y && B == Z) || (A == Z && B == X) ||
				(A == Y && B == X) || (A == Z && B == Y) || (A == X && B == Z))
			{
				first.edgeTargets[0] = &second;
			}
			// ABC XYZ
		}
	}
}

void NavTechniques::Draw()
{
	// final pass: bind back-buffer and clear colour and depth
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 1280, 720);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_program);

	// bind the camera
	int loc = glGetUniformLocation(m_program, "projectionView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &(m_camera->GetProjectionView()[0][0]));

	glm::mat4x4 modelLocation = glm::mat4x4(1);
	int loc2 = glGetUniformLocation(m_program, "model");
	glUniformMatrix4fv(loc2, 1, GL_FALSE, &(modelLocation[0][0]));

	glm::mat4x4 invNormalLocation = glm::mat4x4(1);
	glm::inverse(invNormalLocation);
	int loc3 = glGetUniformLocation(m_program, "invTransposeModel");
	glUniformMatrix4fv(loc3, 1, GL_FALSE, &(invNormalLocation[0][0]));

	// bind our vertex array object and draw the mesh
	for (unsigned int i = 0; i < m_sponza->getMeshCount(); ++i) {
		FBXMeshNode* mesh = m_sponza->getMeshByIndex(i);
		unsigned int* glData = (unsigned int*)mesh->m_userData;
		glBindVertexArray(glData[0]);
		glDrawElements(GL_TRIANGLES,
			(unsigned int)mesh->m_indices.size(),
			GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

}
