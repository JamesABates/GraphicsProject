#include <fstream>
#include <vector>
#include <string>
#include "glm\glm.hpp"
#include "ObjectLoader.h"
#include "FlyCamera.h"

using namespace std;

ObjectLoader::ObjectLoader(FlyCamera* camera)
{
	m_camera = camera;

	m_lightDirX = 0.1f;
	m_lightDirY = 0.0f;
	m_lightDirZ = 0.0f;
	m_specPow = 0.0f;

	std::string err = tinyobj::LoadObj(shapes, materials, "./Content/OBJ/Knife.obj");
	createOpenGLBuffers(shapes);
	CreateShader();

}

ObjectLoader::~ObjectLoader()
{

}

void ObjectLoader::Update()
{

}

void ObjectLoader::Draw()
{
	glUseProgram(m_program_id);

	int view_proj_uniform = glGetUniformLocation(m_program_id, "ProjectionView");
	int lightDirection = glGetUniformLocation(m_program_id, "LightDir");
	int lightColour = glGetUniformLocation(m_program_id, "LightColour");
	int cameraPos = glGetUniformLocation(m_program_id, "CameraPos");
	int specPow = glGetUniformLocation(m_program_id, "SpecPow");

	glUniformMatrix4fv(view_proj_uniform, 1, GL_FALSE,(float*)&m_camera->GetProjectionView());
	glUniform3f(lightDirection, m_lightDirX, m_lightDirY, m_lightDirZ);
	glUniform3f(lightColour, m_lightR, m_lightG, m_lightB);
	//glUniformMatrix4fv(view_proj_uniform, 1, GL_FALSE,(float*)&m_camera->GetPosition());
	glUniform1f(specPow, m_specPow);

	for (unsigned int i = 0; i < m_gl_info.size(); ++i)
	{
	 glBindVertexArray(m_gl_info[i].m_VAO);
	 glDrawElements(GL_TRIANGLES,m_gl_info[i].m_index_count,GL_UNSIGNED_INT, 0);
	}

}

void ObjectLoader::createOpenGLBuffers(std::vector<tinyobj::shape_t>& shapes)
{
	m_gl_info.resize(shapes.size());
	for (unsigned int mesh_index = 0; mesh_index < shapes.size(); ++mesh_index)
	{
		glGenVertexArrays(1, &m_gl_info[mesh_index].m_VAO);
		glGenBuffers(1, &m_gl_info[mesh_index].m_VBO);
		glGenBuffers(1, &m_gl_info[mesh_index].m_IBO);
		glBindVertexArray(m_gl_info[mesh_index].m_VAO);

		unsigned int float_count = shapes[mesh_index].mesh.positions.size();
		float_count += shapes[mesh_index].mesh.normals.size();
		float_count += shapes[mesh_index].mesh.texcoords.size();

		std::vector<float> vertex_data;
		vertex_data.reserve(float_count);

		vertex_data.insert(vertex_data.end(),
			shapes[mesh_index].mesh.positions.begin(),
			shapes[mesh_index].mesh.positions.end());

		vertex_data.insert(vertex_data.end(),
			shapes[mesh_index].mesh.normals.begin(),
			shapes[mesh_index].mesh.normals.end());

		m_gl_info[mesh_index].m_index_count =
			shapes[mesh_index].mesh.indices.size();

		glBindBuffer(GL_ARRAY_BUFFER, m_gl_info[mesh_index].m_VBO);
		glBufferData(GL_ARRAY_BUFFER,
			vertex_data.size() * sizeof(float),
			vertex_data.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_info[mesh_index].m_IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			shapes[mesh_index].mesh.indices.size() * sizeof(unsigned int),
			shapes[mesh_index].mesh.indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); //position
		glEnableVertexAttribArray(1); //normal data

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0,
		(void*)(sizeof(float)*shapes[mesh_index].mesh.positions.size()));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void ObjectLoader::CreateShader()
{
	const char* vsSource = "#version 410\n \
							layout(location=0) in vec4 Position; \
							layout(location=1) in vec4 Normal; \
							out vec4 vNormal; \
							out vec4 vPosition; \
							uniform mat4 ProjectionView; \
							void main() { vNormal = Normal; \
							vPosition = Position; \
							gl_Position = ProjectionView*Position; }";
	
	const char* fsSource = "#version 410\n \
							in vec4 vNormal; \
							in vec4 vPosition; \
							out vec4 FragColor; \
							uniform vec3 LightDir; \
							uniform vec3 LightColour; \
							uniform vec3 CameraPos; \
							uniform float SpecPow; \
							void main() { \
							float d = max(0, dot(normalize(vNormal.xyz), -LightDir ) ); \
							vec3 E = normalize( CameraPos - vPosition.xyz );\
							vec3 R = reflect( -LightDir, vNormal.xyz ); \
							float s = max( 0, dot( E, R ) ); \
							s = pow( s, SpecPow ); \
							FragColor = vec4( LightColour * d + LightColour * s, 1); }";	int success = GL_FALSE;	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);		glShaderSource(vertexShader, 1, (const char**)&vsSource, 0);	glCompileShader(vertexShader);	glShaderSource(fragmentShader, 1, (const char**)&fsSource, 0);	glCompileShader(fragmentShader);		m_program_id = glCreateProgram();
	glAttachShader(m_program_id, vertexShader);
	glAttachShader(m_program_id, fragmentShader);
	glLinkProgram(m_program_id);		glGetProgramiv(m_program_id, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) 
	{
		int infoLogLength = 0;
		glGetShaderiv(m_program_id, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength];
	
		glGetShaderInfoLog(m_program_id, infoLogLength, 0, infoLog);
		printf("Error: Failed to link shader program!\n");
		printf("%s\n", infoLog);
	
		delete[] infoLog;
	}
	
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
}