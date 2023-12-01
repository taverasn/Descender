//#include "renderer.h"
//#include "../Components/Identification.h"
//#include "../Components/Visuals.h"
//#include "../Components/Physics.h"
//#include "../Utils/OpenGLExtensions.h"
//#include "../Entities/Prefabs.h"
//
//
//
//void PrintLabeledDebugString(const char* label, const char* toPrint)
//{
//std::cout << label << toPrint << std::endl;
//#if defined WIN32 //OutputDebugStringA is a windows-only function 
//OutputDebugStringA(label);
//OutputDebugStringA(toPrint);
//#endif
//}
//
//// Used to print debug infomation from OpenGL, pulled straight from the official OpenGL wiki.
//#ifndef NDEBUG
//void APIENTRY
//MessageCallback(GLenum source, GLenum type, GLuint id,
//GLenum severity, GLsizei length,
//const GLchar* message, const void* userParam) {
//
//
//std::string errMessage;
//errMessage = (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "");
//errMessage += " type = ";
//errMessage += type;
//errMessage += ", severity = ";
//errMessage += severity;
//errMessage += ", message = ";
//errMessage += message;
//errMessage += "\n";
//
//PrintLabeledDebugString("GL CALLBACK: ", errMessage.c_str());
//}
//#endif
//
//// Creation, Rendering & Cleanup
//using namespace DSC;
//
//bool DSC::Renderer::Init(std::shared_ptr<flecs::world> _game,
//	std::weak_ptr<const GameConfig> _gameConfig,
//	GW::SYSTEM::GWindow _win,
//	GW::GRAPHICS::GOpenGLSurface _ogl)
//{
//	game = _game;
//	gameConfig = _gameConfig;
//	win = _win;
//	ogl = _ogl;
//
//	QueryOGLExtensionFunctions(ogl);
//	mProxy.Create();
//
//	#ifndef NDEBUG
//		BindDebugCallback(); // In debug mode we link openGL errors to the console
//	#endif
//
//	if (LoadLevelData() == false)
//		return false;
//	if (LoadEntityData() == false)
//		return false;
//	if (LoadTimers() == false)
//		return false;
//	if (LoadShaders() == false) 
//		return false;
//	if (LoadUniforms() == false)
//		return false;
//	if (SetupDrawcalls() == false)
//		return false;
//
//	shutdown.Create(ogl, [&]() {
//		if (+shutdown.Find(GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES, true)) {
//			FreeOpenGLResources(); // unlike D3D we must be careful about destroy timing
//		}
//		});
//
//	return true;
//}
//
//bool DSC::Renderer::Activate(bool runSystem)
//{
//	if (DrawUI.is_alive() &&
//		DrawGameAndMinimap.is_alive()) {
//		if (runSystem) {
//			DrawUI.enable();
//			DrawGameAndMinimap.enable();
//		}
//		else {
//			DrawUI.disable();
//			DrawGameAndMinimap.disable();
//		}
//		return true;
//	}
//	return false;
//}
//
//bool DSC::Renderer::Shutdown()
//{
//	DrawUI.destruct();
//	DrawGameAndMinimap.destruct();
//	return true; // vulkan resource shutdown handled via GEvent in Init()
//}
//
////constructor helper functions 
//void DSC::Renderer::CreateIndexBuffer(const void* data, unsigned int sizeInBytes)
//{
//	glGenBuffers(1, &indexBuffer);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes, data, GL_STATIC_DRAW);
//}	
//void DSC::Renderer::SetUpUBOData()
//{
//	mProxy.LookAtLHF(viewPositionVector, viewTargetVector, worldUpVector, viewMatrix);
//
//	ogl.GetAspectRatio(AR);
//	win.GetClientWidth(width);
//	win.GetClientHeight(height);
//
//	CreateAndSetOrthographicProjectionMatrix(1, -1, 1, -1, -1, 1);
//
//	UBOData.sunDirection = lightDirection;
//	UBOData.sunColor = lightColor;
//	UBOData.viewMatrix = viewMatrix;
//	UBOData.world = worldMatrix;
//
//	mProxy.InverseF(viewMatrix, camMatrix);
//	UBOData.camPos = viewPositionVector;
//	UBOData.sunAmbient = sunAmbient;
//}
//
//void DSC::Renderer::CreateAndSetOrthographicProjectionMatrix(float right, float left, float top, float bottom, float n, float f)
//{
//	UBOData.projectionMatrix = GW::MATH::GIdentityMatrixF;
//	UBOData.projectionMatrix.row1.x = 2 / (right - left);
//	UBOData.projectionMatrix.row1.w = -(right + left) / (right - left);
//	UBOData.projectionMatrix.row2.y = 2 / (top - bottom);
//	UBOData.projectionMatrix.row2.w = -(top + bottom) / (top - bottom);
//	UBOData.projectionMatrix.row3.z = -2 / (f - n);
//	UBOData.projectionMatrix.row3.w = -(f + n) / (f - n);
//
//	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBOData), &UBOData);
//
//}
//
//void DSC::Renderer::CreateUBO(const void* data, unsigned int sizeInBytes)
//{
//	glGenBuffers(1, &uniform_buffer_object);
//	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_object);
//	glBufferData(GL_UNIFORM_BUFFER, sizeInBytes, &UBOData, GL_DYNAMIC_DRAW);
//}
//void DSC::Renderer::CreateSSBO()
//{
//
//	for (int i = 0; i < cpuLevel.levelTransforms.size(); i++)
//	{
//		ssbo.instance_transforms[i] = cpuLevel.levelTransforms[i];
//	}
//
//	glGenBuffers(1, &ssbo_object);
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_object);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SSBO), &ssbo, GL_DYNAMIC_COPY);
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//
//
//	GLuint loc = glGetProgramResourceIndex(shaderExecutable, GL_SHADER_STORAGE_BLOCK, "SSBO");
//	glShaderStorageBlockBinding(shaderExecutable, loc, ssbo_binding_index);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo_binding_index, ssbo_object);
//}
//bool DSC::Renderer::LoadLevelData()
//{
//	// begin loading level
//	log.Create("../LevelLoaderLog.txt");
//	log.EnableConsoleLogging(true);
//	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
//	cpuLevel.LoadLevel((*readCfg).at("Level1").at("gameleveltxt").as<std::string>().c_str(), (*readCfg).at("Level1").at("h2b").as<std::string>().c_str(), log.Relinquish());
//	return true;
//}
//bool DSC::Renderer::LoadEntityData()
//{
//	DSC::Gameplay engine(cpuLevel, log, gameConfig, game);
//	return true;
//}
//bool DSC::Renderer::LoadTimers()
//{
//	flashTimer = XTime(10, 0.75);
//	return true;
//}
//bool DSC::Renderer::LoadUniforms()
//{
//	IntializeVertexBuffer();
//	CreateIndexBuffer(cpuLevel.levelIndices.data(), sizeof(unsigned int) * cpuLevel.levelIndices.size());
//	SetUpUBOData();
//	CreateUBO(&uniform_buffer_object, sizeof(UBOData));
//	SetUBO();
//	CreateSSBO();
//	glUseProgram(0);
//	return true;
//}
//bool DSC::Renderer::LoadShaders()
//{
//	
//	CompileVertexShader();
//	CompileFragmentShader();
//
//	CreateExecutableShaderProgram();
//	return true;
//}
//
//#ifndef NDEBUG
//void DSC::Renderer::BindDebugCallback()
//{
//	glEnable(GL_DEBUG_OUTPUT);
//	glDebugMessageCallback(MessageCallback, 0);
//}
//#endif
//void DSC::Renderer::IntializeVertexBuffer()
//{
//	CreateVertexBuffer(cpuLevel.levelVertices.data(), sizeof(H2B::VERTEX) * cpuLevel.levelVertices.size());
//	SetVertexAttributes();
//}
//void DSC::Renderer::CreateVertexBuffer(const void* data, unsigned int sizeInBytes)
//{
//	glGenVertexArrays(1, &vertexArray);
//	glGenBuffers(1, &vertexBufferObject);
//	glBindVertexArray(vertexArray);
//	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
//	glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data, GL_STATIC_DRAW);
//}	
//std::string DSC::Renderer::ShaderAsString(const char* shaderFilePath)
//{
//	std::string output;
//	unsigned int stringLength = 0;
//	GW::SYSTEM::GFile file; file.Create();
//	file.GetFileSize(shaderFilePath, stringLength);
//	if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
//		output.resize(stringLength);
//		file.Read(&output[0], stringLength);
//	}
//	else
//		std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
//	return output;
//}
//void DSC::Renderer::CompileVertexShader()
//{
//	char errors[1024];
//	GLint result;
//
//	vertexShader = glCreateShader(GL_VERTEX_SHADER);
//
//	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
//	std::string vertexShaderSource = ShaderAsString((*readCfg).at("Shaders").at("vertex").as<std::string>().c_str());
//
//	const GLchar* strings[1] = { vertexShaderSource.c_str() };
//	const GLint lengths[1] = { vertexShaderSource.length() };
//	glShaderSource(vertexShader, 1, strings, lengths);
//
//	glCompileShader(vertexShader);
//	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
//	if (result == false)
//	{
//		glGetShaderInfoLog(vertexShader, 1024, NULL, errors);
//		PrintLabeledDebugString("Vertex Shader Errors:\n", errors);
//		abort();
//		return;
//	}
//}	
//void DSC::Renderer::CompileFragmentShader()
//{
//	char errors[1024];
//	GLint result;
//	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
//	std::string fragmentShaderSource = ShaderAsString((*readCfg).at("Shaders").at("pixel").as<std::string>().c_str());
//	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//
//
//	const GLchar* strings[1] = { fragmentShaderSource.c_str() };
//	const GLint lengths[1] = { fragmentShaderSource.length() };
//	glShaderSource(fragmentShader, 1, strings, lengths);
//
//	glCompileShader(fragmentShader);
//	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
//	if (result == false)
//	{
//		glGetShaderInfoLog(fragmentShader, 1024, NULL, errors);
//		PrintLabeledDebugString("Fragment Shader Errors:\n", errors);
//		abort();
//		return;
//	}
//}
//void DSC::Renderer::CreateExecutableShaderProgram()
//{
//	char errors[1024];
//	GLint result;
//
//	shaderExecutable = glCreateProgram();
//	glAttachShader(shaderExecutable, vertexShader);
//	glAttachShader(shaderExecutable, fragmentShader);
//	glLinkProgram(shaderExecutable);
//	glGetProgramiv(shaderExecutable, GL_LINK_STATUS, &result);
//	if (result == false)
//	{
//		glGetProgramInfoLog(shaderExecutable, 1024, NULL, errors);
//		std::cout << errors << std::endl;
//	}
//}
//void DSC::Renderer::SetVertexAttributes()
//{
//	//DONE: Part 1e
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(H2B::VERTEX), (GLvoid*)0);
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(H2B::VERTEX), (GLvoid*)12);
//	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(H2B::VERTEX), (GLvoid*)24);
//	glEnableVertexAttribArray(0);
//	glEnableVertexAttribArray(1);
//	glEnableVertexAttribArray(2);
//}	
//void DSC::Renderer::SetUBO()
//{
//	GLuint blockIndex = 0;
//	blockIndex = glGetUniformBlockIndex(shaderExecutable, "UboData");
//	glBindBufferBase(GL_UNIFORM_BUFFER, blockIndex, uniform_buffer_object);
//	glUniformBlockBinding(shaderExecutable, blockIndex, 0);
//}
//
//void DSC::Renderer::FlashEffect(float time, float offset)
//{
//	flashTimer.Signal();
//	if (flashTimer.TotalTime() >= time)
//	{
//		glClearColor(1, 1, 1, 1);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	}
//	if (flashTimer.TotalTime() >= time + offset)
//	{
//		glClearColor(0, 0, 0, 0);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	}
//	if (flashTimer.TotalTime() >= time + offset * 2)
//	{
//		glClearColor(1, 1, 1, 1);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	}
//	if (flashTimer.TotalTime() >= time + offset * 3)
//	{
//		glClearColor(0, 0, 0, 0);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		flashTimer.Restart();
//	}
//}
//
//
//bool DSC::Renderer::SetupDrawcalls()
//{
//	struct RenderingSystem {
//		std::vector<Material*> materials;
//	}; // local definition so we control iteration counts
//	game->entity("Rendering System").add<RenderingSystem>();
//
//
//	DrawGameAndMinimap = game->system<Position, Orientation, Material>().kind(flecs::OnUpdate)
//		.each([this](flecs::entity e, Position& p, Orientation& o, Material& m) {
//			ssbo.instanceIdx = m.transformStart;
//			//int i = draw_counter;
//			int i = ssbo.instanceIdx;
//			//GW::MATH::GMATRIXF tempMatrix = GW::MATH::GIdentityMatrixF;
//			ssbo.instance_transforms[i] = GW::MATH::GIdentityMatrixF;
//			// transfer 2D orientation to 4x4
//			ssbo.instance_transforms[i].row4.x = -p.value.x;
//			ssbo.instance_transforms[i].row4.y = p.value.y;
//			ssbo.instance_transforms[i].row1.x = o.value.row1.x;
//			ssbo.instance_transforms[i].row1.y = o.value.row1.y;			
//			ssbo.instance_transforms[i].row2.x = o.value.row2.x;
//			ssbo.instance_transforms[i].row2.y = o.value.row2.y;
//			//ssbo.instance_transforms[i] = tempMatrix;
//
//			if (m.name == (std::string)"Player One")
//			{
//
//				UBOData.viewMatrix.row4.x = -p.value.x;
//				UBOData.viewMatrix.row4.y = -1.0f;
//				//UBOData.viewMatrix.row4.z = 1;
//
//			}
//
//
//			// Bombs and Player Lives
//
//			//glViewport(0, height * 0.8, width / 3, height * 0.2);
//			//CreateAndSetOrthographicProjectionMatrix(1, -1, 1, -1, -1, 1);
//			//DrawModel(m);
//
//
//
//			// Mini Map
//			std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
//
//			glViewport(width / 3, height * 0.8, width / 3, height * 0.2);
//		
//			CreateAndSetOrthographicProjectionMatrix((*readCfg).at("World").at("xsizemax").as<int>()/2, -(*readCfg).at("World").at("xsizemax").as<int>() / 2, 1, -1, -1, 1);
//			DrawModel(m);
//
//
//			// Main Screen
//			glViewport(0, 0, width, height * 0.8f);
//			CreateAndSetOrthographicProjectionMatrix(1, -1, 1, -1, -3, 3);
//			DrawModel(m);
//
//		});
//
//	DrawUI = game->system<RenderingSystem>().kind(flecs::PostUpdate)
//		.each([this](flecs::entity e, RenderingSystem& s) {
//
//				ogl.UniversalSwapBuffers();
//		});
//	return true;
//}
//
//void DSC::Renderer::DrawModel(Material& m)
//{
//	SetUpPipeline();
//	ssbo.instanceIdx = m.transformStart;
//
//
//	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(SSBO), &ssbo);
//	for (int j = 0; j < m.modelMeshes.size(); ++j)
//	{
//		const H2B::MESH* mesh = &m.modelMeshes[j];
//		UBOData.material = m.modelMaterials[j];
//		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBOData), &UBOData);
//		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, mesh->drawInfo.indexCount, GL_UNSIGNED_INT, (void*)((mesh->drawInfo.indexOffset + m.indexStart) * sizeof(unsigned int)), m.transformCount, m.vertexStart); //DONE: Part 1h
//	}
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//	glBindVertexArray(0); // some video cards(cough Intel) need this set back to zero or they won't display
//	glUseProgram(0);
//}
//
//
//void DSC::Renderer::SetUpPipeline()
//{
//	// DONE: Part 1h
//	glUseProgram(shaderExecutable);
//	glBindVertexArray(vertexArray);
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_object);
//}
//
//
//bool DSC::Renderer::FreeOpenGLResources()
//{
//	// free resources
//	glDeleteVertexArrays(1, &vertexArray);
//	glDeleteBuffers(1, &vertexBufferObject);
//	glDeleteBuffers(1, &ssbo_object);
//	glDeleteBuffers(1, &indexBuffer);
//	glDeleteShader(vertexShader);
//	glDeleteShader(fragmentShader);
//	glDeleteProgram(shaderExecutable);
//	glDeleteBuffers(1, &uniform_buffer_object);
//	return true;
//}
