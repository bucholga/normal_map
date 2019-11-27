// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
//#include <glad/glad.h>
// GLFW
#include <glfw3.h>
#include <stb_image.h>

// GL includes
#include <shader.hpp>
#include <camera.hpp>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <vector>
#include <algorithm>
// Other Libs
#include <src/SOIL.h>
#include <stb_image.h>

#define M_PI 3.14159265
#define COUNT_LAYERS 20

struct hm_struct {
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	float* vert;
	int n;
	hm_struct(int n) : n(n) {
		vert = new float[30 * n * n];
		float dx = 1.f / (n);
		auto print_to_verts = [&](int i, int j, int& num) {
			vert[5 * num + 0] = i * dx;
			vert[5 * num + 1] = j * dx;
			vert[5 * num + 2] = 0.f;
			vert[5 * num + 3] = i * dx;
			vert[5 * num + 4] = j * dx;
			++num;
		};
		int num = 0;
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
			{
				print_to_verts(i, j, num);
				print_to_verts(i + 1, j, num);
				print_to_verts(i + 1, j + 1, num);

				print_to_verts(i, j, num);
				print_to_verts(i, j + 1, num);
				print_to_verts(i + 1, j + 1, num);
			}
	}
	~hm_struct() {
		delete [] vert;
	}
};

struct vertex {
	glm::vec3 pos;
	glm::vec2 text_coord;
};

void make_sphere(std::vector<vertex>& points, int n) {
	float dx = 1.f / float(n);
	for (int i = 0; i <= n; ++i)
		for (int j = 0; j <= n; ++j) {
			float angle1 = 2 * M_PI * i * dx;
			float angle2 =  M_PI * j * dx;
			vertex v;
			v.pos = glm::vec3(
				sin(angle2) * cos(angle1),
				cos(angle2),
				sin(angle2) * sin(angle1));
			v.text_coord = glm::vec2(i * dx, j * dx);
			points.push_back(v);
		}
}

struct sphere {
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	float* vert;
	int n;
	sphere(int nn) : n(nn) {
		std::vector<vertex> vs;
		make_sphere(vs, n);
		vert = new float[30 * n * n];
		auto print_to_verts = [&](int i, int j, int& num) {
			int nvert = i * (n + 1) + j;
			vert[5 * num + 0] = vs[nvert].pos[0];
			vert[5 * num + 1] = vs[nvert].pos[1];
			vert[5 * num + 2] = vs[nvert].pos[2];
			vert[5 * num + 3] = vs[nvert].text_coord[0];
			vert[5 * num + 4] = vs[nvert].text_coord[1];
			++num;
		};
		int num = 0;
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
			{
				print_to_verts(i, j, num);
				print_to_verts(i + 1, j, num);
				print_to_verts(i + 1, j + 1, num);

				print_to_verts(i, j, num);
				print_to_verts(i, j + 1, num);
				print_to_verts(i + 1, j + 1, num);
			}
	}
};


void Text_init(unsigned int& VAO, unsigned int& EBO, unsigned int& VBO, GLfloat* vert, int n);
void RenderScene_floor(Shader& shader);
void RenderStCube();
unsigned int Text_init_bil(unsigned int& VAO, unsigned int& VBO, float* vert, int n);
unsigned int Text_init_hm(unsigned int& VAO, unsigned int& EBO, unsigned int& VBO, GLfloat* vert, int n);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void Text_init(unsigned int& VAO, unsigned int& VBO, float* vert, int n);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void renderQuad();
void RenderCube();
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
GLuint planeVAO;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
#if COUNT_LAYERS < 2
#undef COUNT_LAYERS
#define COUNT_LAYERS 2
#endif
	hm_struct hm(30);
	sphere s(400);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Buchatskaya Olga 301", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	// build and compile shaders
	// -------------------------
	Shader shader("normal_mapping.vs", "normal_mapping.fs");
	Shader lampShader("lamp.vs", "lamp.fs");
	Shader mapShader("map.vs", "map.fs");
	Shader hmShader("hm.vs", "hm.fs");
	Shader furShader("furShader.vs", "furShader.fs");
	Shader simpleShader("sim.vs", "sim.fs");
	Shader colorShader("sim.vs", "color.fs");
	Shader fireShader("fire.vs", "fire.fs");
	Shader trasp("blending.vs", "blending.fs");
	

	GLfloat planeVertices[] = {
		// Positions            // Normals           // Texture Coords
		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
		1.0f,  0.5f,  0.0f,  1.0f,  0.0f
	};
	// load textures
	// -------------
	unsigned int diffuseMap = loadTexture("textures/brickwall.jpg");
	unsigned int normalMap = loadTexture("textures/brickwall_normal.jpg");
	unsigned int transparentTexture = loadTexture("textures/window.png");
	unsigned int heightMap = loadTexture("textures/bot.png");
	unsigned int furTexture = loadTexture("textures/fur.png");
	unsigned int furlastTexture = loadTexture("textures/fur_last.png");
	unsigned int stenTexture = loadTexture("textures/bot.png");
	unsigned int picture = loadTexture("textures/bottom.jpg");
	unsigned int snow = loadTexture("textures/snow.jpg");



	unsigned int planeVBO;
	Text_init_bil(planeVAO, planeVBO, planeVertices, sizeof(planeVertices));
	Text_init_hm(hm.VAO, hm.EBO, hm.VBO, hm.vert, hm.n);
	Text_init(s.VAO, s.EBO, s.VBO, s.vert, s.n);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	float firevert[] = {
		0.0f,  0.5f,  0.0f,  1.0f,  
		0.0f, -0.5f,  0.0f,  1.0f, 
		1.0f, -0.5f,  0.0f,  1.0f, 

		0.0f,  0.5f,  0.0f,  1.0f, 
		1.0f, -0.5f,  0.0f,  1.0f,  
		1.0f,  0.5f,  0.0f,  1.0f, 
		//0.7f , 0.7f,  0.0f,  1.0f,
	};
	unsigned int fireVAO, fireVBO;
	glGenVertexArrays(1, &fireVAO);
	glGenBuffers(1, &fireVBO);
	glBindVertexArray(fireVAO);
	glBindBuffer(GL_ARRAY_BUFFER, fireVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(firevert), &firevert, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);



	float cubeVertices[] = {
		// positions          // texture Coords
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);

	// Cubemap (Skybox)
	std::vector<std::string> faces;
	faces.push_back("skybox/merc/front.jpg");
	//faces.push_back("skybox/merc/right.jpg");
	faces.push_back("skybox/merc/back.jpg");
	//faces.push_back("skybox/merc/left.jpg");
	faces.push_back("skybox/merc/top.jpg");
	faces.push_back("skybox/merc/bottom.jpg");
	faces.push_back("skybox/merc/right.jpg");
	//faces.push_back("skybox/merc/front.jpg");
	//faces.push_back("skybox/merc/back.jpg");
	faces.push_back("skybox/merc/left.jpg");
	GLuint skyboxTexture = loadCubemap(faces);
	// shader configuration
	// --------------------
	
	shader.Use();
	shader.setInt("diffuseMap", 0);
	shader.setInt("normalMap", 1);

	hmShader.Use();
	hmShader.setInt("ourTexture", 0);
	hmShader.setInt("picture", 1);

	furShader.Use();
	furShader.setInt("furTexture", 0);

	simpleShader.Use();
	simpleShader.setInt("texture1", 0);

	fireShader.Use();
	fireShader.setInt("sprite", 0);
	fireShader.setInt("colorMap", 1);


	// lighting info
	// -------------
	glm::vec3 lightPos1(0.5f, 1.0f, 0.3f);
	glm::vec3 lightPos(0.5f, 0.5f, 0.3f);
	//glm::vec3 lightPos(0.5f, 0.5f, 0.3f);
    //glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
	// render loop
	// -----------
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		//glEnable(GL_BLEND);
		glStencilMask(0x00);




		// configure view/projection matrices
		shader.Use();
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::translate(model,glm::vec3(0.0f, 0.0f, 0.0));
		
		
		//glClear(GL_STENCIL_BUFFER_BIT);
		glDisable(GL_STENCIL_TEST);

		//======================Карта высот=================================
		hmShader.Use();
		model = glm::mat4(1.f);
	
		model = glm::translate(glm::mat4(1.f), glm::vec3(-50.f, -5.f, -50.f));
		model = glm::scale(model, glm::vec3(100.0f, -100.f, 100.f));
		model = glm::rotate(model, 90.0f, glm::normalize(glm::vec3(1.0, 0.0, 0.0)));
		hmShader.setMat4("model", model);
		hmShader.setMat4("projection", projection);
		hmShader.setMat4("view", view);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, picture);
		glBindVertexArray(hm.VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6 * hm.n * hm.n);
		glBindVertexArray(0);



		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_BLEND);

		//=================================Normal Mapping==================================

		shader.Use();
		projection = glm::perspective(camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0));
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		// render normal-mapped quad
		glClear(GL_STENCIL_BUFFER_BIT);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0));
		model = glm::scale(model, glm::vec3(1.f));
		shader.setMat4("model", model);
		shader.setVec3("viewPos", camera.Position);
		shader.setVec3("lightPos", lightPos);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMap);
		renderQuad();




		//=================================Меховая сфера========================
		for (int i = COUNT_LAYERS - 1; i >= 0; --i) {
			furShader.Use();
			furShader.setMat4("projection", projection);
			furShader.setMat4("view", view);
			float mn = 0.9f;
			model = glm::mat4(100.0f);
			float sc_par = 1.0 - float(i) * (1 - mn) / float(COUNT_LAYERS - 1);
			if ((i + 1) == COUNT_LAYERS) {
				sc_par = 0.7f;
			}
			model = glm::translate(model, glm::vec3(2.f, 0.6f, 0.f));
			model = glm::scale(model, glm::vec3(sc_par * 0.5));
			//model = glm::rotate(model, sin((5.f + i) * currentFrame), glm::vec3(0.f, 0.f, 1.f));
			furShader.setMat4("model", model);
			float begin = 0.3;
			float alpha = begin + float(i + 1) * (1.0 - begin) / COUNT_LAYERS;
			if ((i + 1) == COUNT_LAYERS) {
				alpha = 1.f;
			}
			furShader.setFloat("alpha", alpha);
			glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if (i == COUNT_LAYERS - 1)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, furlastTexture);
				glBindVertexArray(s.VAO);
				glDrawArrays(GL_TRIANGLES, 0, 6 * s.n * s.n);
				glBindVertexArray(0);
			}
			else
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, furTexture);
				glBindVertexArray(s.VAO);
				glDrawArrays(GL_TRIANGLES, 0, 6 * s.n * s.n);
				glBindVertexArray(0);
			}
			
		}
		

		projection = glm::perspective(camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		lampShader.Use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.1f)); // a smaller cube
		lampShader.setMat4("model", model);
		RenderCube();



		//=============================Обводка=============================
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		model = glm::mat4(1.0f);
		view = camera.GetViewMatrix();
		projection = glm::perspective(camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		simpleShader.Use();
		simpleShader.setMat4("view", view);
		simpleShader.setMat4("projection", projection);
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, stenTexture);
		model = glm::translate(model, glm::vec3(5.0f, 0.5f, 2.0f));
		simpleShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);
		colorShader.Use();
		colorShader.setMat4("view", view);
		colorShader.setMat4("projection", projection);
		colorShader.Use();
		// cubes
		glBindVertexArray(cubeVAO);
		glBindTexture(GL_TEXTURE_2D, stenTexture);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(5.0f, 0.5f, 2.0f));
		model = glm::scale(model, glm::vec3(1.05f));
		colorShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		

		// cubes
		

		

		//==============================Кубик для зеркала==============


		glEnable(GL_DEPTH_TEST);
		float mirror = 0;
		simpleShader.Use();
		simpleShader.setMat4("view", view);
		simpleShader.setMat4("projection", projection);
		simpleShader.setFloat("mirror", mirror);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		// cubes
		model = glm::mat4(1.0);
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, snow);
		model = glm::translate(model, glm::vec3(-10.0f, 0.5f, 2.0f));
		simpleShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		//===========================Зеркало==========================

		glDepthFunc(GL_LESS);
		glEnable(GL_STENCIL_TEST);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glDisable(GL_DEPTH_TEST);
		


		model = glm::mat4(1.0);
		trasp.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, transparentTexture);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		projection = glm::perspective(camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		view = camera.GetViewMatrix();
		//model = glm::scale(model, glm::vec3(100.0f));
		model = glm::translate(model, glm::vec3(-9.5f, 0.0f, 2.0));
		model = glm::rotate(model, 90.0f, glm::normalize(glm::vec3(1.0, 0.0, 0.0)));
		trasp.setMat4("projection", projection);
		trasp.setMat4("view", view);
		glUniformMatrix4fv(glGetUniformLocation(trasp.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);


		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glStencilFunc(GL_EQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glEnable(GL_DEPTH_TEST);


		camera.Front.y = -camera.Front.y;
		camera.Position.y = -camera.Position.y;
		camera.Up.y = -camera.Up.y;

		auto sym_matrix = glm::scale(glm::mat4(1.f), glm::vec3(1.f, -1.f, 1.f));
		glEnable(GL_STENCIL_TEST);
	
		simpleShader.Use();
		simpleShader.setMat4("view", view);
		simpleShader.setMat4("projection", projection);
		mirror = 1;
		simpleShader.setFloat("mirror", mirror);

		// cubes
		model = glm::mat4(1.0);
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, snow);
		model = glm::translate(model, glm::vec3(-10.0f, 0.5f, 2.0f));
		simpleShader.setMat4("model",sym_matrix * model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		RenderCube();

		camera.Front.y = -camera.Front.y;
		camera.Position.y = -camera.Position.y;
		camera.Up.y = -camera.Up.y;
	


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

// renders a 1x1 quad in NDC with manually calculated tangent vectors
// ------------------------------------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		// positions
		glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
		glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
		glm::vec3 pos3(1.0f, -1.0f, 0.0f);
		glm::vec3 pos4(1.0f, 1.0f, 0.0f);
		// texture coordinates
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);
		glm::vec2 uv4(1.0f, 1.0f);
		// normal vector
		glm::vec3 nm(0.0f, 0.0f, 1.0f);

		// calculate tangent/bitangent vectors of both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;
		// triangle 1
		// ----------
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent1 = glm::normalize(tangent1);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent1 = glm::normalize(bitangent1);

		// triangle 2
		// ----------
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent2 = glm::normalize(tangent2);


		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent2 = glm::normalize(bitangent2);


		float quadVertices[] = {
			// positions            // normal         // texcoords  // tangent                          // bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
		};
		// configure plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void RenderScene_floor(Shader& shader )
{
	glm::mat4 model;
	glm::mat4 projection = glm::perspective(camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
bool keys[1024];
bool keysPressed[1024];
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key <= 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
			keysPressed[key] = false;
		}
	}
}
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;

void RenderStCube()
{
	if (cubeVAO == 0)
	{
		GLfloat vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
		};
		unsigned int cubeVAO, cubeVBO;
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glBindVertexArray(0);
	}
	// Render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}


void RenderCube()
{
	// Initialize (if necessary)
	if (cubeVAO == 0)
	{
		GLfloat vertices[] = {
			// Back face
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right

			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,  // top-right
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,// top-left
															  // Front face
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // top-right

			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
			// Left face
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
			-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-left

			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
			-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
			// Right face
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right

			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // bottom-right
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // top-left
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
			// Bottom face
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,// bottom-left

			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			// Top face
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right

			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,// top-left
			-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// Fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// Link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// Render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}
void Text_init(unsigned int& VAO, unsigned int& VBO, float* vert, int n)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, n, vert, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
}
unsigned int Text_init_bil(unsigned int& VAO, unsigned int& VBO, float* vert, int n)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, n, vert, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
	return VAO;
}
unsigned int Text_init_hm(unsigned int& VAO, unsigned int& EBO, unsigned int& VBO, GLfloat* vert, int n) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// Fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert[0]) * n * n * 30, vert, GL_STATIC_DRAW);
	// Link vertex attributes
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return VAO;
}

void Text_init(unsigned int& VAO, unsigned int& EBO, unsigned int& VBO, GLfloat* vert, int n) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// Fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert[0]) * n * n * 30, vert, GL_STATIC_DRAW);
	// Link vertex attributes
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}