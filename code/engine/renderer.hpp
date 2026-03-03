//
// renderer.hpp : Controls the renderer.
//

#pragma once

// External headers
#include <SDL3/SDL.h>

// Standard headers
#include <span>

REDLYNX_NAMESPACE_BEGIN_ENGINE

// Shader stages
enum class ShaderStage : uint8
{
	Vertex,
	Fragment
};

// Vertex format for 3D rendering
struct Vertex3D
{
	f32 Position[3];
	f32 Normal[3];
	f32 TexCoord[2];
	f32 Color[4];
};

// Vertex format for 2D rendering
struct Vertex2D
{
	f32 Position[2];
	f32 TexCoord[2];
	f32 Color[4];
};

// Texture format
enum class TextureFormat : uint8
{
	RGBA8,
	BGRA8,
	R8,
	Depth24Stencil8,
	Depth32
};

// Texture filtering mode
enum class TextureFilter : uint8
{
	Nearest,
	Linear
};

// Texture wrap mode
enum class TextureWrap : uint8
{
	Repeat,
	ClampToEdge,
	MirroredRepeat
};

// Texture sampler configuration
struct SamplerConfiguration
{
	TextureFilter MinFilter = TextureFilter::Linear;
	TextureFilter MagFilter = TextureFilter::Linear;
	TextureFilter MipFilter = TextureFilter::Linear;
	TextureWrap WrapU = TextureWrap::Repeat;
	TextureWrap WrapV = TextureWrap::Repeat;
	TextureWrap WrapW = TextureWrap::Repeat;
	f32 MipLodBias = 0.0f;
	f32 MaxAnisotropy = 1.0f;
	bool EnableCompre = false;
};

// BLend mode for rendering
enum class BlendMode : uint8
{
	None,
	Alpha,
	Additive,
	Multiply,
};

// Cull mode for rendering
enum class CullMode : uint8
{
	None,
	Front,
	Back,
};

// Front face winding order
enum class FrontFace : uint8
{
	CounterClockwise,
	Clockwise
};

// Depth compare function
enum class CompareFunction : uint8
{
	Never,
	Less,
	Equal,
	LessOrEqual,
	Greater,
	NotEqual,
	GreaterOrEqual,
	Always
};

// GPU texture handle.
struct GPUTexture
{
	SDL_GPUTexture* Handle = nullptr;
	uint32 Width = 0;
	uint32 Height = 0;
	uint32 Depth = 1;
	uint32 MipLevels = 1;
	TextureFormat Format = TextureFormat::RGBA8;
};

// GPU buffer handle.
struct GPUBuffer
{
	SDL_GPUBuffer* Handle = nullptr;
	size_t Size = 0;
};

// GPU sampler handle.
struct GPUSampler
{
	SDL_GPUSampler* Handle = nullptr;
};

// GPU shader handle.
struct GPUShader
{
	SDL_GPUShader* Handle = nullptr;
	ShaderStage Stage = ShaderStage::Vertex;
};

// GPU pipeline handle
struct GPUPipeline
{
	SDL_GPUGraphicsPipeline* Handle = nullptr;
};

// Render pass configuration
struct RenderPassConfiguration
{
	f32 ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	f32 ClearDepth = 1.0f;
	uint8 ClearStencil = 0;
	bool ClearColorBuffer = true;
	bool ClearDepthBuffer = true;
	bool ClearStencilBuffer = false;
};

// Pipeline configuration
struct PipelineConfiguration
{
	GPUShader* VertexShader = nullptr;
	GPUShader* FragmentShader = nullptr;
	BlendMode Blend = BlendMode::None;
	CullMode Cull = CullMode::Back;
	FrontFace Face = FrontFace::CounterClockwise;
	CompareFunction DepthCompare = CompareFunction::Less;
	bool DepthWrite = true;
	bool DepthTest = true;
};

// Renderer class
class Renderer
{
public:
	Renderer();
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&& Other) noexcept;
	Renderer& operator=(Renderer&& Other) noexcept;

	/// @brief Initializes the renderer with the given window.
	/// @param Window The SDL window to initialize the renderer with.
	/// @return True if the renderer was successfully initialized, false otherwise.
	bool Init(SDL_Window* Window);

	/// @brief Shuts down the renderer and releases all resources.
	void Shutdown();

	/// @brief Checks if the renderer is initialized.
	/// @return True if the renderer is initialized, false otherwise.
	bool IsInitialized() const { return m_Device != nullptr; }

	/// @brief Begins a new frame.
	/// @return True if the frame was successfully started and the swapchain texture is ready for rendering.
	bool BeginFrame();

	/// @brief Ends the current frame and presents the swapchain texture to the screen.
	void EndFrame();

	/// @brief Begins a render pass to the swapchain texture.
	/// @param Configuration The render pass configuration.
	/// @return True if the render pass was successfully started.
	bool BeginRenderPass(const RenderPassConfiguration& Configuration);

	/// @brief Begins a render pass to a custom render target.
	/// @param ColorTarget The color target texture.
	/// @param DepthTarget The depth target texture.
	/// @param Configuration The render pass configuration.
	/// @return True if the render pass was successfully started.
	bool BeginRenderPass(GPUTexture* ColorTarget, GPUTexture* DepthTarget, const RenderPassConfiguration& Configuration = {});

	/// @brief Ends the current render pass.
	void EndRenderPass();

	/// @brief Binds a graphics pipeline for rendering.
	/// @param Pipeline The pipeline to bind.
	void BindPipeline(GPUPipeline* Pipeline);

	/// @brief Sets the viewport for rendering.
	/// @param X The x-coordinate of the top-left corner of the viewport.
	/// @param Y The y-coordinate of the top-left corner of the viewport.
	/// @param Width The width of the viewport.
	/// @param Height The height of the viewport.
	/// @param MinDepth The minimum depth value of the viewport.
	/// @param MaxDepth The maximum depth value of the viewport.

	void SetViewport(f32 X, f32 Y, f32 Width, f32 Height, f32 MinDepth = 0.0f, f32 MaxDepth = 1.0f);

	/// @brief Sets the scissor rectangle for rendering.
	/// @param X The x-coordinate of the top-left corner of the scissor rectangle.
	/// @param Y The y-coordinate of the top-left corner of the scissor rectangle.
	/// @param Width The width of the scissor rectangle.
	/// @param Height The height of the scissor rectangle.
	void SetScissor(int32 X, int32 Y, uint32 Width, uint32 Height);

	/// @brief Binds a vertex buffer for rendering.
	/// @param Slot The slot to bind the vertex buffer to.
	/// @param Buffer The vertex buffer to bind.
	/// @param Offset The offset in the vertex buffer.
	void BindVertexBuffer(uint32 Slot, GPUBuffer* Buffer, uint32 Offset = 0);

	/// @brief Binds an index buffer for rendering.
	/// @param Buffer The index buffer to bind.
	/// @param Offset The offset in the index buffer.
	/// @param Use16Bit True if the index buffer uses 16-bit indices, false for 32-bit indices.
	void BindIndexBuffer(GPUBuffer* Buffer, uint32 Offset = 0, bool Use16Bit = false);

	/// @brief Binds a texture and sampler for rendering.
	/// @param Slot The slot to bind the texture to.
	/// @param Texture The texture to bind.
	/// @param Sampler The sampler to use with the texture.
	void BindTexture(uint32 Slot, GPUTexture* Texture, GPUSampler* Sampler);

	/// @brief Pushes data to a vertex uniform buffer.
	/// @param Slot The slot to push the data to.
	/// @param Data The data to push.
	/// @param Size The size of the data in bytes.
	void PushVertexUniform(uint32 Slot, const void* Data, uint32 Size);

	/// @brief Pushes data to a fragment uniform buffer.
	/// @param Slot The slot to push the data to.
	/// @param Data The data to push.
	/// @param Size The size of the data in bytes.
	void PushFragmentUniform(uint32 Slot, const void* Data, uint32 Size);

	/// @brief Draws non-indexed geometry.
	/// @param VertexCount The number of vertices to draw.
	/// @param InstanceCount The number of instances to draw.
	/// @param FirstVertex The index of the first vertex to draw.
	/// @param FirstInstance The index of the first instance to draw.
	void Draw(uint32 VertexCount, uint32 InstanceCount = 1, uint32 FirstVertex = 0, uint32 FirstInstance = 0);

	/// @brief Draws indexed geometry.
	/// @param IndexCount The number of indices to draw.
	/// @param InstanceCount The number of instances to draw.
	/// @param FirstIndex The index of the first index to draw.
	/// @param VertexOffset The offset to add to each index.
	/// @param FirstInstance The index of the first instance to draw.
	void DrawIndexed(uint32 IndexCount, uint32 InstanceCount = 1, uint32 FirstIndex = 0, int32 VertexOffset = 0, uint32 FirstInstance = 0);

	/// @brief Creates a shader.
	/// @param Stage The shader stage.
	/// @param Code The shader code.
	/// @param EntryPoint The entry point of the shader.
	/// @param UniformBufferCount The number of uniform buffers.
	/// @param StorageBufferCount The number of storage buffers.
	/// @param StorageTextureCount The number of storage textures.
	/// @param SamplerCount The number of samplers.
	/// @return The created shader.
	GPUShader* CreateShader(
		ShaderStage Stage,
		std::span<const uint8> Code,
		const char* EntryPoint = "main",
		uint32 UniformBufferCount = 0,
		uint32 StorageBufferCount = 0,
		uint32 StorageTextureCount = 0,
		uint32 SamplerCount = 0
	);

	/// @brief Creates a pipeline.
	/// @param Configuration The pipeline configuration.
	/// @param ColorFormat The color format.
	/// @param DepthFormat The depth format.
	/// @return The created pipeline.
	GPUPipeline* CreatePipeline(const PipelineConfiguration& Configuration, SDL_GPUTextureFormat ColorFormat, SDL_GPUTextureFormat DepthFormat = SDL_GPU_TEXTUREFORMAT_INVALID);

	/// @brief Creates a vertex buffer.
	/// @param Size The size of the buffer in bytes.
	/// @param Data The initial data for the buffer.
	/// @return The created vertex buffer.
	GPUBuffer* CreateVertexBuffer(uint32 Size, const void* Data = nullptr);

	/// @brief Creates an index buffer.
	/// @param Size The size of the buffer in bytes.
	/// @param Data The initial data for the buffer.
	/// @return The created index buffer.
	GPUBuffer* CreateIndexBuffer(uint32 Size, const void* Data = nullptr);

	/// @brief Creates a uniform buffer.
	/// @param Size The size of the buffer in bytes.
	/// @return The created uniform buffer.
	GPUBuffer* CreateUniformBuffer(uint32 Size);

	/// @brief Creates a texture.
	/// @param Width The width of the texture.
	/// @param Height The height of the texture.
	/// @param Format The format of the texture.
	/// @param MipLevels The number of mip levels.
	/// @param Data The initial data for the texture.
	/// @return The created texture.
	GPUTexture* CreateTexture(uint32 Width, uint32 Height, TextureFormat Format, uint32 MipLevels = 1, const void* Data = nullptr);

	/// @brief Creates a sampler.
	/// @param Configuration The sampler configuration.
	/// @return The created sampler.
	GPUSampler* CreateSampler(const SamplerConfiguration& Configuration = {});

	/// @brief Destroys a shader.
	/// @param Shader The shader to destroy.
	void DestroyShader(GPUShader* Shader);

	/// @brief Destroys a pipeline.
	/// @param Pipeline The pipeline to destroy.
	void DestroyPipeline(GPUPipeline* Pipeline);

	/// @brief Destroys a buffer.
	/// @param Buffer The buffer to destroy.
	void DestroyBuffer(GPUBuffer* Buffer);

	/// @brief Destroys a texture.
	/// @param Texture The texture to destroy.
	void DestroyTexture(GPUTexture* Texture);

	/// @brief Destroys a sampler.
	/// @param Sampler The sampler to destroy.
	void DestroySampler(GPUSampler* Sampler);

	/// @brief Updates a buffer.
	/// @param Buffer The buffer to update.
	/// @param Data The data to update the buffer with.
	/// @param Size The size of the data in bytes.
	/// @param Offset The offset in the buffer to start updating.
	void UpdateBuffer(GPUBuffer* Buffer, const void* Data, uint32 Size, uint32 Offset = 0);

	/// @brief Updates a texture.
	/// @param Texture The texture to update.
	/// @param Data The data to update the texture with.
	/// @param Size The size of the data in bytes.
	void UpdateTexture(GPUTexture* Texture, const void* Data, uint32 Size);

	/// @brief Gets the format of the swapchain.
	/// @return The format of the swapchain.
	SDL_GPUTextureFormat GetSwapchainFormat() const { return m_SwapchainFormat; }

	/// @brief Gets the GPU device.
	/// @return The GPU device.
	SDL_GPUDevice* GetDevice() const { return m_Device; }

	/// @brief Gets the command buffer for the current frame.
	/// @return The command buffer for the current frame.
	SDL_GPUCommandBuffer* GetCommandBuffer() const { return m_CommandBuffer; }

	/// @brief Gets the current render pass.
	/// @return The current render pass.
	SDL_GPURenderPass* GetRenderPass() const { return m_RenderPass; }

	/// @brief Gets the swapchain texture for the current frame.
	/// @return The swapchain texture for the current frame.
	SDL_Window* GetWindow() const { return m_Window; }

	/// @brief Gets the width of the window.
	/// @return The width of the window.
	uint32 GetWidth() const { return m_Width; }

	/// @brief Gets the height of the window.
	/// @return The height of the window.
	uint32 GetHeight() const { return m_Height; }
private:
	// Pointer to the window
	SDL_Window* 					m_Window 				= nullptr;
	// Pointer to the GPU device
	SDL_GPUDevice* 					m_Device 				= nullptr;
	// Pointer to the command buffer for the current frame
	SDL_GPUCommandBuffer*			m_CommandBuffer 		= nullptr;
	// Pointer to the current render pass
	SDL_GPURenderPass*				m_RenderPass 			= nullptr;
	// Pointer to the swapchain texture for the current frame
	SDL_GPUTexture*					m_SwapchainTexture 		= nullptr;
	// The format of the swapchain texture
	SDL_GPUTextureFormat			m_SwapchainFormat 		= SDL_GPU_TEXTUREFORMAT_INVALID;
	// The width of the window
	uint32							m_Width 				= 0;
	// The height of the window
	uint32							m_Height 				= 0;
	// Whether a frame has been started and the swapchain texture is ready for rendering
	bool							m_FrameStarted 			= false;

	/// @brief Converts the given texture format to the corresponding SDL_GPUTextureFormat.
	/// @param Format The texture format to convert.
	/// @return The corresponding SDL_GPUTextureFormat.
	SDL_GPUTextureFormat		_ConvertFormat(TextureFormat Format) const;

	/// @brief Converts the given texture filter to the corresponding SDL_GPUFilter.
	/// @param Filter The texture filter to convert.
	/// @return The corresponding SDL_GPUFilter.
	SDL_GPUFilter				_ConvertFilter(TextureFilter Filter) const;

	/// @brief Converts the given texture wrap mode to the corresponding SDL_GPUSamplerAddressMode.
	/// @param Wrap The texture wrap mode to convert.
	/// @return The corresponding SDL_GPUSamplerAddressMode.
	SDL_GPUSamplerAddressMode	_ConvertWrap(TextureWrap Wrap) const;

	/// @brief Converts the given blend mode to the corresponding SDL_GPUBlendFactor for the source.
	/// @param Mode The blend mode to convert.
	/// @return The corresponding SDL_GPUBlendFactor for the source.
	SDL_GPUBlendFactor			_GetSourceBlendFactor(BlendMode Mode) const;

	/// @brief Converts the given blend mode to the corresponding SDL_GPUBlendFactor for the destination.
	/// @param Mode The blend mode to convert.
	/// @return The corresponding SDL_GPUBlendFactor for the destination.
	SDL_GPUBlendFactor			_GetDestinationBlendFactor(BlendMode Mode) const;

	/// @brief Converts the given blend mode to the corresponding SDL_GPUBlendOp.
	/// @param Mode The blend mode to convert.
	/// @return The corresponding SDL_GPUBlendOp.
	SDL_GPUBlendOp				_GetBlendOperation(BlendMode Mode) const;

	/// @brief Converts the given cull mode to the corresponding SDL_GPUCullMode.
	/// @param Mode The cull mode to convert.
	/// @return The corresponding SDL_GPUCullMode.
	SDL_GPUCullMode 			_ConvertCullMode(CullMode Mode) const;

	/// @brief Converts the given front face to the corresponding SDL_GPUFrontFace.
	/// @param Face The front face to convert.
	/// @return The corresponding SDL_GPUFrontFace.
	SDL_GPUFrontFace 			_ConvertFrontFace(FrontFace Face) const;

	/// @brief Converts the given compare function to the corresponding SDL_GPUCompareOp.
	/// @param Func The compare function to convert.
	/// @return The corresponding SDL_GPUCompareOp.
	SDL_GPUCompareOp 			_ConvertCompareFunction(CompareFunction Func) const;
};

REDLYNX_NAMESPACE_END_ENGINE
