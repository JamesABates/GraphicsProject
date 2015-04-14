
#include "ShadowMap.h"

ShadowMap::ShadowMap(FlyCamera* flyCamera)
{
	m_flyCamera = flyCamera;

	//CreateOpenGLBuffers(m_fbx);
	SetupShadowBuffer();
	CreateShaders();
	CreateShadowShaders();
}

ShadowMap::~ShadowMap()
{

}

void ShadowMap::CreateOpenGLBuffers(FBXFile* fbx)
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

void ShadowMap::SetupShadowBuffer()
{
	// setup shadow map buffer
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glGenTextures(1, &m_fboDepth);
	glBindTexture(GL_TEXTURE_2D, m_fboDepth);

	// texture uses a 16-bit depth component format
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE);

	// attached as a depth attachment to capture depth not colour
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		m_fboDepth, 0);

	// no colour targets are used
	glDrawBuffer(GL_NONE);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		printf("Framebuffer Error!\n");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::CreateShaders()
{
		const char* vsSource = "#version 410\n \
							in vec4 Position;\
							in vec4 Normal;\
							\
							out vec4 vNormal;\
							out vec4 vShadowCoord;\
							\
							uniform mat4 ProjectionView;\
							uniform mat4 LightMatrix;\
							void main() \
							{\
								vNormal = Normal;\
								gl_Position = ProjectionView * Position;\
								vShadowCoord = LightMatrix * Position;\
							}";
	
	const char* fsSource = "#version 410\n \
							in vec4 vNormal;\
							in vec4 vShadowCoord;\
							\
							out vec4 FragColour;\
							uniform vec3 lightDir;\
							uniform sampler2D shadowMap;\
							uniform float shadowBias;\
							\
							void main()	\
							{\
								float d = max(0, dot(normalize(vNormal.xyz), lightDir));\
								\
								if (texture(shadowMap, vShadowCoord.xy).r < vShadowCoord.z - shadowBias)\
								{\
									d = 0;\
								}\
								FragColour = vec4(d, d, d, 1);\
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

void ShadowMap::CreateShadowShaders()
{
		const char* vsSource = "#version 410\n \
							in vec4 Position;\
							uniform mat4 LightMatrix;\
							void main() \
							{\
								gl_Position = LightMatrix * Position;\
							}";
	
	const char* fsSource = "#version 410\n \
							out float FragDepth; \
							void main() \
							{\
								FragDepth = gl_FragCoord.z;\
							}";

	int success = GL_FALSE;
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertexShader, 1, (const char**)&vsSource, 0);
	glCompileShader(vertexShader);
	glShaderSource(fragmentShader, 1, (const char**)&fsSource, 0);
	glCompileShader(fragmentShader);

	m_shaderProgram = glCreateProgram();

	glAttachShader(m_shaderProgram, vertexShader);
	glAttachShader(m_shaderProgram, fragmentShader);
	glLinkProgram(m_shaderProgram);
	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);

	if (success == GL_FALSE)
	{
		int infoLogLength = 0;
		glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength];
		glGetProgramInfoLog(m_shaderProgram, infoLogLength, 0, infoLog);
		printf("Error: Failed to link shader program!\n");
		printf("%s\n", infoLog);
		delete[] infoLog;
	}

	glDeleteShader(fragmentShader);
}

void ShadowMap::Update(float dt)
{
	m_lightDirection = glm::normalize(glm::vec3(1, 1.25f, 1));

	glm::mat4 lightProjection = glm::ortho<float>(-10, 10, -10, 10, -10, 10);
	glm::mat4 lightView = glm::lookAt(m_lightDirection, glm::vec3(0), glm::vec3(0, 1, 0));

	m_lightMatrix = lightProjection * lightView;

	m_flyCamera->Update(dt);
}

void ShadowMap::Draw()
{
	// shadow pass: bind our shadow map target and clear the depth
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, 1024, 1024);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_shaderProgram);

	// bind the light matrix
	int loc = glGetUniformLocation(m_shaderProgram, "LightMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &(m_lightMatrix[0][0]));

	//// draw all shadow-casting geometry
	//for (unsigned int i = 0; i < m_fbx->getMeshCount(); ++i)
	//{
	//	FBXMeshNode* mesh = m_fbx->getMeshByIndex(i);
	//	unsigned int* glData = (unsigned int*)mesh->m_userData;
	//	glBindVertexArray(glData[0]);
	//	glDrawElements(GL_TRIANGLES,
	//		(unsigned int)mesh->m_indices.size(),
	//		GL_UNSIGNED_INT, 0);
	//}

	// final pass: bind back-buffer and clear colour and depth
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 1920, 1080);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_program);

	// bind the camera
	loc = glGetUniformLocation(m_program, "ProjectionView");
	glUniformMatrix4fv(loc, 1, GL_FALSE,
		&(m_flyCamera->GetProjectionView()[0][0]));

	glm::mat4 textureSpaceOffset(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f
		);

	glm::mat4 lightMatrix = textureSpaceOffset * m_lightMatrix;

	loc = glGetUniformLocation(m_program, "LightMatrix");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &lightMatrix[0][0]);

	loc = glGetUniformLocation(m_program, "lightDir");
	glUniform3fv(loc, 1, &m_lightDirection[0]);

	loc = glGetUniformLocation(m_program, "shadowMap");
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(m_program, "shadowBias");
	glUniform1f(loc, 0.01f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fboDepth);

	//// bind our vertex array object and draw the mesh
	//for (unsigned int i = 0; i < m_fbx->getMeshCount(); ++i) {
	//	FBXMeshNode* mesh = m_fbx->getMeshByIndex(i);
	//	unsigned int* glData = (unsigned int*)mesh->m_userData;
	//	glBindVertexArray(glData[0]);
	//	glDrawElements(GL_TRIANGLES,
	//		(unsigned int)mesh->m_indices.size(),
	//		GL_UNSIGNED_INT, 0);
	//}

	// draw a plane under the bunny
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

}