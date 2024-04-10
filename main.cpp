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

// config
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// camera
Camera camera(glm::vec3(.7f, .7f, .7f));
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

	// ³õÊ¼»¯Dear ImGui
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
		unsigned int VAO, lightVAO, VBO;
		glGenVertexArrays(1, &VAO);
		glGenVertexArrays(1, &lightVAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(lightVAO);	// light
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// obj
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		/*----------TEXTURE------------*/
		unsigned int cubeTexture;
		glGenTextures(1, &cubeTexture);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		int width, height, nrChannels;
		unsigned char* data = stbi_load("container2.png", &width, &height, &nrChannels, 0);
		if (data) { 
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else {
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data); 
		glActiveTexture(GL_TEXTURE0);

		/*----------SHADER------------*/
		Shader shader("3.3.shader.vert", "3.3.shader.frag");
		Shader lightShader("3.3.shader.vert", "3.3.lightShader.frag");

		// Material
		//glm::vec3 light_ambient(.2f, .2f, .2f);
		//glm::vec3 light_diffuse(.5f, .5f, .5f);
		//glm::vec3 light_specular(1.f, 1.f, 1.f);
		glm::vec3 light_ambient(1.f);
		glm::vec3 light_diffuse(1.f);
		glm::vec3 light_specular(1.f);
		//glm::vec3 material_ambient(.2f, .2f, .2f);
		//glm::vec3 material_diffuse(.6f, .6f, .6f);
		//glm::vec3 material_specular(.2f, .2f, .2f);
		glm::vec3 material_ambient(0.24725,	0.1995,	0.0745);
		glm::vec3 material_diffuse(0.75164,	0.60648,0.22648);
		glm::vec3 material_specular(0.628281,0.555802,0.366065);
		float material_shininess = 25.f;
		

		/*----------RENDER LOOP------------*/ 
		camera.front = glm::vec3(-.65f, -.4f, -.65f);
		camera.Lock();
		float light_lock = false;

		while (!glfwWindowShouldClose(window)) {
			//timing
			float curTime = glfwGetTime();
			deltaTime = curTime - lastTime;
			lastTime = curTime;

			// clear last frame
			glClearColor(0.1f, 0.1f, 0.1f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			prossessInput(window);

			// transform
			glm::mat4 model_light = glm::mat4(1.f);
			if(light_lock){
				lightPos.x = sin(curTime) * 1.4f;
				lightPos.y = cos(curTime) * 1.4f;
				lightPos.z = sin(curTime) * 1.4f;
			}
			model_light = glm::translate(model_light, lightPos);
			model_light = glm::scale(model_light, glm::vec3(.2f));
			glm::mat4 model_obj = glm::mat4(1.f);
			model_obj = glm::translate(model_obj, objPos);
			model_obj = glm::scale(model_obj, glm::vec3(.6f));
			//model_obj = glm::rotate(model_obj, (float)curTime, glm::vec3(0.f, 0.f, 1.f));
			glm::mat4 projection = glm::mat4(1.f);
			projection = glm::perspective(glm::radians(45.f), (float)SCR_WIDTH / SCR_HEIGHT, .1f, 100.f);
			glm::mat4 nrmMat = glm::mat4(1.f);
			nrmMat = glm::transpose(glm::inverse(model_obj));
			glm::mat4 view = camera.getViewMatrix(); // view

			// imgui
			{
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				ImGui::Begin("Menu");
				glm::vec3* input_value[] = {&lightPos, &light_ambient, &light_diffuse, &light_specular, &material_ambient, &material_diffuse, &material_specular};
				const char* input_value_name[] = { "light.position", "light.ambient", "light.diffuse", "light.specular", "material.ambient", "material.diffuse", "material.specular" };
				float vec3f[3];
				glm::vec3 temp(0.f);
				
				for (int i = 0; i < 7; i++) {
					vec3f[0] = input_value[i]->x, vec3f[1] = input_value[i]->y, vec3f[2] = input_value[i]->z;
					ImGui::InputFloat3(input_value_name[i], vec3f);
					input_value[i]->x = vec3f[0], input_value[i]->y = vec3f[1], input_value[i]->z = vec3f[2];
					ImGui::Separator();
				}
				ImGui::SliderFloat("material_shininess", &material_shininess, 1.f, 256.f);
				ImGui::Separator();
				ImGui::Text("camera.position: %.2f %.2f %.2f", camera.position.x, camera.position.y, camera.position.z);
				ImGui::Separator();
				ImGui::Text("camera.front: %.2f %.2f %.2f", camera.front.x, camera.front.y, camera.front.z);
				ImGui::Separator();
				if (ImGui::Button("light.lock")) {
					light_lock = !light_lock;
				}

				ImGui::End();
				ImGui::Render();
				int display_w, display_h;
				glfwGetFramebufferSize(window, &display_w, &display_h);
				glViewport(0, 0, display_w, display_h);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			}

			// uniform
			shader.use();
			shader.setMat4f("model", 1, glm::value_ptr(model_obj));
			shader.setMat4f("projection", 1, glm::value_ptr(projection));
			shader.setMat4f("nrmMat", 1, glm::value_ptr(nrmMat));
			shader.setVec3f("light.position", lightPos);
			shader.setVec3f("light.ambient", light_ambient);
			shader.setVec3f("light.diffuse", light_diffuse);
			shader.setVec3f("light.specular", light_specular);
			shader.setVec3f("material.ambient", material_ambient);
			shader.setVec3f("material.diffuse", material_diffuse);
			shader.setVec3f("material.specular", material_specular);
			shader.setFloat("material.shininess", material_shininess);
			lightShader.use();
			lightShader.setMat4f("model", 1, glm::value_ptr(model_light));
			lightShader.setMat4f("projection", 1, glm::value_ptr(projection));

			// draw
			shader.use();
			shader.setMat4f("view", 1, glm::value_ptr(view));
			shader.setVec3f("viewPos", camera.position);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			lightShader.use();
			lightShader.setMat4f("view", 1, glm::value_ptr(view));
			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);

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
