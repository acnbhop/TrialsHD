//
// renderer.cpp
//

#include "renderer.hpp"

#include <cstring>

REDLYNX_NAMESPACE_BEGIN_ENGINE

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
	Shutdown();
}

Renderer::Renderer(Renderer&& Other) noexcept
	: m_Window(Other.m_Window)
	, m_Device(Other.m_Device)
	, m_CommandBuffer(Other.m_CommandBuffer)
	, m_RenderPass(Other.m_RenderPass)
	, m_SwapchainTexture(Other.m_SwapchainTexture)
	, m_SwapchainFormat(Other.m_SwapchainFormat)
	, m_Width(Other.m_Width)
	, m_Height(Other.m_Height)
	, m_FrameStarted(Other.m_FrameStarted)
{
	Other.m_Window = nullptr;
	Other.m_Device = nullptr;
	Other.m_CommandBuffer = nullptr;
	Other.m_RenderPass = nullptr;
	Other.m_SwapchainTexture = nullptr;
	Other.m_FrameStarted = false;
}

Renderer& Renderer::operator=(Renderer&& Other) noexcept
{
	if (this != &Other)
	{
		Shutdown();

		m_Window = Other.m_Window;
		m_Device = Other.m_Device;
		m_CommandBuffer = Other.m_CommandBuffer;
		m_RenderPass = Other.m_RenderPass;
		m_SwapchainTexture = Other.m_SwapchainTexture;
		m_SwapchainFormat = Other.m_SwapchainFormat;
		m_Width = Other.m_Width;
		m_Height = Other.m_Height;
		m_FrameStarted = Other.m_FrameStarted;

		Other.m_Window = nullptr;
		Other.m_Device = nullptr;
		Other.m_CommandBuffer = nullptr;
		Other.m_RenderPass = nullptr;
		Other.m_SwapchainTexture = nullptr;
		Other.m_FrameStarted = false;
	}

	return* this;
}

/// @brief Initializes the renderer with the given window.
/// @param Window The SDL window to initialize the renderer with.
/// @return True if the renderer was successfully initialized, false otherwise.
bool Renderer::Init(SDL_Window* Window)
{

}

/// @brief Shuts down the renderer and releases all resources.
void Renderer::Shutdown()
{

}

/// @brief Begins a new frame.
/// @return True if the frame was successfully started and the swapchain texture is ready for rendering.
bool Renderer::BeginFrame()
{

}

/// @brief Ends the current frame and presents the swapchain texture to the screen.
void Renderer::EndFrame()
{

}

/// @brief Begins a render pass to the swapchain texture.
/// @param Configuration The render pass configuration.
/// @return True if the render pass was successfully started.
bool Renderer::BeginRenderPass(const RenderPassConfiguration& Configuration)
{

}
	
/// @brief Begins a render pass to a custom render target.
/// @param ColorTarget The color target texture.
/// @param DepthTarget The depth target texture.
/// @param Configuration The render pass configuration.
/// @return True if the render pass was successfully started.
bool Renderer::BeginRenderPass(GPUTexture* ColorTarget, GPUTexture* DepthTarget, const RenderPassConfiguration& Configuration = {})
{

}

/// @brief Ends the current render pass.
void Renderer::EndRenderPass()
{

}

/// @brief Binds a graphics pipeline for rendering.
/// @param Pipeline The pipeline to bind.
void Renderer::BindPipeline(GPUPipeline* Pipeline)
{

}

/// @brief Sets the viewport for rendering.
/// @param X The x-coordinate of the top-left corner of the viewport.
/// @param Y The y-coordinate of the top-left corner of the viewport.
/// @param Width The width of the viewport.
/// @param Height The height of the viewport.
/// @param MinDepth The minimum depth value of the viewport.
/// @param MaxDepth The maximum depth value of the viewport.
void Renderer::SetViewport(f32 X, f32 Y, f32 Width, f32 Height, f32 MinDepth = 0.0f, f32 MaxDepth = 1.0f)
{

}

/// @brief Sets the scissor rectangle for rendering.
/// @param X The x-coordinate of the top-left corner of the scissor rectangle.
/// @param Y The y-coordinate of the top-left corner of the scissor rectangle.
/// @param Width The width of the scissor rectangle.
/// @param Height The height of the scissor rectangle.
void Renderer::SetScissor(int32 X, int32 Y, uint32 Width, uint32 Height)
{

}

/// @brief Binds a vertex buffer for rendering.
/// @param Slot The slot to bind the vertex buffer to.
/// @param Buffer The vertex buffer to bind.
/// @param Offset The offset in the vertex buffer.
void Renderer::BindVertexBuffer(uint32 Slot, GPUBuffer* Buffer, uint32 Offset = 0)
{

}

/// @brief Binds an index buffer for rendering.
/// @param Buffer The index buffer to bind.
/// @param Offset The offset in the index buffer.
/// @param Use16Bit True if the index buffer uses 16-bit indices, false for 32-bit indices.
void Renderer::BindIndexBuffer(GPUBuffer* Buffer, uint32 Offset = 0, bool Use16Bit = false)
{

}

/// @brief Binds a texture and sampler for rendering.
/// @param Slot The slot to bind the texture to.
/// @param Texture The texture to bind.
/// @param Sampler The sampler to use with the texture.
void Renderer::BindTexture(uint32 Slot, GPUTexture* Texture, GPUSampler* Sampler)
{

}

/// @brief Pushes data to a vertex uniform buffer.
/// @param Slot The slot to push the data to.
/// @param Data The data to push.
/// @param Size The size of the data in bytes.
void Renderer::PushVertexUniform(uint32 Slot, const void* Data, uint32 Size)
{

}

/// @brief Pushes data to a fragment uniform buffer.
/// @param Slot The slot to push the data to.
/// @param Data The data to push.
/// @param Size The size of the data in bytes.
void Renderer::PushFragmentUniform(uint32 Slot, const void* Data, uint32 Size)
{

}

/// @brief Draws non-indexed geometry.
/// @param VertexCount The number of vertices to draw.
/// @param InstanceCount The number of instances to draw.
/// @param FirstVertex The index of the first vertex to draw.
/// @param FirstInstance The index of the first instance to draw.
void Renderer::Draw(uint32 VertexCount, uint32 InstanceCount = 1, uint32 FirstVertex = 0, uint32 FirstInstance = 0)
{

}

/// @brief Draws indexed geometry.
/// @param IndexCount The number of indices to draw.
/// @param InstanceCount The number of instances to draw.
/// @param FirstIndex The index of the first index to draw.
/// @param VertexOffset The offset to add to each index.
/// @param FirstInstance The index of the first instance to draw.
void Renderer::DrawIndexed(uint32 IndexCount, uint32 InstanceCount = 1, uint32 FirstIndex = 0, int32 VertexOffset = 0, uint32 FirstInstance = 0)
{

}

/// @brief Creates a shader.
/// @param Stage The shader stage.
/// @param Code The shader code.
/// @param EntryPoint The entry point of the shader.
/// @param UniformBufferCount The number of uniform buffers.
/// @param StorageBufferCount The number of storage buffers.
/// @param StorageTextureCount The number of storage textures.
/// @param SamplerCount The number of samplers.
/// @return The created shader.
GPUShader* Renderer::CreateShader(ShaderStage Stage, std::span<const uint8> Code, const char* EntryPoint = "main", uint32 UniformBufferCount = 0, uint32 StorageBufferCount = 0, uint32 StorageTextureCount = 0, uint32 SamplerCount = 0)
{

}

/// @brief Creates a pipeline.
/// @param Configuration The pipeline configuration.
/// @param ColorFormat The color format.
/// @param DepthFormat The depth format.
/// @return The created pipeline.
GPUPipeline* Renderer::CreatePipeline(const PipelineConfiguration& Configuration, SDL_GPUTextureFormat ColorFormat, SDL_GPUTextureFormat DepthFormat = SDL_GPU_TEXTUREFORMAT_INVALID)
{

}

/// @brief Creates a vertex buffer.
/// @param Size The size of the buffer in bytes.
/// @param Data The initial data for the buffer.
/// @return The created vertex buffer.
GPUBuffer* Renderer::CreateVertexBuffer(uint32 Size, const void* Data = nullptr)
{

}
	
/// @brief Creates an index buffer.
/// @param Size The size of the buffer in bytes.
/// @param Data The initial data for the buffer.
/// @return The created index buffer.
GPUBuffer* Renderer::CreateIndexBuffer(uint32 Size, const void* Data = nullptr)
{

}

/// @brief Creates a uniform buffer.
/// @param Size The size of the buffer in bytes.
/// @return The created uniform buffer.
GPUBuffer* Renderer::CreateUniformBuffer(uint32 Size)
{

}

/// @brief Creates a texture.
/// @param Width The width of the texture.
/// @param Height The height of the texture.
/// @param Format The format of the texture.
/// @param MipLevels The number of mip levels.
/// @param Data The initial data for the texture.
/// @return The created texture.
GPUTexture* Renderer::CreateTexture(uint32 Width, uint32 Height, TextureFormat Format, uint32 MipLevels = 1, const void* Data = nullptr)
{

}

/// @brief Creates a sampler.
/// @param Configuration The sampler configuration.
/// @return The created sampler.
GPUSampler* Renderer::CreateSampler(const SamplerConfiguration& Configuration = {})
{

}

/// @brief Destroys a shader.
/// @param Shader The shader to destroy.
void Renderer::DestroyShader(GPUShader* Shader)
{

}

/// @brief Destroys a pipeline.
/// @param Pipeline The pipeline to destroy.
void Renderer::DestroyPipeline(GPUPipeline* Pipeline)
{

}

/// @brief Destroys a buffer.
/// @param Buffer The buffer to destroy.
void Renderer::DestroyBuffer(GPUBuffer* Buffer)
{

}

/// @brief Destroys a texture.
/// @param Texture The texture to destroy.
void Renderer::DestroyTexture(GPUTexture* Texture)
{

}

/// @brief Destroys a sampler.
/// @param Sampler The sampler to destroy.
void Renderer::DestroySampler(GPUSampler* Sampler)
{

}

/// @brief Updates a buffer.
/// @param Buffer The buffer to update.
/// @param Data The data to update the buffer with.
/// @param Size The size of the data in bytes.
/// @param Offset The offset in the buffer to start updating.
void Renderer::UpdateBuffer(GPUBuffer* Buffer, const void* Data, uint32 Size, uint32 Offset = 0)
{

}

/// @brief Updates a texture.
/// @param Texture The texture to update.
/// @param Data The data to update the texture with.
/// @param Size The size of the data in bytes.
void Renderer::UpdateTexture(GPUTexture* Texture, const void* Data, uint32 Size)
{
	
}

REDLYNX_NAMESPACE_END_ENGINE
