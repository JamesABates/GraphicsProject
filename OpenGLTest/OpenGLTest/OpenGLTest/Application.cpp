#include "Application.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;

//int testNumber = 10;

Application::Application()
{
	camera = new FlyCamera(100.0f, 0.5f); 
	camera->SetPosition(vec3(0,0,0));
	camera->SetPerspective(glm::pi<float>() * 0.25f, 16/9.f, 0.1f, 1000.f);
	camera->SetTransform(mat4(1,0,0,0,
							  0,1,0,0,
							  0,0,1,0,
							  0,0,0,1));
	indexCount = 0;

	if (glfwInit() == false)
		return;

	window = glfwCreateWindow(1280, 720, "Computer Graphics", nullptr, nullptr); 

	glfwMakeContextCurrent(window);

	if (ogl_LoadFunctions() == ogl_LOAD_FAILED)
	{
	 glfwDestroyWindow(window);
	 glfwTerminate();
	 return;
	}

	m_positions[0] = glm::vec3(10, 5, 10);
	m_positions[1] = glm::vec3(-10, 0, -10);
	m_rotations[0] = glm::quat(glm::vec3(0, -1, 0));
	m_rotations[1] = glm::quat(glm::vec3(0, 1, 0));

	m_hipFrames[0].position = glm::vec3(0, 5, 0);
	m_hipFrames[0].rotation = glm::quat(glm::vec3(1, 0, 0));
	m_hipFrames[1].position = glm::vec3(0, 5, 0);
	m_hipFrames[1].rotation = glm::quat(glm::vec3(-1, 0, 0));
	m_kneeFrames[0].position = glm::vec3(0, -2.5f, 0);
	m_kneeFrames[0].rotation = glm::quat(glm::vec3(1, 0, 0));
	m_kneeFrames[1].position = glm::vec3(0, -2.5f, 0);
	m_kneeFrames[1].rotation = glm::quat(glm::vec3(0, 0, 0));
	m_ankleFrames[0].position = glm::vec3(0, -2.5f, 0);
	m_ankleFrames[0].rotation = glm::quat(glm::vec3(-1, 0, 0));
	m_ankleFrames[1].position = glm::vec3(0, -2.5f, 0);
	m_ankleFrames[1].rotation = glm::quat(glm::vec3(0, 0, 0));

	crate = "./Content/Images/crate.png";

	antTweakBar = new AntTweakBar(window);

	Gizmos::create();

	//fbObject = new FBObject(camera, window);
	//texture = new Texture(camera);
	//texture->CreateShader();
	//texture->SetInputWindow(window);

	//navTechniques = new NavTechniques(camera);

	generateGrid(10, 10);

	procedualGen = new ProcedualGen(camera);

	//shadowMap = new ShadowMap(camera);

	//objLoader = new ObjectLoader(camera);

	//string crate = "./Content/OBJ/TestObject";
	//objLoader = new ObjectLoader(crate, Verts);
	//pObjectToRender = objLoader->getModel();

	//CreateShaders();

}

Application::~Application()
{
	Gizmos::destroy();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::RunGame()
{
	//projection = glm::perspective(glm::pi<float>() * 0.25f, 16/9.f, 0.1f, 1000.f);
	currentTime = 0;
	deltaTime = 0;
	previousTime = 0;
	
	camX = 10.0f;
	camY = 10.0f;
	camZ = 10.0f;
	rotation = 0.005f;
	camera->SetInputWindow(window);

	while (glfwWindowShouldClose(window) == false &&
		   glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)
	{
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
		glClearColor( antTweakBar->m_clearColour.r, antTweakBar->m_clearColour.g, antTweakBar->m_clearColour.b, antTweakBar->m_clearColour.a );
		glEnable(GL_DEPTH_TEST);
		Update();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Application::Update()
{
	// our game logic and update code goes here!
	// so does our render code!
	currentTime		= (float)glfwGetTime();
	deltaTime		= currentTime - previousTime; // prev of last frame
	previousTime	= currentTime;

	// use time to animate a alue between [0, 1]
	float s = glm::cos((float)glfwGetTime()) * 0.5f + 0.5f;
	// standard linear interpolation
	glm::vec3 p = (1.0f - s) * m_positions[0] + s * m_positions[1];
	// quaternion slerp
	glm::quat r = glm::slerp(m_rotations[0], m_rotations[1], s);
	// build a matrix
	glm::mat4 m = glm::translate(p) * glm::toMat4(r);
	// draw a transform and box
	Gizmos::addTransform(m);
	Gizmos::addAABBFilled(p, glm::vec3(.5f), glm::vec4(1,0,0,1), &m);

	// animate leg
	// linearly interpolate hip position
	glm::vec3 posHip = (1.0f - s) * m_hipFrames[0].position + s * m_hipFrames[1].position;
	// spherically interpolate hip rotation
	glm::quat rotHip = glm::slerp(m_hipFrames[0].rotation, m_hipFrames[1].rotation, s);
	// update the hip bone
	m_hipBone = glm::translate(posHip) * glm::toMat4(rotHip);

	// linearly interpolate hip position
	glm::vec3 posKnee = (1.0f - s) * m_kneeFrames[0].position + s * m_kneeFrames[1].position;
	// spherically interpolate hip rotation
	glm::quat rotKnee = glm::slerp(m_kneeFrames[0].rotation, m_kneeFrames[1].rotation, s);
	// update the hip bone
	m_kneeBone = glm::translate(posKnee) * glm::toMat4(rotKnee);
	m_kneeBone = m_hipBone * m_kneeBone;

	// linearly interpolate hip position
	glm::vec3 posAnk = (1.0f - s) * m_ankleFrames[0].position + s * m_ankleFrames[1].position;
	// spherically interpolate hip rotation
	glm::quat rotAnk = glm::slerp(m_ankleFrames[0].rotation, m_ankleFrames[1].rotation, s);
	// update the hip bone
	m_ankleBone = glm::translate(posAnk) * glm::toMat4(rotAnk);
	m_ankleBone = m_kneeBone * m_ankleBone;

	glm::vec3 hipPos = glm::vec3(m_hipBone[3].x, m_hipBone[3].y, m_hipBone[3].z);
	glm::vec3 kneePos = glm::vec3(m_kneeBone[3].x, m_kneeBone[3].y, m_kneeBone[3].z);
	glm::vec3 anklePos = glm::vec3(m_ankleBone[3].x, m_ankleBone[3].y, m_ankleBone[3].z);

	glm::vec3 half(0.5f);
	glm::vec4 pink(1, 0, 1, 1);

	Gizmos::addAABBFilled(hipPos, half, pink, &m_hipBone);
	Gizmos::addAABBFilled(kneePos, half, pink, &m_kneeBone);
	Gizmos::addAABBFilled(anklePos, half, pink, &m_ankleBone);

	view = camera->GetProjectionView();

	camera->Update(deltaTime);

	rotation += 0.005f;

	Gizmos::addTransform(glm::mat4(1));

	mat4 inWorldPosition = glm::inverse(view);

	//objLoader->SetLightDir(antTweakBar->m_light.x, antTweakBar->m_light.y, antTweakBar->m_light.z);
	//objLoader->SetLightColour(antTweakBar->m_lightColour.r, antTweakBar->m_lightColour.g, antTweakBar->m_lightColour.b);
	//objLoader->SetSpecPow(antTweakBar->m_specPow);

	vec4 white(1);
	vec4 black(0,0,0,1);

	//texture->Update(deltaTime);
	//fbObject->Update(deltaTime);
	//shadowMap->Update(deltaTime);
	//navTechniques->Update();
	Draw();
}
// sizeof(float), vertex_data.data()
void Application::generateGrid( unsigned int rows, unsigned int cols )
{
	Vertex* aoVertices = new Vertex[ rows * cols ];
	for ( unsigned int r = 0; r < rows; ++r )
	{
		for ( unsigned int c = 0; c < cols; ++c )
		{
			aoVertices[ r * cols + c ].position = vec4((float)c, 0, (float)r, 1);

			vec3 colour = vec3( sinf( (c / (float)(cols - 1)) *
								( r / (float)(rows - 1 ))) );
			aoVertices[ r * cols + c ].colour = vec4( colour, 1 );
		}
	}

	unsigned int* auiIndices = new unsigned int[ (rows - 1) * (cols - 1) * 6 ];

	unsigned int index = 0;
	for ( unsigned int r = 0; r < (rows - 1); ++r )
	{
		for ( unsigned int c = 0; c < (cols - 1); ++c )
		{
			auiIndices[ index++ ] = r * cols + c;
			auiIndices[ index++ ] = (r + 1) * cols + c;
			auiIndices[ index++ ] = (r + 1) * cols + (c + 1);

			auiIndices[ index++ ] = r * cols + c;
			auiIndices[ index++ ] = (r + 1) * cols + (c + 1);
			auiIndices[ index++ ] = r * cols + (c + 1);
		}
	}

	indexCount = index;

		// Create VAO, VBO and IBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &IBO);

	// Bind buffers
	glBindVertexArray( VAO );
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	// send data to the buffers	
	glBufferData(GL_ARRAY_BUFFER, (rows * cols) * sizeof(Vertex), aoVertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), auiIndices, GL_STATIC_DRAW);
	
	// describe how the vertices are setup so they can be sent to the shader
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec4)));

	// m_VAO hold all our ARRAY_BUFFER and ARRAY_ELEMENT_BUFFER settings
	// so just rebind it later before using glDrawElements
	glBindVertexArray(0);

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	delete[] auiIndices;
	delete[] aoVertices;
}

void Application::CreateShaders()
{
	//create shaders
	const char* vsSource = "#version 410\n \
						   layout(location=0) in vec4 Position; \
						   layout(location=1) in vec4 Colour; \
						   out vec4 vColour; \
						   uniform mat4 ProjectionView; \
						   void main() { vColour = Colour; gl_Position = ProjectionView * Position; }";
	
	const char* fsSource = "#version 410\n \
						   in vec4 vColour; \
						   out vec4 FragColor; \
						   void main() { FragColor = vColour; }";
	
	int success = GL_FALSE;
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	
	glShaderSource(vertexShader, 1, (const char**)&vsSource, 0);
	glCompileShader(vertexShader);
	glShaderSource(fragmentShader, 1, (const char**)&fsSource, 0);
	glCompileShader(fragmentShader);
	
	programID = glCreateProgram();
	glAttachShader(programID, vertexShader);
	glAttachShader(programID, fragmentShader);
	glLinkProgram(programID);
	
	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) 
	{
		int infoLogLength = 0;
		glGetShaderiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength];
	
		glGetShaderInfoLog(programID, infoLogLength, 0, infoLog);
		printf("Error: Failed to link shader program!\n");
		printf("%s\n", infoLog);
	
		delete[] infoLog;
	}
	
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
}

void Application::Draw()
{
	//fbObject->Draw();
	//texture->Draw();
	//shadowMap->Draw();
	//navTechniques->Draw();
	procedualGen->Draw();
	Gizmos::draw(projection * view);
	Gizmos::clear();
	antTweakBar->Draw();
	//objLoader->Draw();
}


