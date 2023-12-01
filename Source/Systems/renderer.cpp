#include <random>
#include <iostream>
#include "renderer.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Physics.h"
#include "../Events/Levelevents.h"
#include "../Entities/Prefabs.h"
#define TEXTURES_PATH "../textures/"
#define XML_PATH "../xml/"

#define STB_IMAGE_IMPLEMENTATION
#include "../../stb/stb_image.h"
#include "../../tinyxml2/tinyxml2.h"
#include "../Utils/OpenGLExtensions.h"


class Shader
{
public:
	unsigned int ID;
	// constructor generates the shader on the fly
	// ------------------------------------------------------------------------
	Shader(const char* vertexPath = nullptr, const char* fragmentPath = nullptr, const char* geometryPath = nullptr)
	{
		if (vertexPath != nullptr && fragmentPath != nullptr)
		{

			// 1. retrieve the vertex/fragment source code from filePath
			std::string vertexCode;
			std::string fragmentCode;
			std::string geometryCode;
			std::ifstream vShaderFile;
			std::ifstream fShaderFile;
			std::ifstream gShaderFile;
			// ensure ifstream objects can throw exceptions:
			vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			try
			{
				// open files
				vShaderFile.open(vertexPath);
				fShaderFile.open(fragmentPath);
				std::stringstream vShaderStream, fShaderStream;
				// read file's buffer contents into streams
				vShaderStream << vShaderFile.rdbuf();
				fShaderStream << fShaderFile.rdbuf();
				// close file handlers
				vShaderFile.close();
				fShaderFile.close();
				// convert stream into string
				vertexCode = vShaderStream.str();
				fragmentCode = fShaderStream.str();
				// if geometry shader path is present, also load a geometry shader
				if (geometryPath != nullptr)
				{
					gShaderFile.open(geometryPath);
					std::stringstream gShaderStream;
					gShaderStream << gShaderFile.rdbuf();
					gShaderFile.close();
					geometryCode = gShaderStream.str();
				}
			}
			catch (std::ifstream::failure& e)
			{
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
			}
			const char* vShaderCode = vertexCode.c_str();
			const char* fShaderCode = fragmentCode.c_str();
			// 2. compile shaders
			unsigned int vertex, fragment;
			// vertex shader
			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, NULL);
			glCompileShader(vertex);
			checkCompileErrors(vertex, "VERTEX");
			// fragment Shader
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fShaderCode, NULL);
			glCompileShader(fragment);
			checkCompileErrors(fragment, "FRAGMENT");
			// if geometry shader is given, compile geometry shader
			unsigned int geometry;
			if (geometryPath != nullptr)
			{
				const char* gShaderCode = geometryCode.c_str();
				geometry = glCreateShader(GL_GEOMETRY_SHADER);
				glShaderSource(geometry, 1, &gShaderCode, NULL);
				glCompileShader(geometry);
				checkCompileErrors(geometry, "GEOMETRY");
			}
			// shader Program
			ID = glCreateProgram();
			glAttachShader(ID, vertex);
			glAttachShader(ID, fragment);
			if (geometryPath != nullptr)
				glAttachShader(ID, geometry);
			glLinkProgram(ID);
			checkCompileErrors(ID, "PROGRAM");
			// delete the shaders as they're linked into our program now and no longer necessary
			glDeleteShader(vertex);
			glDeleteShader(fragment);
			if (geometryPath != nullptr)
				glDeleteShader(geometry);
		}

	}
	// activate the shader
	// ------------------------------------------------------------------------
	void use()
	{
		glUseProgram(ID);
	}
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	// ------------------------------------------------------------------------
	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setVec2(const std::string& name, const GW::MATH2D::GVECTOR2F& value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, (float*)&value);
	}
	void setVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3(const std::string& name, const GW::MATH2D::GVECTOR3F& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, (float*)&value);
	}
	void setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4(const std::string& name, const GW::MATH::GVECTORF& value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, (float*)&value);
	}
	void setVec4(const std::string& name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2(const std::string& name, const GW::MATH2D::GMATRIX2D& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, (float*)&mat);
	}
	// ------------------------------------------------------------------------
	void setMat3(const std::string& name, const GW::MATH2D::GMATRIX3F& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, (float*)&mat);
	}
	// ------------------------------------------------------------------------
	void setMat4(const std::string& name, const GW::MATH::GMATRIXF& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, (float*)&mat);
	}

private:
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};




void PrintLabeledDebugString(const char* label, const char* toPrint)
{
std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
OutputDebugStringA(label);
OutputDebugStringA(toPrint);
#endif
}

// Used to print debug infomation from OpenGL, pulled straight from the official OpenGL wiki.
#ifndef NDEBUG
void APIENTRY
MessageCallback(GLenum source, GLenum type, GLuint id,
GLenum severity, GLsizei length,
const GLchar* message, const void* userParam) {


std::string errMessage;
errMessage = (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "");
errMessage += " type = ";
errMessage += type;
errMessage += ", severity = ";
errMessage += severity;
errMessage += ", message = ";
errMessage += message;
errMessage += "\n";

PrintLabeledDebugString("GL CALLBACK: ", errMessage.c_str());
}
#endif

// Creation, Rendering & Cleanup
using namespace DSC;

bool DSC::Renderer::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::SYSTEM::GWindow _win,
	GW::GRAPHICS::GOpenGLSurface _ogl)
{
	game = _game;
	gameConfig = _gameConfig;
	win = _win;
	ogl = _ogl;

	QueryOGLExtensionFunctions(ogl);
	mProxy.Create();
	win.GetClientWidth(width);
	win.GetClientHeight(height);

	#ifndef NDEBUG
		BindDebugCallback(); // In debug mode we link openGL errors to the console
	#endif
	if (LoadCameraLerpData() == false)
		return false;
	if (LoadLevelData() == false)
		return false;
	if (LoadEntityData() == false)
		return false;
	if (LoadFreeType() == false)
		return false;
	if (LoadShaders() == false) 
		return false;
	if (LoadUniforms() == false)
		return false;
	if (LoadTextUniforms() == false)
		return false;
	if (LoadTerrainUniforms() == false)
		return false;
	if (LoadUIUniforms() == false)
		return false;
	if (SetupDrawcalls() == false)
		return false;

	shutdown.Create(ogl, [&]() {
		if (+shutdown.Find(GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES, true)) {
			FreeOpenGLResources(); // unlike D3D we must be careful about destroy timing
		}
		});

	return true;
}

#pragma region ModelFunctions

bool DSC::Renderer::LoadUniforms()
{
	IntializeVertexBuffer();
	CreateIndexBuffer(indexBuffer, cpuLevel.levelIndices.data(), sizeof(unsigned int) * cpuLevel.levelIndices.size());
	SetUpUBOData();
	CreateUBO(&uniform_buffer_object, sizeof(UBOData));
	SetUBO();
	CreateSSBO();

	return true;
}
bool DSC::Renderer::LoadShaders()
{
	// Model Shaders
	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	Shader shader = Shader((*readCfg).at("Shaders").at("vertex").as<std::string>().c_str(), (*readCfg).at("Shaders").at("pixel").as<std::string>().c_str());
	shaderExecutable = shader.ID;
	
	// UI Shaders
	shader = Shader((*readCfg).at("Shaders").at("UIVertex").as<std::string>().c_str(), (*readCfg).at("Shaders").at("UIPixel").as<std::string>().c_str());
	shaderUIExecutable = shader.ID;
	uniform_handle_position = glGetUniformLocation(shaderUIExecutable, "uni_position");
	uniform_handle_scale = glGetUniformLocation(shaderUIExecutable, "uni_scale");
	uniform_handle_rotation = glGetUniformLocation(shaderUIExecutable, "uni_rotation");
	uniform_handle_depth = glGetUniformLocation(shaderUIExecutable, "uni_depth");
	glUseProgram(0);

	// Terrain Shaders
	shader = Shader((*readCfg).at("Shaders").at("terrainVertex").as<std::string>().c_str(), (*readCfg).at("Shaders").at("terrainPixel").as<std::string>().c_str());
	shaderTerrainExecutable = shader.ID;

	// Collider Shaders
	shader = Shader((*readCfg).at("Shaders").at("colliderVertex").as<std::string>().c_str(), (*readCfg).at("Shaders").at("colliderPixel").as<std::string>().c_str());
	shaderColliderExecutable = shader.ID;

	// Text Shaders
	shader = Shader((*readCfg).at("Shaders").at("textVertex").as<std::string>().c_str(), (*readCfg).at("Shaders").at("textPixel").as<std::string>().c_str());
	shaderTextExecutable = shader.ID;
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

//constructor helper functions 
void DSC::Renderer::CreateIndexBuffer(GLuint& IBO, const void* data, unsigned int sizeInBytes)
{
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes, data, GL_STATIC_DRAW);
}	
void DSC::Renderer::SetUpUBOData()
{
	mProxy.LookAtLHF(viewPositionVector, viewTargetVector, worldUpVector, viewMatrix);

	ogl.GetAspectRatio(AR);


	CreateAndSetOrthographicProjectionMatrix(UBOData.projectionMatrix, 1, -1, 1, -1, -1, 1);

	UBOData.sunDirection = lightDirection;
	UBOData.sunColor = lightColor;
	viewMatrix.row4.y = -1.0f;
	UBOData.viewMatrix = viewMatrix;
	UBOData.world = worldMatrix;

	mProxy.InverseF(viewMatrix, camMatrix);
	UBOData.camPos = viewPositionVector;
	UBOData.sunAmbient = sunAmbient;
}

void DSC::Renderer::CreateUBO(const void* data, unsigned int sizeInBytes)
{
	glGenBuffers(1, &uniform_buffer_object);
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_object);
	glBufferData(GL_UNIFORM_BUFFER, sizeInBytes, &UBOData, GL_DYNAMIC_DRAW);
}
void DSC::Renderer::CreateSSBO()
{

	for (int i = 0; i < cpuLevel.levelTransforms.size(); i++)
	{
		ssbo.instance_transforms[i] = cpuLevel.levelTransforms[i];
	}

	glGenBuffers(1, &ssbo_object);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_object);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SSBO), &ssbo, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	GLuint loc = glGetProgramResourceIndex(shaderExecutable, GL_SHADER_STORAGE_BLOCK, "SSBO");
	glShaderStorageBlockBinding(shaderExecutable, loc, ssbo_binding_index);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_binding_index, ssbo_object);
}

void DSC::Renderer::SetVertexAttributes()
{
	//DONE: Part 1e
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(H2B::VERTEX), (GLvoid*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(H2B::VERTEX), (GLvoid*)12);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(H2B::VERTEX), (GLvoid*)24);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}	
void DSC::Renderer::SetUBO()
{
	GLuint blockIndex = 0;
	blockIndex = glGetUniformBlockIndex(shaderExecutable, "UboData");
	glBindBufferBase(GL_UNIFORM_BUFFER, blockIndex, uniform_buffer_object);
	glUniformBlockBinding(shaderExecutable, blockIndex, 0);
}
bool DSC::Renderer::LoadLevelData()
{
	// begin loading level
	log.Create("../LevelLoaderLog.txt");
	log.EnableConsoleLogging(true);
	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	cpuLevel.LoadLevel((*readCfg).at("Level1").at("gameleveltxt").as<std::string>().c_str(), (*readCfg).at("Level1").at("h2b").as<std::string>().c_str(), log.Relinquish());
	return true;
}
bool DSC::Renderer::LoadEntityData()
{
	DSC::Gameplay engine(cpuLevel, log, gameConfig, game);
	return true;
}

void DSC::Renderer::IntializeVertexBuffer()
{
	CreateVertexBuffer(vertexArray, vertexBufferObject ,cpuLevel.levelVertices.data(), sizeof(H2B::VERTEX) * cpuLevel.levelVertices.size());
	SetVertexAttributes();
}
void DSC::Renderer::CreateVertexBuffer(GLuint& VAO, GLuint& VBO, const void* data, unsigned int sizeInBytes)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data, GL_STATIC_DRAW);
}	

void DSC::Renderer::DrawModel(Material& m)
{
	SetUpPipeline(shaderExecutable, vertexArray);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_object);
	ssbo.instanceIdx = m.transformStart;


	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(SSBO), &ssbo);
	for (int j = 0; j < m.modelMeshes.size(); ++j)
	{
		const H2B::MESH* mesh = &m.modelMeshes[j];
		UBOData.material = m.modelMaterials[j];
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBOData), &UBOData);
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, mesh->drawInfo.indexCount, GL_UNSIGNED_INT, (void*)((mesh->drawInfo.indexOffset + m.indexStart) * sizeof(unsigned int)), m.transformCount, m.vertexStart); //DONE: Part 1h
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	glBindVertexArray(0); // some video cards(cough Intel) need this set back to zero or they won't display
	glUseProgram(0);
}


void DSC::Renderer::SetUpPipeline(GLuint Shader, GLuint VAO)
{
	// DONE: Part 1h
	glUseProgram(Shader);
	glBindVertexArray(VAO);
}
#pragma endregion

#pragma region TextFunctions
bool DSC::Renderer::LoadFreeType()
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return -1;
	}

	FT_Face face;
	if (FT_New_Face(ft, "../fonts/Antonio-SemiBold.ttf", 0, &face))
	{
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		return -1;
	}
	else {
		// set size to load glyphs as
		FT_Set_Pixel_Sizes(face, 0, 48);

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// load first 128 characters of ASCII set
		for (unsigned char c = 0; c < 128; c++)
		{
			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}
			// generate texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
			// set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// now store character for later use
			Character character = {
				texture,
				{face->glyph->bitmap.width, face->glyph->bitmap.rows},
				{face->glyph->bitmap_left, face->glyph->bitmap_top},
				static_cast<unsigned int>(face->glyph->advance.x)
			};
			Characters.insert(std::pair<char, Character>(c, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	// destroy FreeType once we're finished
	
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	return true;
}

bool DSC::Renderer::LoadTextUniforms()
{
	InitializeTextVertexBuffer();
	SetTextUniform();
	return true;
}
void DSC::Renderer::SetTextUniform()
{
	GW::MATH::GMATRIXF projection;
	CreateAndSetOrthographicProjectionMatrix(projection, width, 0, height, 0, -1, 1);
	glUseProgram(shaderTextExecutable);
	glUniformMatrix4fv(glGetUniformLocation(shaderTextExecutable, "projection"), 1, GL_FALSE, (GLfloat*)&projection);
	glUseProgram(0);
}
void DSC::Renderer::InitializeTextVertexBuffer()
{
	CreateVertexBuffer(vertexTextArray, vertexTextBufferObject, NULL, sizeof(float) * 6 * 4);
	SetVertexTextAttributes();
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void DSC::Renderer::SetVertexTextAttributes()
{
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
}
void DSC::Renderer::RenderText(std::string text, float x, float y, float scale, vec3 color)
{
	SetUpPipeline(shaderTextExecutable, vertexTextArray);

	glUniform3f(glGetUniformLocation(shaderTextExecutable, "textColor"), color.r, color.g, color.b);
	glActiveTexture(GL_TEXTURE0);
	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			   			
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, vertexTextBufferObject);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glUseProgram(0);
	glBindVertexArray(0);
	glDisable(GL_CULL_FACE);
	glBindTexture(GL_TEXTURE_2D, 0);
}




#pragma endregion

#pragma region TerrainFunctions

bool DSC::Renderer::LoadTerrainUniforms()
{
	InitializeTerrainVertexBuffer();
	SetTerrainUBO();
	return true;
}

void DSC::Renderer::SetTerrainUBO()
{
	GLuint blockIndex = 0;
	blockIndex = glGetUniformBlockIndex(shaderTerrainExecutable, "UboData");
	glBindBufferBase(GL_UNIFORM_BUFFER, blockIndex, uniform_buffer_object);
	glUniformBlockBinding(shaderTerrainExecutable, blockIndex, 0);
}
void DSC::Renderer::SetTerrainData()
{
	// X Starts at 0 and ends at -12
	// You will need duplicate points since each unique point is only used once in the drawing process
	// example creat a v with unique points a, b, c
	// point b will need to be duplicated to create the next line
	// ab
	// bc
	
	// clear any existing data
	terrain.clear();

	// get world constraints (would be better to read from default.ini)
	float worldMinX = 0.0f;
	float worldMaxX = 12.0f;
	float worldMinY = 0.0f;
	float worldMaxY = 2.0f;

	if (!game->has<TERRAIN_LEVEL>()) {
		std::random_device rd;  // Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
		std::uniform_real_distribution<float> x_pos(worldMinX, worldMaxX);
		std::uniform_real_distribution<float> y_pos(worldMinY + 0.05f, worldMaxY - 0.05f);
		std::uniform_real_distribution<float> x_off(-0.002f, 0.002f);
		std::uniform_real_distribution<float> y_off(-0.002f, 0.002f);
		
		for (int i = 0; i < 500; i++) {
			float x = x_pos(gen);
			float y = y_pos(gen);
			float xo = x_off(gen);
			float yo = y_off(gen);
			GW::MATH::GVECTORF point1;
			GW::MATH::GVECTORF point2;
			point1 = { -x, y, 0.5, 1 };
			point2 = { -x + xo, y + yo, 0.5, 1 };
			terrain.push_back(point1);
			terrain.push_back(point2);
		}
	}
	else {

		// get frequency of points
		float x = 0.0f;
		float y = 0.1f;
		float z = 0.5f;
		float w = 1.0f;
		// how much in the y-direction per point
		float y_offset = 0.1f;
		// how much in the x-direction per point
		float x_offset = 0.1f;
		// minimum ground level
		float groundMin = 0.1f;
		// maximum ground level
		float groundMax = 0.5f;

		std::random_device rd;  // Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
		// set up a random 0-1 float
		std::uniform_real_distribution<float> stepChance(0.0f, 1.0f);

		GW::MATH::GVECTORF step;
		// begin and end at same point
		step = { worldMinX, groundMin, z, w };
		// push back first step to begin line
		terrain.push_back(step);

		// probability of moving terrain up, down, or remaining flat; sum to 1.0f
		float terrainUp = 0.4f;
		float terrainDown = 0.3f;
		float terrainFlat = 0.3f;

		float i = worldMinX;
		// iterate from min-x to max-x per x_offset
		for (i += x_offset; i < worldMaxX; i += x_offset) {
		
			// gen random 0 to 1
			float num = stepChance(gen);

			// preserve peaks (elimate plateaus)
			if (y >= groundMax) {
				y -= y_offset;
			}
			// crude probability check
			else if (num < terrainUp) {
				y += y_offset; // step up
			}
			else {
				num -= terrainUp;
				if (num < terrainDown) {
					y -= y_offset; // step down
				}
				else { // don't really need this with no logic when flat
					num -= terrainDown;
					if (num < terrainFlat) {
						// do not modify y
					}
					else {
						num -= terrainFlat;
					}
				}
			}

			// trend down for last iterations
			if (i > 11.5f) {
				terrainDown = 1.0f;
				terrainUp = 0.0f;
				terrainFlat = 0.0f;
			}

			// maintain min/max
			if (y > groundMax) {
				y = groundMax;
			}
			else if (y < groundMin) {
				y = groundMin;
			}
				
			// build step with results; needs negative x-axis due to camera/renderer
			step = { -i, y, z, w };

			// push back two points; first point is already set
			// sets second and third point to begin the next line
			terrain.push_back(step);
			terrain.push_back(step);
		}

		// set last point at max-x and groundMin to complete all lines
		step = { -worldMaxX, groundMin, z, w };
		terrain.push_back(step);

		// example
		// terrain.push_back({ -0.5,  0.5, 0.5, 1 });
	}
}

void DSC::Renderer::InitializeTerrainVertexBuffer()
{
	SetTerrainData();
	CreateVertexBuffer(vertexTerrainArray, vertexTerrainBufferObject, terrain.data(), sizeof(GW::MATH::GVECTORF) * terrain.size());
	SetVertexTerrainAttributes();
}
void DSC::Renderer::SetVertexTerrainAttributes()
{
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GW::MATH::GVECTORF), (GLvoid*)0);
	glEnableVertexAttribArray(0);
}
void DSC::Renderer::RenderTerrain()
{
	SetUpPipeline(shaderTerrainExecutable, vertexTerrainArray);


	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBOData), &UBOData);

	glDrawArrays(GL_LINES, 0, terrain.size());

	glUseProgram(0);
	glBindVertexArray(0);
}

#pragma endregion

#pragma region UIFunctions

void DSC::Renderer::LoadTexture(const char* file_path, GLuint& texture_object)
{
	GLint width, height, channels;
	unsigned char* data = stbi_load(file_path, &width, &height, &channels, 0);

	if (!data)
	{
		std::cout << "ERROR: Texture \"" << file_path << "\" Not Found!" << std::endl;
		return;
	}

	glGenTextures(1, &texture_object);
	glBindTexture(GL_TEXTURE_2D, texture_object);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLint format = 0;
	switch (channels)
	{
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		break;
	};

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);
}

std::vector<Sprite>	DSC::Renderer::LoadHudFromXML(std::string filepath)
{
	std::vector<Sprite> result;

	tinyxml2::XMLDocument document;
	tinyxml2::XMLError error_message = document.LoadFile(filepath.c_str());
	if (error_message != tinyxml2::XML_SUCCESS)
	{
		std::cout << "XML file [" + filepath + "] did not load properly." << std::endl;
		return std::vector<Sprite>();
	}

	std::string name = document.FirstChildElement("hud")->FindAttribute("name")->Value();
	GW::MATH2D::GVECTOR2F screen_size;
	screen_size.x = atof(document.FirstChildElement("hud")->FindAttribute("width")->Value());
	screen_size.y = atof(document.FirstChildElement("hud")->FindAttribute("height")->Value());

	tinyxml2::XMLElement* current = document.FirstChildElement("hud")->FirstChildElement("element");
	while (current)
	{
		Sprite s = Sprite();
		name = current->FindAttribute("name")->Value();
		FLOAT x = atof(current->FindAttribute("pos_x")->Value());
		FLOAT y = atof(current->FindAttribute("pos_y")->Value());
		FLOAT sx = atof(current->FindAttribute("scale_x")->Value());
		FLOAT sy = atof(current->FindAttribute("scale_y")->Value());
		FLOAT r = atof(current->FindAttribute("rotation")->Value());
		FLOAT d = atof(current->FindAttribute("depth")->Value());
		GW::MATH2D::GVECTOR2F s_min, s_max;
		s_min.x = atof(current->FindAttribute("sr_x")->Value());
		s_min.y = atof(current->FindAttribute("sr_y")->Value());
		s_max.x = atof(current->FindAttribute("sr_w")->Value());
		s_max.y = atof(current->FindAttribute("sr_h")->Value());
		UINT tid = atoi(current->FindAttribute("textureID")->Value());

		s.SetName(name);
		s.SetScale(sx, sy);
		s.SetPosition(x, y);
		s.SetRotation(r);
		s.SetDepth(d);
		s.SetScissorRect({ s_min, s_max });
		s.SetTextureIndex(tid);

		result.push_back(s);

		current = current->NextSiblingElement();
	}
	return result;
}


bool DSC::Renderer::LoadUIUniforms()
{
	InitializeUIVertexBuffer();
	UIIndices =
	{
		0, 1, 2,
		1, 3, 2
	};
	CreateIndexBuffer(indexUIBuffer, UIIndices.data(), sizeof(unsigned int) * UIIndices.size());

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// enable the scissor rectangle
	glEnable(GL_SCISSOR_TEST);
	// enable blending
	glEnable(GL_BLEND);
	// describe the blending modes for RGB channels and the A channel
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	return true;
}

void DSC::Renderer::SetUIData(HUD& hud, std::vector<std::string>& sprite_names, std::string _filepath)
{
	stbi_set_flip_vertically_on_load(true);




	for (size_t i = 0; i < sprite_names.size(); i++)
	{
		texture[i] = -1;
		std::string filepath = TEXTURES_PATH;
		filepath += sprite_names[i];
		LoadTexture(filepath.c_str(), texture[i]);
	}

	// HUD loading
	std::string filepath1 = XML_PATH;
	filepath1 += _filepath;
	HUD xmlP_items = LoadHudFromXML(filepath1);
	hud.insert(hud.end(), xmlP_items.begin(), xmlP_items.end());




	// sorting lambda based on depth of sprites
	auto sortfunc = [=](const Sprite& a, const Sprite& b)
	{
		return a.GetDepth() > b.GetDepth();
	};
	// sort the hud from furthest to closest
	std::sort(hud.begin(), hud.end(), sortfunc);
}



void DSC::Renderer::InitializeUIVertexBuffer()
{
	std::vector<std::string> sprite_names =
	{
		"ship.png",
		"ship.png",
		"ship.png",
		"ship.png",
		"ship.png",		
		"ship.png",		
		"bomb_circle.png",
		"bomb_circle.png",
		"bomb_circle.png",
		"bomb_circle.png",
		"bomb_circle.png",
		"bomb_circle.png",
	};

	SetUIData(allSprites, sprite_names, "playerHud.xml");

	playerLivesUI.push_back(allSprites[0]);
	playerLivesUI.push_back(allSprites[1]);
	playerLivesUI.push_back(allSprites[2]);
	playerLivesUI.push_back(allSprites[3]);
	playerLivesUI.push_back(allSprites[4]);
	bombCountUI.push_back(allSprites[5]);
	bombCountUI.push_back(allSprites[6]);
	bombCountUI.push_back(allSprites[7]);
	bombCountUI.push_back(allSprites[8]);
	bombCountUI.push_back(allSprites[9]);

	UIVerts =
	{
		-1.0f,  1.0f, 0.0f, 1.0f,		//[x,y,u,v]
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f
	};
	CreateVertexBuffer(vertexUIArray, vertexUIBufferObject, UIVerts.data(), sizeof(float) * UIVerts.size());
	SetVertexUIAttributes();
}
void DSC::Renderer::SetVertexUIAttributes()
{
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void DSC::Renderer::RenderUI(HUD _hud, int size)
{
	// since we have overlapping geometry we would like to compare with LEQUAL
	glDepthFunc(GL_LEQUAL);
	// set the shader 
	SetUpPipeline(shaderUIExecutable, vertexUIArray);
	// loop over all of the hud elements
	for (size_t i = 0; i < size; i++)
	{
		const Sprite& current_sprite = _hud[i];
		//// setup shader variable data
		GW::MATH2D::GVECTOR2F pos = current_sprite.GetPosition();
		GW::MATH2D::GVECTOR2F scale = current_sprite.GetScale();
		float rot = current_sprite.GetRotation();
		float depth = current_sprite.GetDepth();
		unsigned int tex_id = current_sprite.GetTextureIndex();


		glUniform2fv(uniform_handle_position, 1, (GLfloat*)&pos);
		glUniform2fv(uniform_handle_scale, 1, (GLfloat*)&scale);
		glUniform1fv(uniform_handle_rotation, 1, (GLfloat*)&rot);
		glUniform1fv(uniform_handle_depth, 1, (GLfloat*)&depth);

		//// activate the texture slot 0 and bind the texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[tex_id]);
		// draw the quad
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*)0);
	}
	
	// unbind the vertex array object and shader program
	glBindVertexArray(0);
	glUseProgram(0);
	// restore the depth testing function back to less
	glDepthFunc(GL_LESS);
}




#pragma endregion

#pragma region FlecsSystems

bool DSC::Renderer::Activate(bool runSystem)
{
	if (DrawUI.is_alive() &&
		DrawGameAndMinimap.is_alive() &&
		DrawTerrain.is_alive()) {
		if (runSystem) {
			DrawUI.enable();
			DrawGameAndMinimap.enable();
			DrawTerrain.enable();
		}
		else {
			DrawUI.disable();
			DrawGameAndMinimap.disable();
			DrawTerrain.disable();
		}
		return true;
	}
	return false;
}

bool DSC::Renderer::Shutdown()
{
	DrawUI.destruct();
	DrawGameAndMinimap.destruct();
	DrawTerrain.destruct();
	return true; // vulkan resource shutdown handled via GEvent in Init()
}
#pragma endregion

#pragma region HelperFunctions
void DSC::Renderer::CreateAndSetOrthographicProjectionMatrix(GW::MATH::GMATRIXF& mat, float right, float left, float top, float bottom, float n, float f)
{
	mat = GW::MATH::GIdentityMatrixF;
	mat.row1.x = 2 / (right - left);
	mat.row2.y = 2 / (top - bottom);
	mat.row3.z = -1 / (f - n);
	mat.row4.x = -(right + left) / (right - left);
	mat.row4.y = -(top + bottom) / (top - bottom);
	mat.row4.z = -(f + n) / (f - n);
}


#ifndef NDEBUG
void DSC::Renderer::BindDebugCallback()
{
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
}
#endif
std::string DSC::Renderer::ShaderAsString(const char* shaderFilePath)
{
	std::string output;
	unsigned int stringLength = 0;
	GW::SYSTEM::GFile file; file.Create();
	file.GetFileSize(shaderFilePath, stringLength);
	if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
		output.resize(stringLength);
		file.Read(&output[0], stringLength);
	}
	else
		std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
	return output;
}

#pragma endregion

bool DSC::Renderer::LoadCameraLerpData()
{
	// init flashTime
	flashTime = 0.0f;
	flashTimeLerp = 1.0f;

	// read from config file
	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	// offset from true playerx, float time speeds up camera lerp
	playerCameraOffset = (*readCfg).at("Player1").at("cameraoffset").as<float>();
	playerCameraFloatTime = (*readCfg).at("Player1").at("camerafloat").as<float>();
	// init; camerax becomes the current camera position
	playerCameraX = 0.0f;
	// init global camera x position
	game->set<CAMERA_X_POSITION>({ 0.0f });
	return true;
}
void DSC::Renderer::ClearRenderer()
{
	if (game->has<FLASH_EFFECT>()) {
		FlashEffect(0.08f, 0.05f);
	}
	else {
		glClearColor(0, 0, 0, 0);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DSC::Renderer::FlashEffect(float time, float offset)
{
	flashTime += game->delta_time();
	if (flashTime < time) {
		glClearColor(1, 1, 1, 1);
	}
	else if (flashTime < (time + offset)) {
		glClearColor(0, 0, 0, 0);
	}
	else if (flashTime < (time + offset * 2)) {
		glClearColor(0.5, 0, 0, 1);
	}
	else if (flashTime < (time + offset * 3)) {
		glClearColor(0, 0, 0, 0);
	}
	else if (flashTime < (time + offset * 4)) {
		glClearColor(0.5, 0.6, 0.5, 1);
	}
	else if (flashTime < (time + offset * 5)) {
		glClearColor(0, 0, 0, 0);
	}
	else if (flashTime < (time + offset * 6)) {
		GW::MATH2D::GVector2D::LerpF(flashTimeLerp, 0.0f, game->delta_time() * 30, flashTimeLerp);
		glClearColor(flashTimeLerp, 0, 0, 1);
	}
	else if (flashTime > (time + offset * 8)) {
		flashTime = 0.0f;
		flashTimeLerp = 1.0f;
		game->remove<FLASH_EFFECT>();
	}
}


bool DSC::Renderer::SetupDrawcalls()
{	
	struct RenderingSystem {};
	game->entity("Rendering System").add<RenderingSystem>();


	DrawTerrain = game->system<RenderingSystem>().kind(flecs::OnUpdate)
		.each([this](flecs::entity e, RenderingSystem) {
		
			std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();

			// draw new terrain if game has this level component
			if (game->has<GENERATE_NEW_TERRAIN>() || game->has<DESTROY_TERRAIN>()) {
				InitializeTerrainVertexBuffer();
				game->remove<GENERATE_NEW_TERRAIN>();
			}
			// Mini Map
			// Set Viewport
			glViewport(width / 3, height * 0.9, width / 3, height * 0.1);
			// Set Mini Map Projection matrix based on game world size
			CreateAndSetOrthographicProjectionMatrix(UBOData.projectionMatrix, (*readCfg).at("World").at("xsizemax").as<int>()/2, -(*readCfg).at("World").at("xsizemax").as<int>()/2, 1, -1, -1, 1);

			// Render Mini Map Terrain
			RenderTerrain();

			// Main Screen
			// Set the View Port and Projection Matrix
			glViewport(0, 0, width, height * 0.9f);
			CreateAndSetOrthographicProjectionMatrix(UBOData.projectionMatrix, 1, -1, 1, -1, -1, 1);

			// Render Main Screen Terrain
			RenderTerrain();
			// Render Main Screen Text
			std::string text = "ESC: Pause";
			RenderText(text, 10, 10, 0.2, { 1, 1, 1 });
			text = "R: Restart";
			RenderText(text, 10, 25, 0.2, { 1, 1, 1 });

		});

	DrawGameAndMinimap = game->system<Position, Orientation, Material>().kind(flecs::OnUpdate)
		.each([this](flecs::entity e, Position& p, Orientation& o, Material& m) {
			
			std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
			
			// transfer 2D orientation to 4x4
			ssbo.instanceIdx = m.transformStart;
			int i = ssbo.instanceIdx;
			ssbo.instance_transforms[i] = GW::MATH::GIdentityMatrixF;
			ssbo.instance_transforms[i].row4.x = -p.value.x;
			ssbo.instance_transforms[i].row4.y = p.value.y;
			ssbo.instance_transforms[i].row1.x = o.value.row1.x;
			ssbo.instance_transforms[i].row1.y = o.value.row1.y;			
			ssbo.instance_transforms[i].row2.x = o.value.row2.x;
			ssbo.instance_transforms[i].row2.y = o.value.row2.y;


			// Mini Map
			// Set Viewport
			glViewport(width / 3, height * 0.9, width / 3, height * 0.1);
			// Set Mini Map Projection matrix based on game world size
			CreateAndSetOrthographicProjectionMatrix(UBOData.projectionMatrix, (*readCfg).at("World").at("xsizemax").as<int>() / 2, -(*readCfg).at("World").at("xsizemax").as<int>() / 2, 1, -1, -1, 1);
			// Render Mini Map Models
			DrawModel(m);

		
			// Main Screen
			// Set the View Port and Projection Matrix
			glViewport(0, 0, width, height * 0.9f);
			CreateAndSetOrthographicProjectionMatrix(UBOData.projectionMatrix, 1, -1, 1, -1, -1, 1);

			// camera position and lerp
			if (m.name == (std::string)"Player One")
			{
				flecs::entity player = game->entity("Player One");
				// true = player facing right
				if (player.get<FacingDirection>()->value == true) {
					// trend towards right
					GW::MATH2D::GVector2D::LerpF(playerCameraX, -playerCameraOffset, playerCameraFloatTime * game->delta_time(), playerCameraX);
					UBOData.viewMatrix.row4.x = -p.value.x - playerCameraX;
				}
				else {
					//trend towards left
					GW::MATH2D::GVector2D::LerpF(playerCameraX, playerCameraOffset, playerCameraFloatTime * game->delta_time(), playerCameraX);
					UBOData.viewMatrix.row4.x = -p.value.x - playerCameraX;
				}
				// update global camera position; used for tracking active enemies
				game->set<CAMERA_X_POSITION>({ UBOData.viewMatrix.row4.x });
			}


			// Render Main Screen Models
			DrawModel(m);

		});

	DrawUI = game->system<LifeCount, BombCount, Score>().kind(flecs::PostUpdate)
		.each([this](flecs::entity e, LifeCount& l, BombCount& b, Score& s) {
			// Set the Viewport and projection for HUD
			glViewport(0, height * 0.9, width / 3, height * 0.1);
			// Render player lift sprites based on player life count
			RenderUI(playerLivesUI, l.value);
			// Render Bomb Count Sprites based on players bomb count
			RenderUI(bombCountUI, b.value);
			// Render High Score Text

			std::string score;
			if (s.value == 0)
			{
				score = "0000";
			}
			if (score.size() < 4)
			{
				score = '0';
				score += std::to_string(s.value);
			}
			RenderText(score, 100, 500, 5, { 1, 1, 1 });
			ogl.UniversalSwapBuffers();
		});


	return true;
}

bool DSC::Renderer::FreeOpenGLResources()
{
	// free Model resources
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteBuffers(1, &vertexBufferObject);
	glDeleteBuffers(1, &ssbo_object);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteProgram(shaderExecutable);
	glDeleteBuffers(1, &uniform_buffer_object);
	// free Terrain Resources
	glDeleteVertexArrays(1, &vertexTerrainArray);
	glDeleteBuffers(1, &vertexTerrainBufferObject);
	glDeleteProgram(shaderTerrainExecutable);	
	// free Collider Resources
	glDeleteVertexArrays(1, &vertexColliderArray);
	glDeleteBuffers(1, &vertexColliderBufferObject);
	glDeleteProgram(shaderColliderExecutable);
	// free UI Resources
	glDeleteBuffers(1, &indexUIBuffer);
	glDeleteVertexArrays(1, &vertexUIArray);
	glDeleteBuffers(1, &vertexUIBufferObject);
	glDeleteProgram(shaderUIExecutable);
	// free text resources
	glDeleteVertexArrays(1, &vertexTextArray);
	glDeleteBuffers(1, &vertexTextBufferObject);
	glDeleteProgram(shaderTextExecutable);
	return true;
}
