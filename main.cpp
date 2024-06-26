#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include <LearnOpenGL/shader_s.h>
#include <LearnOpenGL/camera.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

#include "sphere.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double XposIn, double YposIn);
void prossessInput(GLFWwindow* window);
void cameraLock_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void loadTexture(unsigned* texture, const char* texture_file_name, GLint internalformat);

// config
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// camera
// Camera camera(glm::vec3(-6.03f, -.83f, -13.16f));
Camera camera(glm::vec3(0.f, 0.f, -2.f));
bool firstMouse = true;
float lastXpos = SCR_WIDTH / 2.f;
float lastYpos = SCR_HEIGHT / 2.f;

// timing
float deltaTime = 0.f;
float lastTime = 0.f;

// pos
glm::vec3 lightPos(0.f, 0.f, 0.f);
glm::vec3 objPos(0.f, 0.f, 0.f);

int main() {
	// initialize GLFW window and openGL context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL2", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	// initialize Viewport and call the callback fun
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, cameraLock_key_callback);

	// ��ʼ��Dear ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");
	ImGui::StyleColorsDark();

	// render
	{
		glEnable(GL_DEPTH_TEST);

		/*----------BUFFER------------*/

		// ball
		int ballVertNum = 6 * 3 * 30 * 60;
		static float ballVertices[6 * 3 * 30 * 60];
		createSphere(ballVertices);

		unsigned int VAO, lightVAO, lightVBO, VBO;
		glGenVertexArrays(1, &VAO);
		glGenVertexArrays(1, &lightVAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &lightVBO);

		glBindVertexArray(lightVAO);	// light
		glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ballVertices), ballVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// obj
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		/*----------TEXTURE------------*/
		// diffuse
		unsigned int texture_diffuse;
		loadTexture(&texture_diffuse, "container2.png", GL_RGBA);

		// specular
		unsigned int texture_specular;
		loadTexture(&texture_specular, "container2_specular.png", GL_RGBA);


		/*----------SHADER------------*/
		Shader shader("3.3.shader.vert", "3.3.shader.frag");
		Shader lightShader("3.3.lightShader.vert", "3.3.lightShader.frag");

		// Material
		glm::vec3 light_ambient(.05f, .05f, .05f);
		glm::vec3 light_diffuse(.3f, .3f, .3f);
		glm::vec3 light_specular(1.f, 1.f, 1.f);
		glm::vec3 light_direction(0.f, -1.f, 0.f);
		// DirLight(Sun)
		glm::vec3 dirlight_ambient(.05f, .038f, .021f);
		glm::vec3 dirlight_diffuse(.5f, .38f, .21f);
		glm::vec3 dirlight_specular(1.f, .77f, .42f);

		float dirLight_phi = glm::pi<float>();
		float linear = 0.09f;
		float quadratic = 0.032f;
		float innerCone_phi = 12.5f;
		float outerCone_phi = 17.5f;
		float material_shininess = 50.f;
		

		/*----------RENDER LOOP------------*/ 
		camera.front = glm::vec3(.68f, -.10f, .73f);
		camera.Lock();
		float light_lock = false;

		while (!glfwWindowShouldClose(window)) {
			//timing
			{
				float curTime = glfwGetTime();
				deltaTime = curTime - lastTime;
				lastTime = curTime;
			}

			// clear last frame
			{
				glClearColor(0.f, 0.f, 0.f, 1.f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				prossessInput(window);
			}

			// transform
			glm::mat4 projection = glm::mat4(1.f);
			projection = glm::perspective(glm::radians(45.f), (float)SCR_WIDTH / SCR_HEIGHT, .1f, 100.f);
			glm::mat4 view = camera.getViewMatrix(); // view

			// imgui
			{
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				ImGui::Begin("Menu");
				glm::vec3* input_value[] = { &light_ambient, &light_diffuse, &light_specular };
				const char* input_value_name[] = { "light.ambient", "light.diffuse", "light.specular",};
				float vec3f[3];
				for (int i = 0; i < 3; i++) {
					vec3f[0] = input_value[i]->x, vec3f[1] = input_value[i]->y, vec3f[2] = input_value[i]->z;
					ImGui::InputFloat3(input_value_name[i], vec3f);
					input_value[i]->x = vec3f[0], input_value[i]->y = vec3f[1], input_value[i]->z = vec3f[2];
					ImGui::Separator();
				}
				ImGui::SliderFloat("dirLight_pos", &dirLight_phi, 0.f, 2 * glm::pi<float>());
				ImGui::Separator();
				ImGui::SliderFloat("material_shininess", &material_shininess, 1.f, 128.f);
				ImGui::Separator();
				ImGui::SliderFloat("pointLight.linear", &linear, 0.f, 1.f);
				ImGui::Separator();
				ImGui::SliderFloat("pointLight.quadratic", &quadratic, 0.f, 1.f);
				ImGui::Separator();
				ImGui::SliderFloat("light.innerCone_phi", &innerCone_phi, 0.f, 90.f);
				ImGui::Separator();
				ImGui::SliderFloat("light.outerCone_phi", &outerCone_phi, 0.f, 90.f);
				ImGui::Separator();
				ImGui::Text("camera.position: %.2f %.2f %.2f", camera.position.x, camera.position.y, camera.position.z);
				ImGui::Separator();
				ImGui::Text("camera.front: %.2f %.2f %.2f", camera.front.x, camera.front.y, camera.front.z);
				ImGui::Separator();

				ImGui::End();
				ImGui::Render();
				int display_w, display_h;
				glfwGetFramebufferSize(window, &display_w, &display_h);
				glViewport(0, 0, display_w, display_h);
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			}

			// uniform
			{
				shader.use();
				shader.setMat4f("projection", 1, glm::value_ptr(projection));
				shader.setMat4f("view", 1, glm::value_ptr(view));
				shader.setVec3f("viewPos", camera.position);
				// Dirlight
				shader.setVec3f("dirLight.direction", sin(dirLight_phi), light_direction.y, cos(dirLight_phi));
				shader.setVec3f("dirLight.ambient", dirlight_ambient);
				shader.setVec3f("dirLight.diffuse", dirlight_diffuse);
				shader.setVec3f("dirLight.specular", dirlight_specular);
				// Spotlight
				shader.setVec3f("spotLight.position", camera.position);
				shader.setVec3f("spotLight.ambient", light_ambient);
				shader.setVec3f("spotLight.diffuse", light_diffuse);
				shader.setVec3f("spotLight.specular", light_specular);
				shader.setVec3f("spotLight.direction", camera.front);
				shader.setFloat("spotLight.cutOff", cos(glm::radians(innerCone_phi)));
				shader.setFloat("spotLight.outerCutOff", cos(glm::radians(outerCone_phi)));

				shader.setInt("material.diffuse", 0);
				shader.setInt("material.specular", 1);
				shader.setFloat("material.shininess", material_shininess);
				// PointLight
				for (int i = 0; i < 4; i++) {
					std::string value_name = "pointLight[x].";
					value_name[11] = i + '0';
					shader.setVec3f((value_name + "position").c_str(), pointLightPositions[i]);
					shader.setVec3f((value_name + "ambient").c_str(), light_ambient);
					shader.setVec3f((value_name + "diffuse").c_str(), light_diffuse);
					shader.setVec3f((value_name + "specular").c_str(), light_specular);
					shader.setFloat((value_name + "constant").c_str(), 1.f);
					shader.setFloat((value_name + "linear").c_str(), linear);
					shader.setFloat((value_name + "quadratic").c_str(), quadratic);
				}
			}

			// draw
			{
				shader.use();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture_diffuse);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, texture_specular);
				glBindVertexArray(VAO);

				// cube party
				for (int i = 0; i < 10; i++) {
					glm::mat4 model_obj = glm::mat4(1.f);
					model_obj = glm::translate(model_obj, cubePositions[i]);
					model_obj = glm::rotate(model_obj, glm::radians(i * 20.f), glm::vec3(1.f));
					shader.setMat4f("model", 1, glm::value_ptr(model_obj));
					glm::mat4 nrmMat = glm::mat4(1.f);
					nrmMat = glm::transpose(glm::inverse(model_obj));
					shader.setMat4f("nrmMat", 1, glm::value_ptr(nrmMat));
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}

				// 4 * PointLight
				for (int i = 0; i < 4; i++) {
					lightShader.use();
					glm::mat4 model_light = glm::mat4(1.f);
					model_light = glm::translate(model_light, pointLightPositions[i]);
					model_light = glm::scale(model_light, glm::vec3(.1f));
					lightShader.setVec3f("lightColor", glm::vec3(1.f));
					lightShader.setMat4f("model", 1, glm::value_ptr(model_light));
					lightShader.setMat4f("projection", 1, glm::value_ptr(projection));
					lightShader.setMat4f("view", 1, glm::value_ptr(view));
					glBindVertexArray(lightVAO);
					glDrawArrays(GL_TRIANGLES, 0, ballVertNum);
				}
				// DirLight
				glm::mat4 model_light = glm::mat4(1.f);
				model_light = glm::translate(model_light, glm::vec3(-sin(dirLight_phi) * 15.f, 20.f, -cos(dirLight_phi) * 25));
				model_light = glm::scale(model_light, glm::vec3(4.f));
				lightShader.setVec3f("lightColor", glm::vec3(1.f, .77f, .42f));
				lightShader.setMat4f("model", 1, glm::value_ptr(model_light));
				lightShader.setMat4f("projection", 1, glm::value_ptr(projection));
				lightShader.setMat4f("view", 1, glm::value_ptr(view));
				glBindVertexArray(lightVAO);
				glDrawArrays(GL_TRIANGLES, 0, ballVertNum);
			}

			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		// delete
		glDeleteVertexArrays(1, &VAO);
		glDeleteVertexArrays(1, &lightVAO);
		glDeleteBuffers(1, &VBO);
		glDeleteProgram(lightShader.ID);
		glDeleteProgram(shader.ID);

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		glfwTerminate();
	}
	return 0;
}

void loadTexture(unsigned* texture, const char* texture_file_name, GLint internalformat) {
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nrChannels;
	unsigned char* data = stbi_load(texture_file_name, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, internalformat, GL_UNSIGNED_BYTE, data);
	}
	else {
		std::cout << "Failed to load texture " << texture_file_name << std::endl;
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double XposIn, double YposIn)
{
	float xpos = static_cast<float>(XposIn);
	float ypos = static_cast<float>(YposIn);

	if (firstMouse) {
		lastXpos = xpos;
		lastYpos = ypos;
		firstMouse = false;
	}

	float Xoffset = xpos - lastXpos;
	float Yoffset = -1 * (ypos - lastYpos);
	lastXpos = xpos;
	lastYpos = ypos;

	camera.ProcessMouseMovement(Xoffset, Yoffset);
}

void prossessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	// camera
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

void cameraLock_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		camera.Lock();
	}
	if (key == GLFW_KEY_U && action == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}
