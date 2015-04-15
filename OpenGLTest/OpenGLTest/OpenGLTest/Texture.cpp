#include "Texture.h"

//#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Texture::Texture(FlyCamera* camera, AntTweakBar* gui)
{
	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data;
	m_timer = 0;
	m_gui = gui;

	m_fbxFile = new FBXFile();
	m_fbxFile->load("./Content/FBX/pyro/pyro.fbx", FBXFile::UNITS_METER, true, true, true);
	m_fbxFile->initialiseOpenGLTextures();

	createOpenGLBuffers(m_fbxFile);

	m_camera = camera;

	m_emitter = new ParticleSystem();
	m_emitter->initalise(1000, 500, 0.1f, 1.0f, 1, 5, 1, 0.1f, glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 1));
	modelX = 0;
	modelY = 0;
	modelZ = 0;

	crouch = false;
	aniNumber = Idle;
}

void Texture::CreateShader()
{
	const char* vsSource = "#version 410\n																														\
							layout(location=0) in vec4 Position;																								\
							layout(location=1) in vec4 Normal;																									\
							layout(location=2) in vec4 Tangent;																									\
							layout(location=3) in vec2 TexCoord;																								\
							layout(location=4) in vec4 Weights;																									\
							layout(location=5) in vec4 Indices;																									\
																																								\
							out vec3 vPosition;																													\
							out vec2 vTexCoord;																													\
							out vec3 vNormal;																													\
							out vec3 vTangent;																													\
							out vec3 vBiTangent;																												\
																																								\
							uniform mat4 ProjectionView;																										\
							uniform mat4 global;																												\
							uniform vec3 offset;																												\
																																								\
							const int MAX_BONES = 128;																											\
							uniform mat4 bones[MAX_BONES];																										\
																																								\
							void main()																															\
							{                                                                                                                                   \																									\
								vec4 totalOffset = vec4(offset.x, offset.y, offset.z, 0.0);																		\
								vPosition = Position.xyz + totalOffset.xyz;																										\
								vTexCoord = TexCoord;																											\
								vNormal = Normal.xyz;																											\
								vTangent = Tangent.xyz;																											\
								vBiTangent = cross(vNormal, vTangent);																							\
								gl_Position = ProjectionView * Position + totalOffset;																						\
																																								\
								ivec4 index = ivec4(Indices);																									\
																																								\
								vec4 P = bones[index.x]*Position*Weights.x;																						\
								P += bones[index.y]*Position*Weights.y;																							\
								P += bones[index.z]*Position*Weights.z;																							\
								P += bones[index.w]*Position*Weights.w;\
								P += totalOffset;\
																																								\
								gl_Position = ProjectionView * global * P;																						\
								}";

	const char* fsSource = "#version 410\n																														\
							in vec2 vTexCoord;																													\
							in vec3 vNormal;																													\
							in vec3 vTangent;																													\
							in vec3 vBiTangent;																													\
							out vec4 FragColor;																													\
							uniform vec3 LightDir;																												\
							uniform sampler2D diffuse;																											\
							uniform sampler2D normal;																											\
							void main()																															\
							{																																	\
								float a = 0.05; \
								mat3 TBN = mat3(normalize( vTangent ), normalize( vBiTangent ), normalize( vNormal ));											\
								vec3 N = texture(normal, vTexCoord).xyz * 2 - 1;																				\
								float d = max( 0, dot( normalize( TBN * N ), normalize( -LightDir )));															\
								FragColor = texture(diffuse, vTexCoord)*vec4(d+a, d+a, d+a, 1);																						\
							}";

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, (const char**)&vsSource, 0);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, (const char**)&fsSource, 0);
	glCompileShader(fragmentShader);

	m_program = glCreateProgram();

	glAttachShader(m_program, vertexShader);
	glAttachShader(m_program, fragmentShader);
	glLinkProgram(m_program);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Texture::createOpenGLBuffers(FBXFile* fbx)
{
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

		glEnableVertexAttribArray(0); //position
		glEnableVertexAttribArray(1); //normals
		glEnableVertexAttribArray(2); //tangents
		glEnableVertexAttribArray(3); //texcoords
		glEnableVertexAttribArray(4); //weights
		glEnableVertexAttribArray(5); //indices

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::PositionOffset);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), (void*)FBXVertex::NormalOffset);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(FBXVertex), (void*)FBXVertex::TangentOffset);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::TexCoord1Offset);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::WeightsOffset);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (void*)FBXVertex::IndicesOffset);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		mesh->m_userData = glData;
	}
}

void Texture::Update(float dt)
{
	m_timer += dt;

	m_emitter->Update(dt, m_camera->GetTransform());

	if (glfwGetKey(m_pWindow, GLFW_KEY_T) == GLFW_PRESS)
	{
		glm::vec3 emitterPosition = m_emitter->GetPosition();
		m_emitter->SetPosition(glm::vec3(emitterPosition.x, emitterPosition.y, emitterPosition.z -= 1));
		aniNumber = Run;
		modelZ += 50;
	}

	else
	{
		aniNumber = Idle;
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_Y) == GLFW_PRESS)
	{
		modelY -= 50;
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_U) == GLFW_PRESS)
	{
		modelY += 50;
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_N) == GLFW_PRESS)
	{
		aniNumber = Crouch;
	}

	else { crouch = false; }

	if (glfwGetKey(m_pWindow, GLFW_KEY_G) == GLFW_PRESS)
	{
		glm::vec3 emitterPosition = m_emitter->GetPosition();
		m_emitter->SetPosition(glm::vec3(emitterPosition.x, emitterPosition.y, emitterPosition.z += 1));
		modelZ -= 50;
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_F) == GLFW_PRESS)
	{
		glm::vec3 emitterPosition = m_emitter->GetPosition();
		m_emitter->SetPosition(glm::vec3(emitterPosition.x -= 1, emitterPosition.y, emitterPosition.z));
		modelX += 50;
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_H) == GLFW_PRESS)
	{
		glm::vec3 emitterPosition = m_emitter->GetPosition();
		m_emitter->SetPosition(glm::vec3(emitterPosition.x += 1, emitterPosition.y, emitterPosition.z));
		modelX -= 50;
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_R) == GLFW_PRESS)
	{
		glm::vec3 emitterPosition = m_emitter->GetPosition();
		m_emitter->SetPosition(glm::vec3(emitterPosition.x, emitterPosition.y -= 1, emitterPosition.z));
	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_Y) == GLFW_PRESS)
	{
		glm::vec3 emitterPosition = m_emitter->GetPosition();
		m_emitter->SetPosition(glm::vec3(emitterPosition.x, emitterPosition.y += 1, emitterPosition.z));
	}

}

void Texture::Draw()
{
	glUseProgram(m_program);

	int lightDirection = glGetUniformLocation(m_program, "LightDir");
	int lightColour = glGetUniformLocation(m_program, "LightColour");
	int cameraPos = glGetUniformLocation(m_program, "CameraPos");
	int specPow = glGetUniformLocation(m_program, "SpecPow");

	glUniform3f(lightDirection, m_gui->m_light.x, m_gui->m_light.y, m_gui->m_light.z);
	glUniform3f(lightColour, m_gui->m_lightColour.r, m_gui->m_lightColour.g, m_gui->m_lightColour.b);
	//glUniformMatrix4fv(view_proj_uniform, 1, GL_FALSE,(float*)&m_camera->GetPosition());
	glUniform1f(specPow, m_gui->m_specPow);

	// grab the skeleton and animation we want to use
	skeleton = m_fbxFile->getSkeletonByIndex(0);
	animation = m_fbxFile->getAnimationByIndex(0);
	Animations();
	// evaluate the animation to update bones
	skeleton->evaluate(animation, m_timer);

	for (unsigned int bone_index = 0; bone_index < skeleton->m_boneCount; ++bone_index)
	{
		skeleton->m_nodes[bone_index]->updateGlobalTransform();
	}

	skeleton->updateBones();
	
	// bind the camera
	int loc = glGetUniformLocation(m_program, "ProjectionView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, &(m_camera->GetProjectionView()[0][0]));

	int bones_location = glGetUniformLocation(m_program, "bones");
	glUniformMatrix4fv(bones_location, skeleton->m_boneCount, GL_FALSE, (float*)skeleton->m_bones);

	//// set texture slots
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, m_fbxFile->getTextureByIndex(3)->handle);

	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, m_fbxFile->getTextureByIndex(1)->handle);

	// tell the shader where it is
	loc = glGetUniformLocation(m_program, "diffuse");
	glUniform1i(loc, 0);
	//loc = glGetUniformLocation(m_program, "Normal");
	//glUniform1i(loc, 1);

	loc = glGetUniformLocation(m_program, "offset");
	glUniform3f(loc, modelX, modelY, modelZ);

	// bind our vertex array object and draw the mesh
	for (unsigned int i = 0; i < m_fbxFile->getMeshCount(); ++i) 
	{
		FBXMeshNode* mesh = m_fbxFile->getMeshByIndex(i);
		unsigned int* glData = (unsigned int*)mesh->m_userData;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh->m_material->textures[FBXMaterial::DiffuseTexture]->handle);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mesh->m_material->textures[FBXMaterial::DiffuseTexture]->handle);

		int m_global = glGetUniformLocation(m_program, "global");
		glUniformMatrix4fv(m_global, 1, GL_FALSE, &(mesh->m_globalTransform)[0][0]);

		glBindVertexArray(glData[0]);
		glDrawElements(GL_TRIANGLES, (unsigned int)mesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	}

	glUseProgram(m_emitter->m_program);
	int loc2 = glGetUniformLocation(m_emitter->m_program, "projectionView");
	glUniformMatrix4fv(loc2, 1, GL_FALSE, &(m_camera->GetProjectionView()[0][0]));

	m_emitter->Draw();
}

void Texture::Animations()
{
	if (aniNumber == Idle)
	{
		animation->m_startFrame = 1;
		animation->m_endFrame = 90;
	}

	if (aniNumber == TakeDmgS)
	{
		animation->m_startFrame = 101;
		animation->m_endFrame = 160;
	}

	if (aniNumber == DeathS)
	{
		animation->m_startFrame = 171;
		animation->m_endFrame = 225;
	}

	if (aniNumber == Crouch)
	{
		if (crouch == false)
		{
			animation->m_startFrame = 235;
			animation->m_endFrame = 305;
			crouch = true;
		}

		else 
		{
			animation->m_startFrame = 305;
			animation->m_endFrame = 305;
		}
	}

	if (aniNumber == TakeDmgC)
	{
		animation->m_startFrame = 316;
		animation->m_endFrame = 375;
	}

	if (aniNumber == DeathC)
	{
		animation->m_startFrame = 386;
		animation->m_endFrame = 425;
	}

	if (aniNumber == Shoot)
	{
		animation->m_startFrame = 436;
		animation->m_endFrame = 635;
	}

	if (aniNumber == ReloadS)
	{
		animation->m_startFrame = 646;
		animation->m_endFrame = 835;
	}

	if (aniNumber == ReloadC)
	{
		animation->m_startFrame = 846;
		animation->m_endFrame = 985;
	}

	if (aniNumber == Run)
	{
		animation->m_startFrame = 995;
		animation->m_endFrame = 1019;
	}
}

