//////////////////////////////////////////////////
//gl2d.h				1.2.3
//Copyright(c) 2020 Luta Vlad
//https://github.com/meemknight/gl2d
//
//
//	dependences: glew, glm, stb_image, stb_trueType
//
//
//	features: 
//	
//	draw shapes with rotation color \
//		texture transparency
//	draw text with font and shadows
//	camera
//	shaders
//	setVsync
//	texture atlases and loading textures with \
//		padding to fix visual bugs when using \
//		pixel art sprite atlases
//	draw to screen of frame buffer that	can \
//		be used as a texture
//
//	a particle system that can use a custom \
//	shader and apply a pixelate effect
//
//
//////////////////////////////////////////////////

#pragma once

//enable simd functions
//set GL2D_SIMD to 0 if it doesn't work on your platform
#ifdef _WIN32
#define GL2D_SIMD 1
#else
#define GL2D_SIMD 0
#endif

//if you are not using visual studio make shure you link to "Opengl32.lib"

//if this is true it will use opengl130. If not it will use fome functionality from opengl3.
//With some small tweaks to the shader code you can go even lower with minimal effort.
#define GL2D_USE_OPENGL_130 false

#define GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED false
#define GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS true


//version of the shading language. this is the minimum but you can go lower if you midify the shader code with minimal effort
#define GL2D_OPNEGL_SHADER_VERSION "#version 130"
#define GL2D_OPNEGL_SHADER_PRECISION "precision highp float;"

//this is the default capacity of the renderer
#define GL2D_Renderer2D_Max_Triangle_Capacity 4200
#define GL2D_DefaultTextureCoords (glm::vec4{ 0, 1, 1, 0 })

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <random>
#include <stb_image/stb_image.h>
#include <stb_truetype/stb_truetype.h>

namespace gl2d
{

	//todo proper cmake, rename repo

	void init();

	void defaultErrorFunc(const char *msg);

	using errorFuncType = decltype(defaultErrorFunc);

	errorFuncType *setErrorFuncCallback(errorFuncType *newFunc);

	struct Font;

	//returns false on fail
	bool setVsync(bool b);

	namespace internal
	{
		struct ShaderProgram
		{
			GLuint id;
			int u_sampler;
		};

		float positionToScreenCoordsX(const float position, float w);
		float positionToScreenCoordsY(const float position, float h);

		stbtt_aligned_quad fontGetGlyphQuad(const Font font, const char c);
		glm::vec4 fontGetGlyphTextureCoords(const Font font, const char c);

	}

	///////////////////// COLOR ///////////////////
#pragma region Color

	using Color4f = glm::vec4;
#define Colors_Red (gl2d::Color4f{ 1, 0, 0, 1 })
#define Colors_Green (gl2d::Color4f{ 0, 1, 0, 1 })
#define Colors_Blue (gl2d::Color4f{ 0, 0, 1, 1 })
#define Colors_Black (gl2d::Color4f{ 0, 0, 0, 1 })
#define Colors_White (gl2d::Color4f{ 1, 1, 1, 1 })
#define Colors_Yellow (gl2d::Color4f{ 1, 1, 0, 1 })
#define Colors_Magenta (gl2d::Color4f{ 1, 0, 1, 1 })
#define Colors_Turqoise (gl2d::Color4f{ 0, 1, 1, 1 })
#define Colors_Orange (gl2d::Color4f{ 1, (float)0x7F / 255.0f, 0, 1 })
#define Colors_Purple (gl2d::Color4f{ 101.0f / 255.0f, 29.0f / 255.0f, 173.0f / 255.0f, 1 })
#define Colors_Gray (gl2d::Color4f{ (float)0x7F / 255.0f, (float)0x7F / 255.0f, (float)0x7F / 255.0f, 1 })

#pragma endregion

	///////////////////// MATH ////////////////////
#pragma region math

	using Rect = glm::vec4;
	glm::vec2 rotateAroundPoint(glm::vec2 vec, glm::vec2 point, const float degrees);
	glm::vec2 scaleAroundPoint(glm::vec2 vec, glm::vec2 point, float scale);

#pragma endregion

	///////////////////// Texture /////////////////////
#pragma region Texture

	struct Texture
	{
		GLuint id = 0;

		Texture() {};
		Texture(const char *file, bool pixelated = GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED,
			bool useMipMaps = GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS)
		{
			loadFromFile(file, pixelated, useMipMaps);
		}

		glm::ivec2 GetSize();

		//Note: This function expects a buffer of bytes in GL_RGBA format
		void createFromBuffer(const char *image_data, const int width,
			const int height, bool pixelated = GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED, bool useMipMaps = GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS);
		void create1PxSquare(const char *b = 0);
		void createFromFileData(const unsigned char *image_file_data, const size_t image_file_size,
			bool pixelated = GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED, bool useMipMaps = GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS);
		void createFromFileDataWithPixelPadding(const unsigned char *image_file_data,
			const size_t image_file_size, int blockSize,
			bool pixelated = GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED, bool useMipMaps = GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS);

		void loadFromFile(const char *fileName,
			bool pixelated = GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED, bool useMipMaps = GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS);

		//used for texture atlases, adds a pixel between each item to remove visual artefacts
		void loadFromFileWithPixelPadding(const char *fileName, int blockSize,
			bool pixelated = GL2D_DEFAULT_TEXTURE_LOAD_MODE_PIXELATED, bool useMipMaps = GL2D_DEFAULT_TEXTURE_LOAD_MODE_USE_MIPMAPS);

		void bind(const unsigned int sample = 0);
		void unbind();

		void cleanup();
	};

	struct TextureRegion
	{
		Texture texture;
		glm::vec4 textureCoords;
	};

#pragma endregion


	///////////////////// TextureAtlas /////////////////////
#pragma region TextureAtlas

	glm::vec4 computeTextureAtlas(int xCount, int yCount, int x, int y, bool flip = 0);

	glm::vec4 computeTextureAtlasWithPadding(int mapXsize, int mapYsize, int xCount, int yCount, int x, int y, bool flip = 0);

	struct TextureAtlas
	{
		TextureAtlas() {};
		TextureAtlas(int x, int y):xCount(x), yCount(y) {};

		int xCount = 0;
		int yCount = 0;

		glm::vec4 get(int x, int y, bool flip = 0)
		{
			return computeTextureAtlas(xCount, yCount, x, y, flip);
		}
	};

	struct TextureAtlasPadding
	{
		TextureAtlasPadding() {};

		//count count size of the full texture(in pixels)
		TextureAtlasPadding(int x, int y, int xSize, int ySize):xCount(x), yCount(y)
			, xSize(xSize), ySize(ySize)
		{
		};

		int xCount = 0;
		int yCount = 0;
		int xSize = 0;
		int ySize = 0;

		glm::vec4 get(int x, int y, bool flip = 0)
		{
			return computeTextureAtlasWithPadding(xSize, ySize, xCount, yCount, x, y, flip);
		}
	};
	// Get default internal texture (white texture)
#pragma endregion


	///////////////////// Font /////////////////////
#pragma region Font
#define Default_Font_Characters_Range_Begin cast(char, ' ')
#define Default_Font_Characters_Range_End cast(char, '~')
#define Default_Font_Characters_Range_Size cast(isize, Default_Font_Characters_Range_End - Default_Font_Characters_Range_Begin)

	typedef float Font_Size;

	typedef struct Font Font;
	struct Font
	{
		Texture           texture = {};
		glm::ivec2        size = {};
		stbtt_packedchar *packedCharsBuffer = 0;
		int               packedCharsBufferSize = 0;
		float             max_height = 0.f;

		Font() {}
		explicit Font(const char *file) { createFromFile(file); }

		void createFromTTF(const unsigned char *ttf_data, const size_t ttf_data_size);
		void createFromFile(const char *file);
	};

#pragma endregion

	///////////////////// Camera /////////////////////
#pragma region Camera

	struct Camera;

	struct Camera
	{
		glm::vec2  position = {};
		glm::vec2  target = {};   // Camera target (rotation and zoom origin)
		float rotation = 0.f; // Camera rotation in degrees
		float zoom = 1.0;     // Camera zoom (scaling), should be 1.0f by default

		void setDefault() { *this = Camera{}; }
		glm::mat3 getMatrix();

		void follow(glm::vec2 pos, float speed, float max, float w, float h);

		glm::vec2 convertPoint(const glm::vec2 &p, float windowW, float windowH); //todo move to internal
	};


#pragma endregion

	///////////////////// Renderer2d /////////////////////
#pragma region Renderer2d

	typedef glm::vec2 Position2D;
	typedef glm::vec4 Texture_Coords;

	struct FrameBuffer
	{
		unsigned int fbo = 0;
		Texture texture = {};

		void create(unsigned int w, unsigned int h);
		void resize(unsigned int w, unsigned int h);

		//clears resources
		void cleanup();

		//clears colors
		void clear();
	};



	enum Renderer2DBufferType
	{
		quadPositions,
		quadColors,
		texturePositions,

		bufferSize
	};

	typedef struct Renderer2D Renderer2D;
	struct Renderer2D
	{
		Renderer2D() {};

		//feel free to delete this lines but you probably don't want to copy the renderer from a place to another
		Renderer2D(Renderer2D &other) = delete;
		Renderer2D operator=(Renderer2D &other) = delete;

		void create();

		//todo
		void clear();

		GLuint buffers[Renderer2DBufferType::bufferSize] = {};
		GLuint vao = {};

		//4 elements each component
		glm::vec2 spritePositions[GL2D_Renderer2D_Max_Triangle_Capacity * 6];
		glm::vec4 spriteColors[GL2D_Renderer2D_Max_Triangle_Capacity * 6];
		glm::vec2 texturePositions[GL2D_Renderer2D_Max_Triangle_Capacity * 6];
		Texture   spriteTextures[GL2D_Renderer2D_Max_Triangle_Capacity * 6];

		int spritePositionsCount = 0;
		int spriteColorsCount = 0;
		int texturePositionsCount = 0;
		int spriteTexturesCount = 0;

		Texture white1pxSquareTexture = {};

		internal::ShaderProgram currentShader = {};
		Camera currentCamera = {};

		//window metrics, should be up to date at all times
		int windowW = 0;
		int windowH = 0;
		void updateWindowMetrics(int w, int h) { windowW = w; windowH = h; }

		//converts pixels to screen (top left) (bottom right)
		glm::vec4 toScreen(const glm::vec4 &transform);

		inline void clearDrawData()
		{
			spritePositionsCount = 0;
			spriteColorsCount = 0;
			spriteTexturesCount = 0;
			texturePositionsCount = 0;
		}

		glm::vec2 getTextSize(const char *text, const Font font, const float size = 1.5f,
			const float spacing = 4, const float line_space = 3);

		// The origin will be the bottom left corner since it represents the line for the text to be drawn
		//Pacing and lineSpace are influenced by size
		//todo the function should returns the size of the text drawn also refactor
		void renderText(glm::vec2 position, const char *text, const Font font, const Color4f color, const float size = 1.5f,
			const float spacing = 4, const float line_space = 3, bool showInCenter = 1, const Color4f ShadowColor = {0.1,0.1,0.1,1}
		, const Color4f LightColor = {});

		void renderRectangle(const Rect transforms, const Color4f colors[4], const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords = GL2D_DefaultTextureCoords);
		inline void renderRectangle(const Rect transforms, const Color4f colors, const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords = GL2D_DefaultTextureCoords)
		{
			Color4f c[4] = {colors,colors,colors,colors};
			renderRectangle(transforms, c, origin, rotation, texture, textureCoords);
		}

		void renderRectangleAbsRotation(const Rect transforms, const Color4f colors[4], const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords = GL2D_DefaultTextureCoords);
		inline void renderRectangleAbsRotation(const Rect transforms, const Color4f colors, const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords = GL2D_DefaultTextureCoords)
		{
			Color4f c[4] = {colors,colors,colors,colors};
			renderRectangleAbsRotation(transforms, c, origin, rotation, texture, textureCoords);
		}

		void renderRectangle(const Rect transforms, const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords = GL2D_DefaultTextureCoords);
		void renderRectangleAbsRotation(const Rect transforms, const glm::vec2 origin, const float rotation, const Texture texture, const glm::vec4 textureCoords = GL2D_DefaultTextureCoords);

		void renderRectangle(const Rect transforms, const Color4f colors[4], const glm::vec2 origin = {0,0}, const float rotation = 0);
		inline void renderRectangle(const Rect transforms, const Color4f colors, const glm::vec2 origin = {0,0}, const float rotation = 0)
		{
			Color4f c[4] = {colors,colors,colors,colors};
			renderRectangle(transforms, c, origin, rotation);
		}

		void renderRectangleAbsRotation(const Rect transforms, const Color4f colors[4], const glm::vec2 origin = {0,0}, const float rotation = 0);
		inline void renderRectangleAbsRotation(const Rect transforms, const Color4f colors, const glm::vec2 origin = {0,0}, const float rotation = 0)
		{
			Color4f c[4] = {colors,colors,colors,colors};
			renderRectangleAbsRotation(transforms, c, origin, rotation);
		}

		void render9Patch(const Rect position, const int borderSize, const Color4f color, const glm::vec2 origin, const float rotation, const Texture texture, const Texture_Coords textureCoords, const Texture_Coords inner_texture_coords);
		void render9Patch2(const Rect position, const Color4f color, const glm::vec2 origin, const float rotation, const Texture texture, const Texture_Coords textureCoords, const Texture_Coords inner_texture_coords);

		void clearScreen(const Color4f color = Color4f{0,0,0,0});

		void setShaderProgram(const internal::ShaderProgram shader);
		void setCamera(const Camera camera);

		void resetCameraAndShader();

		//draws to the screen
		void flush();
		void flushFBO(FrameBuffer frameBuffer);
	};

	void enableNecessaryGLFeatures();

#pragma endregion


	///////////////////// ParticleSysyem /////////////////////
#pragma region ParticleSysyem

	struct ParticleApearence
	{
		glm::vec2 size = {};
		glm::vec4 color1 = {};
		glm::vec4 color2 = {};
	};

	enum TRANZITION_TYPES
	{
		none = 0,
		linear,
		curbe,
		abruptCurbe,
		wave,
		wave2,
		delay,
		delay2
	};


	struct ParticleSettings
	{
		ParticleSettings *deathRattle = nullptr;
		ParticleSettings *subemitParticle = nullptr;

		int onCreateCount = 0;

		glm::vec2 subemitParticleTime = {};

		glm::vec2 positionX = {};
		glm::vec2 positionY = {};

		glm::vec2 particleLifeTime = {}; // move
		glm::vec2 directionX = {};
		glm::vec2 directionY = {};
		glm::vec2 dragX = {};
		glm::vec2 dragY = {};

		glm::vec2 rotation = {};
		glm::vec2 rotationSpeed = {};
		glm::vec2 rotationDrag = {};

		ParticleApearence createApearence = {};
		ParticleApearence createEndApearence = {};

		gl2d::Texture *texturePtr = 0;

		int tranzitionType = TRANZITION_TYPES::linear;
	};


	struct ParticleSystem
	{
		void initParticleSystem(int size);
		void cleanup();

		void emitParticleWave(ParticleSettings *ps, glm::vec2 pos);


		void applyMovement(float deltaTime);

		void draw(Renderer2D &r);

		bool postProcessing = true;
		float pixelateFactor = 2;

	private:

		int size = 0;

		float *posX = 0;
		float *posY = 0;

		float *directionX = 0;
		float *directionY = 0;

		float *rotation = 0;

		float *sizeXY = 0;

		float *dragX = 0;
		float *dragY = 0;

		float *duration = 0;
		float *durationTotal = 0;

		glm::vec4 *color = 0;

		float *rotationSpeed = 0;
		float *rotationDrag = 0;

		float *emitTime = 0;

		char *tranzitionType = 0;
		ParticleSettings **deathRattle = 0;
		ParticleSettings **thisParticleSettings = 0;
		ParticleSettings **emitParticle = 0;

		gl2d::Texture **textures = 0;

		std::mt19937 random{std::random_device{}()};

		gl2d::FrameBuffer fb = {};

		float rand(glm::vec2 v);
	};


#pragma endregion




};