#ifndef RENDERER_H
#define RENDERER_H


#include "../GameConfig.h"
#include "load_data_oriented.h"
#include "GamePlayObjs.h"
#include "../Utils/XTime.h"
#include "../Components/Visuals.h"


#include "sprite.h"


enum TEXTURE_ID { LIFE_1 = 0, LIFE_2, LIFE_3, LIFE_4, LIFE_5, BOMB_1, BOMB_2, BOMB_3, BOMB_4, BOMB_5, FILLER_1, FILLER_2, COUNT };

using HUD = std::vector<Sprite>;

namespace DSC
{
	

	// Creation, Rendering & Cleanup
	class Renderer
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// handle to our running ECS systems
		flecs::system DrawGameAndMinimap;
		flecs::system DrawUI;
		flecs::system DrawTerrain;
		// camera positions
		float playerCameraOffset;
		float playerCameraFloatTime;
		float playerCameraX;
		float flashTime;
		float flashTimeLerp;

		GW::SYSTEM::GWindow win;
		GW::GRAPHICS::GOpenGLSurface ogl;
		// Model
		Level_Data cpuLevel;
		GLuint vertexArray = 0;
		GLuint vertexBufferObject = 0;
		GLuint indexBuffer = 0;
		GLuint shaderExecutable = 0;
		// Terrain
		GLuint vertexTerrainArray = 0;
		GLuint vertexTerrainBufferObject = 0;
		GLuint shaderTerrainExecutable = 0;

		std::vector<GW::MATH::GVECTORF> terrain;


		// Text Rendering
		struct vec2 {
			float x, y;
		};
		struct vec3 {
			float r,g,b;
		};

		struct Character {
			unsigned int TextureID;  // ID handle of the glyph texture
			vec2 Size;       // Size of glyph
			vec2 Bearing;    // Offset from baseline to left/top of glyph
			unsigned int Advance;    // Offset to advance to next glyph
		};

		std::map<GLchar, Character> Characters;

		GLuint vertexTextArray = 0;
		GLuint vertexTextBufferObject = 0;
		GLuint shaderTextExecutable = 0;


		// UI
		GLuint indexUIBuffer = 0;
		GLuint vertexUIArray = 0;
		GLuint vertexUIBufferObject = 0;

		GLuint shaderUIExecutable = 0;


		GLint  uniform_handle_position = 0;
		GLint  uniform_handle_scale = 0;
		GLint  uniform_handle_rotation = 0;
		GLint  uniform_handle_depth = 0;

		GLuint texture[TEXTURE_ID::COUNT];
		HUD	   playerLivesUI;
		HUD	   bombCountUI;
		HUD    allSprites;

		std::vector<unsigned int> UIIndices;
		std::vector<float> UIVerts;

		// Collider
		GLuint vertexColliderArray = 0;
		GLuint vertexColliderBufferObject = 0;
		GLuint shaderColliderExecutable = 0;

		// UBO and SSBO
		GLuint uniform_buffer_object = 0;
		float AR;
		unsigned int width;
		unsigned int height;
		GW::MATH::GMatrix mProxy;
		GW::MATH::GMATRIXF yRotMatrix = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF worldMatrix = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF viewMatrix = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF camMatrix = GW::MATH::GIdentityMatrixF;
		GW::MATH::GMATRIXF projectionMatrix = GW::MATH::GIdentityMatrixF;
		GW::MATH::GVECTORF lightDirection = { -1, -1, -2 };
		GW::MATH::GVECTORF lightColor = { 0.9, 0.9, 1.0, 1 };
		GW::MATH::GVECTORF viewPositionVector = { 0, 0, 1, 1 };
		GW::MATH::GVECTORF viewTargetVector = { 0, 0, 0, 1 };
		GW::MATH::GVECTORF worldUpVector = { 0, 1, 0, 0 };
		GW::MATH::GVECTORF sunAmbient = { 1, 1, 1, 1 };
		GLuint ssbo_object = 0;
		GLuint ssbo_binding_index = 0;
		GW::SYSTEM::GLog log; // handy for logging any messages/warning/errors
		GW::CORE::GEventReceiver shutdown;
		struct UBO_DATA
		{
			GW::MATH::GVECTORF sunDirection, sunColor, sunAmbient, camPos; //lighting info
			GW::MATH::GMATRIXF viewMatrix, projectionMatrix; // viewing info
			// sub-mesh transform and material data, updated each draw
			GW::MATH::GMATRIXF world; // final world space transformwa
			H2B::ATTRIBUTES material; // color/texture of surface

		};
		UBO_DATA UBOData;
		static constexpr unsigned int Instance_Max = 1000;
		struct SSBO
		{
			GW::MATH::GMATRIXF instance_transforms[Instance_Max];
			int instanceIdx;
		}ssbo;
	public:
		bool Init(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			GW::SYSTEM::GWindow _win,
			GW::GRAPHICS::GOpenGLSurface _ogl);
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
		bool LoadEntityData();
		void ClearRenderer();
		
	private:
		void FlashEffect(float time, float offset);

#pragma region Text
		bool LoadFreeType();
		bool LoadTextUniforms();
		void SetTextUniform();
		void InitializeTextVertexBuffer();
		void SetVertexTextAttributes();
		void RenderText(std::string text, float x, float y, float scale, vec3 color);
#pragma endregion


#pragma region Terrain
		bool LoadTerrainUniforms();
		void SetTerrainData();
		void InitializeTerrainVertexBuffer();
		void SetVertexTerrainAttributes();
		void SetTerrainUBO();
		void RenderTerrain();
#pragma endregion

#pragma region UI
		bool LoadUIUniforms();

		void SetUIData(HUD& hud, std::vector<std::string>& sprite_names, std::string _filepath);
		void InitializeUIVertexBuffer();
		void SetVertexUIAttributes();
		void RenderUI(HUD _hud, int size);
		void LoadTexture(const char* file_path, GLuint& texture_object);
		std::vector<Sprite>	LoadHudFromXML(std::string filepath);
#pragma endregion

#pragma region Collider
		bool LoadColliderUniforms();
		void SetColliderData();
		void InitializeColliderVertexBuffer();
		void SetVertexColliderAttributes();
		void RenderCollider();
#pragma endregion





		void DrawModel(Material& m);
		void CreateAndSetOrthographicProjectionMatrix(GW::MATH::GMATRIXF& mat, float right, float left, float top, float bottom, float n, float f);
		bool LoadShaders();
		bool LoadUniforms();
		bool LoadLevelData();
		bool SetupDrawcalls();
		void SetUpUBOData();
		bool FreeOpenGLResources();
		std::string DSC::Renderer::ShaderAsString(const char* shaderFilePath);
		//constructor helper functions 
		void CreateIndexBuffer(GLuint& IBO, const void* data, unsigned int sizeInBytes);
		void CreateUBO(const void* data, unsigned int sizeInBytes);
		void CreateSSBO();
		bool LoadCameraLerpData();
#ifndef NDEBUG
		void BindDebugCallback();
#endif
		void IntializeVertexBuffer();
		void CreateVertexBuffer(GLuint& VAO, GLuint& VBO, const void* data, unsigned int sizeInBytes);
		void SetVertexAttributes();
		void SetUBO();
		//Render helper functions
		void SetUpPipeline(GLuint Shader, GLuint VAO);
	};

	
};

#endif
