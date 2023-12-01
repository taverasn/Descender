//#ifndef RENDERER_H
//#define RENDERER_H
//
//
//#include "../GameConfig.h"
//#include "load_data_oriented.h"
//#include "GamePlayObjs.h"
//#include "../Utils/XTime.h"
//#include "../Components/Visuals.h"
//
//
//
//namespace DSC
//{
//	// Creation, Rendering & Cleanup
//	class Renderer
//	{
//		// shared connection to the main ECS engine
//		std::shared_ptr<flecs::world> game;
//		// non-ownership handle to configuration settings
//		std::weak_ptr<const GameConfig> gameConfig;
//		// handle to our running ECS systems
//		flecs::system DrawGameAndMinimap;
//		flecs::system DrawUI;
//		// proxy handles
//		GW::SYSTEM::GWindow win;
//		GW::GRAPHICS::GOpenGLSurface ogl;
//		Level_Data cpuLevel;
//		GLuint vertexArray = 0;
//		GLuint vertexBufferObject = 0;
//		GLuint indexBuffer = 0;
//		GLuint vertexShader = 0;
//		GLuint fragmentShader = 0;
//		GLuint shaderExecutable = 0;
//		GLuint uniform_buffer_object = 0;
//		float AR;
//		unsigned int width;
//		unsigned int height;
//		XTime flashTimer;
//		GW::MATH::GMatrix mProxy;
//		GW::MATH::GMATRIXF yRotMatrix = GW::MATH::GIdentityMatrixF;
//		GW::MATH::GMATRIXF worldMatrix = GW::MATH::GIdentityMatrixF;
//		GW::MATH::GMATRIXF viewMatrix = GW::MATH::GIdentityMatrixF;
//		GW::MATH::GMATRIXF camMatrix = GW::MATH::GIdentityMatrixF;
//		GW::MATH::GMATRIXF projectionMatrix = GW::MATH::GIdentityMatrixF;
//		GW::MATH::GVECTORF lightDirection = { -1, -1, -2 };
//		GW::MATH::GVECTORF lightColor = { 0.9, 0.9, 1.0, 1 };
//		GW::MATH::GVECTORF viewPositionVector = { 0, 0, 1, 1 };
//		GW::MATH::GVECTORF viewTargetVector = { 0, 0, 0, 1 };
//		GW::MATH::GVECTORF worldUpVector = { 0, 1, 0, 0 };
//		GW::MATH::GVECTORF sunAmbient = { 0.25, 0.25, 0.35, 1 };
//		GLuint ssbo_object = 0;
//		GLuint ssbo_binding_index = 0;
//		GW::SYSTEM::GLog log; // handy for logging any messages/warning/errors
//		GW::CORE::GEventReceiver shutdown;
//		struct UBO_DATA
//		{
//			GW::MATH::GVECTORF sunDirection, sunColor, sunAmbient, camPos; //lighting info
//			GW::MATH::GMATRIXF viewMatrix, projectionMatrix; // viewing info
//			// sub-mesh transform and material data, updated each draw
//			GW::MATH::GMATRIXF world; // final world space transformwa
//			H2B::ATTRIBUTES material; // color/texture of surface
//
//		};
//		UBO_DATA UBOData;
//		static constexpr unsigned int Instance_Max = 1000;
//		struct SSBO
//		{
//			GW::MATH::GMATRIXF instance_transforms[Instance_Max];
//			int instanceIdx;
//		}ssbo;
//	public:
//		bool Init(std::shared_ptr<flecs::world> _game,
//			std::weak_ptr<const GameConfig> _gameConfig,
//			GW::SYSTEM::GWindow _win,
//			GW::GRAPHICS::GOpenGLSurface _ogl);
//		bool Activate(bool runSystem);
//		// release any resources allocated by the system
//		bool Shutdown();
//		void FlashEffect(float time, float offset);
//		bool LoadEntityData();
//	private:
//		void DrawModel(Material& m);
//		void CreateAndSetOrthographicProjectionMatrix(float right, float left, float top, float bottom, float n, float f);
//		bool LoadShaders();
//		bool LoadUniforms();
//		bool LoadTimers();
//		bool LoadLevelData();
//		bool SetupDrawcalls();
//		void SetUpUBOData();
//		bool FreeOpenGLResources();
//		std::string DSC::Renderer::ShaderAsString(const char* shaderFilePath);
//		//constructor helper functions 
//		void CreateIndexBuffer(const void* data, unsigned int sizeInBytes);
//		void CreateUBO(const void* data, unsigned int sizeInBytes);
//		void CreateSSBO();
//#ifndef NDEBUG
//		void BindDebugCallback();
//#endif
//		void IntializeVertexBuffer();
//		void CreateVertexBuffer(const void* data, unsigned int sizeInBytes);
//		void CompileVertexShader();
//		void CompileFragmentShader();
//		void CreateExecutableShaderProgram();
//		void SetVertexAttributes();
//		void SetUBO();
//		//Render helper functions
//		void SetUpPipeline();
//	};
//};
//
//#endif
