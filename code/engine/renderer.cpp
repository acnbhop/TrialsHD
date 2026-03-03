//
// renderer.cpp
//

#include "renderer.hpp"

REDLYNX_NAMESPACE_BEGIN_ENGINE

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Converts the given texture format to the corresponding SDL_GPUTextureFormat.
/// @param Format The texture format to convert.
/// @return The corresponding SDL_GPUTextureFormat.
SDL_GPUTextureFormat		Renderer::_ConvertFormat(TextureFormat Format) const
{
	switch (Format)
	{
		case TextureFormat::RGBA8:
			return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
		case TextureFormat::BGRA8:
			return SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
		case TextureFormat::R8:
			return SDL_GPU_TEXTUREFORMAT_R8_UNORM;
		case TextureFormat::Depth24Stencil8:
			return SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
		case TextureFormat::Depth32:
			return SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
		default:
			return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	}
}

/// @brief Converts the given texture filter to the corresponding SDL_GPUFilter.
/// @param Filter The texture filter to convert.
/// @return The corresponding SDL_GPUFilter.
SDL_GPUFilter				Renderer::_ConvertFilter(TextureFilter Filter) const
{
	switch (Filter)
	{
		case TextureFilter::Nearest:
			return SDL_GPU_FILTER_NEAREST;
		case TextureFilter::Linear:
			return SDL_GPU_FILTER_LINEAR;
		default:
			return SDL_GPU_FILTER_LINEAR;
	}
}

/// @brief Converts the given texture wrap mode to the corresponding SDL_GPUSamplerAddressMode.
/// @param Wrap The texture wrap mode to convert.
/// @return The corresponding SDL_GPUSamplerAddressMode.
SDL_GPUSamplerAddressMode	Renderer::_ConvertWrap(TextureWrap Wrap) const
{
	switch (Wrap)
	{
		case TextureWrap::Repeat:
			return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
		case TextureWrap::ClampToEdge:
			return SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
		case TextureWrap::MirroredRepeat:
			return SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT;
		default:
			return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
	}
}

/// @brief Converts the given blend mode to the corresponding SDL_GPUBlendFactor for the source.
/// @param Mode The blend mode to convert.
/// @return The corresponding SDL_GPUBlendFactor for the source.
SDL_GPUBlendFactor			Renderer::_GetSourceBlendFactor(BlendMode Mode) const
{
	switch (Mode)
	{
		case BlendMode::Alpha:
			return SDL_GPU_BLENDFACTOR_SRC_ALPHA;
		case BlendMode::Additive:
			return SDL_GPU_BLENDFACTOR_SRC_ALPHA;
		case BlendMode::Multiply:
			return SDL_GPU_BLENDFACTOR_DST_COLOR;
		default:
			return SDL_GPU_BLENDFACTOR_ONE;
	}
}

/// @brief Converts the given blend mode to the corresponding SDL_GPUBlendFactor for the destination.
/// @param Mode The blend mode to convert.
/// @return The corresponding SDL_GPUBlendFactor for the destination.
SDL_GPUBlendFactor			Renderer::_GetDestinationBlendFactor(BlendMode Mode) const
{
	switch (Mode)
	{
		case BlendMode::Alpha:
			return SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		case BlendMode::Additive:
			return SDL_GPU_BLENDFACTOR_ONE;
		case BlendMode::Multiply:
			return SDL_GPU_BLENDFACTOR_ZERO;
		default:
			return SDL_GPU_BLENDFACTOR_ZERO;
	}
}

/// @brief Converts the given blend mode to the corresponding SDL_GPUBlendOp.
/// @param Mode The blend mode to convert.
/// @return The corresponding SDL_GPUBlendOp.
SDL_GPUBlendOp				Renderer::_GetBlendOperation(BlendMode Mode) const
{
	(void)Mode;
	return SDL_GPU_BLENDOP_ADD;
}

/// @brief Converts the given cull mode to the corresponding SDL_GPUCullMode.
/// @param Mode The cull mode to convert.
/// @return The corresponding SDL_GPUCullMode.
SDL_GPUCullMode 			Renderer::_ConvertCullMode(CullMode Mode) const
{
	switch (Mode)
	{
		case CullMode::None:
			return SDL_GPU_CULLMODE_NONE;
		case CullMode::Front:
			return SDL_GPU_CULLMODE_FRONT;
		case CullMode::Back:
			return SDL_GPU_CULLMODE_BACK;
		default:
			return SDL_GPU_CULLMODE_NONE;
	}
}

/// @brief Converts the given front face to the corresponding SDL_GPUFrontFace.
/// @param Face The front face to convert.
/// @return The corresponding SDL_GPUFrontFace.
SDL_GPUFrontFace 			Renderer::_ConvertFrontFace(FrontFace Face) const
{
	switch (Face)
	{
		case FrontFace::CounterClockwise:
			return SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
		case FrontFace::Clockwise:
			return SDL_GPU_FRONTFACE_CLOCKWISE;
		default:
			return SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
	}
}

/// @brief Converts the given compare function to the corresponding SDL_GPUCompareOp.
/// @param Func The compare function to convert.
/// @return The corresponding SDL_GPUCompareOp.
SDL_GPUCompareOp 			Renderer::_ConvertCompareFunction(CompareFunction Func) const
{
	switch (Func)
	{
		case CompareFunction::Never:
			return SDL_GPU_COMPAREOP_NEVER;
		case CompareFunction::Less:
			return SDL_GPU_COMPAREOP_LESS;
		case CompareFunction::Equal:
			return SDL_GPU_COMPAREOP_EQUAL;
		case CompareFunction::LessOrEqual:
			return SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
		case CompareFunction::Greater:
			return SDL_GPU_COMPAREOP_GREATER;
		case CompareFunction::NotEqual:
			return SDL_GPU_COMPAREOP_NOT_EQUAL;
		case CompareFunction::GreaterOrEqual:
			return SDL_GPU_COMPAREOP_GREATER_OR_EQUAL;
		case CompareFunction::Always:
			return SDL_GPU_COMPAREOP_ALWAYS;
		default:
			return SDL_GPU_COMPAREOP_LESS;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(REDLYNX_NO_REDEFINE_EMPTY_CONSTRUCTOR)
Renderer::Renderer()
{
}
#endif

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
	if (!Window)
	{
		return false;
	}

	m_Window = Window;

	// Create GPU device with support for the common shader formats.
	m_Device = SDL_CreateGPUDevice(
		SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL | SDL_GPU_SHADERFORMAT_METALLIB,
	//
	// Enable debug mode in development builds.
	//
#if BUILD_DEV
		true,
#else
		false,
#endif
		nullptr		// Preferred driver. Will automatically set to best available if nullptr.
	);

	if (!m_Device)
	{
		SDL_Log("Error: SDL_CreateGPUDevice() failed: %s", SDL_GetError());
		return false;
	}

	// Claim the window for GPU rendering.
	if (!SDL_ClaimWindowForGPUDevice(m_Device, m_Window))
	{
		SDL_Log("Error: SDL_ClaimWindowForGPUDevice() failed: %s", SDL_GetError());
		SDL_DestroyGPUDevice(m_Device);
		m_Device = nullptr;
		return false;
	}

	// Set swapchain parameters.
	SDL_SetGPUSwapchainParameters(m_Device, m_Window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

	// Get swapchain format.
	m_SwapchainFormat = SDL_GetGPUSwapchainTextureFormat(m_Device, m_Window);

	//
	// Get window size.
	//

	int WindowWidth = 0, WindowHeight = 0;
	SDL_GetWindowSize(m_Window, &WindowWidth, &WindowHeight);
	m_Width			= static_cast<uint32>(WindowWidth);
	m_Height		= static_cast<uint32>(WindowHeight);

	return true;
}

/// @brief Shuts down the renderer and releases all resources.
void Renderer::Shutdown()
{
	if (m_Device)
	{
		SDL_WaitForGPUIdle(m_Device);

		if (m_Window)
		{
			SDL_ReleaseWindowFromGPUDevice(m_Device, m_Window);
		}

		SDL_DestroyGPUDevice(m_Device);
		m_Device = nullptr;
	}

	m_Window				= nullptr;
	m_CommandBuffer			= nullptr;
	m_RenderPass			= nullptr;
	m_SwapchainTexture		= nullptr;
	m_FrameStarted			= false;
}

/// @brief Begins a new frame.
/// @return True if the frame was successfully started and the swapchain texture is ready for rendering.
bool Renderer::BeginFrame()
{
	if (!m_Device || !m_Window)
	{
		SDL_Log("[Renderer] BeginFrame() called but renderer is not initialized!");
		return false;
	}

	// Update window size.

	int WindowWidth = 0, WindowHeight = 0;
	SDL_GetWindowSize(m_Window, &WindowWidth, &WindowHeight);
	m_Width			= static_cast<uint32>(WindowWidth);
	m_Height		= static_cast<uint32>(WindowHeight);

	// Acquire a command buffer.
	m_CommandBuffer = SDL_AcquireGPUCommandBuffer(m_Device);
	if (!m_CommandBuffer)
	{
		SDL_Log("[Renderer] [Error] Failed to acquire command buffer: %s", SDL_GetError());
		return false;
	}

	// Acquire swapchain texture.
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(m_CommandBuffer, m_Window, &m_SwapchainTexture, nullptr, nullptr))
	{
		SDL_Log("[Renderer] [Error] Failed to acquire swapchain texture: %s", SDL_GetError());

		// Even if we fail we must properly release the command buffer.
		SDL_CancelGPUCommandBuffer(m_CommandBuffer);

		m_CommandBuffer = nullptr;
		return false;
	}

	// Frame has started.
	m_FrameStarted = true;

	return m_SwapchainTexture != nullptr;
}

/// @brief Ends the current frame and presents the swapchain texture to the screen.
void Renderer::EndFrame()
{
	if (!m_FrameStarted || !m_CommandBuffer)
	{
		if (!m_FrameStarted)
		{
			SDL_Log("[Renderer] EndFrame() called but no frame has been started!");
		}
		else if (!m_CommandBuffer)
		{
			SDL_Log("[Renderer] EndFrame() called but command buffer is null!");
		}
	}

	// End any active render pass happening right now.
	if (m_RenderPass)
	{
		SDL_EndGPURenderPass(m_RenderPass);
		m_RenderPass = nullptr;
	}

	// Submit command buffer.
	SDL_SubmitGPUCommandBuffer(m_CommandBuffer);

	m_CommandBuffer = nullptr;
	m_SwapchainTexture = nullptr;
	m_FrameStarted = false;
}

/// @brief Begins a render pass to the swapchain texture.
/// @param Configuration The render pass configuration.
/// @return True if the render pass was successfully started.
bool Renderer::BeginRenderPass(const RenderPassConfiguration& Configuration)
{
	if (!m_FrameStarted || !m_CommandBuffer || m_RenderPass)
	{
		if (!m_FrameStarted)
		{
			SDL_Log("[Renderer] BeginRenderPass() called but no frame has been started!");
		}
		else if (!m_CommandBuffer)
		{
			SDL_Log("[Renderer] BeginRenderPass() called but command buffer is null!");
		}
		else if (m_RenderPass)
		{
			SDL_Log("[Renderer] BeginRenderPass() called but a render pass is already active!");
		}

		return false;
	}

	SDL_GPUColorTargetInfo ColorTarget = {};

	ColorTarget.texture 				= m_SwapchainTexture;
	ColorTarget.clear_color				= SDL_FColor { Configuration.ClearColor[0], Configuration.ClearColor[1], Configuration.ClearColor[2], Configuration.ClearColor[3] };
	ColorTarget.load_op					= Configuration.ClearColorBuffer ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
	ColorTarget.store_op				= SDL_GPU_STOREOP_STORE;
	ColorTarget.mip_level				= 0;
	ColorTarget.layer_or_depth_plane	= 0;
	ColorTarget.cycle					= false;

	m_RenderPass = SDL_BeginGPURenderPass(m_CommandBuffer, &ColorTarget, 1, nullptr);
	return m_RenderPass != nullptr;
}

/// @brief Begins a render pass to a custom render target.
/// @param ColorTarget The color target texture.
/// @param DepthTarget The depth target texture.
/// @param Configuration The render pass configuration.
/// @return True if the render pass was successfully started.
bool Renderer::BeginRenderPass(GPUTexture* ColorTarget, GPUTexture* DepthTarget, const RenderPassConfiguration& Configuration /* = {} */)
{
	if (!m_FrameStarted || m_RenderPass)
	{
		if (!m_FrameStarted)
		{
			SDL_Log("[Renderer] BeginRenderPass() called but no frame has been started!");
		}
		else if (m_RenderPass)
		{
			SDL_Log("[Renderer] BeginRenderPass() called but a render pass is already active!");
		}

		return false;
	}

	SDL_GPUColorTargetInfo ColorInfo = {};
	SDL_GPUDepthStencilTargetInfo DepthInfo = {};
	uint32 ColorTargetCount = 0;
	SDL_GPUDepthStencilTargetInfo* DepthInfoPointer = nullptr;

	if (ColorTarget && ColorTarget->Handle)
	{
		ColorInfo.texture				= ColorTarget->Handle;
		ColorInfo.clear_color			= SDL_FColor { Configuration.ClearColor[0], Configuration.ClearColor[1], Configuration.ClearColor[2], Configuration.ClearColor[3] };
		ColorInfo.load_op				= Configuration.ClearColorBuffer ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
		ColorInfo.store_op				= SDL_GPU_STOREOP_STORE;
		ColorInfo.mip_level				= 0;
		ColorInfo.layer_or_depth_plane	= 0;
		ColorInfo.cycle					= false;
		ColorTargetCount				= 1;
	}

	if (DepthTarget && DepthTarget->Handle)
	{
		DepthInfo.texture				= DepthTarget->Handle;
		DepthInfo.clear_depth			= Configuration.ClearDepth;
		DepthInfo.clear_stencil			= Configuration.ClearStencil;
		DepthInfo.load_op				= Configuration.ClearDepthBuffer ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
		DepthInfo.store_op				= SDL_GPU_STOREOP_STORE;
		DepthInfo.stencil_load_op		= Configuration.ClearStencilBuffer ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
		DepthInfo.stencil_store_op		= SDL_GPU_STOREOP_STORE;
		DepthInfo.cycle					= false;
		DepthInfoPointer				= &DepthInfo;
	}

	//
	// Second parameter works like this:
	//		- If ColorTarget is valid, we set up ColorInfo and pass a pointer to it with count 1.
	//		- If ColorTarget is null, we pass nullptr with count 0.
	//		- If DepthTarget is valid, we set up DepthInfo and pass a pointer to it.
	//		- If DepthTarget is null, we pass nullptr.
	//
	m_RenderPass = SDL_BeginGPURenderPass(m_CommandBuffer, ColorTargetCount > 0 ? &ColorInfo : nullptr, ColorTargetCount, DepthInfoPointer);
	return m_RenderPass != nullptr;
}

/// @brief Ends the current render pass.
void Renderer::EndRenderPass()
{
	if (m_RenderPass)
	{
		SDL_EndGPURenderPass(m_RenderPass);
		m_RenderPass = nullptr;
	}
}

/// @brief Binds a graphics pipeline for rendering.
/// @param Pipeline The pipeline to bind.
void Renderer::BindPipeline(GPUPipeline* Pipeline)
{
	if (m_RenderPass && Pipeline && Pipeline->Handle)
	{
		SDL_BindGPUGraphicsPipeline(m_RenderPass, Pipeline->Handle);
	}
}

/// @brief Sets the viewport for rendering.
/// @param X The x-coordinate of the top-left corner of the viewport.
/// @param Y The y-coordinate of the top-left corner of the viewport.
/// @param Width The width of the viewport.
/// @param Height The height of the viewport.
/// @param MinDepth The minimum depth value of the viewport.
/// @param MaxDepth The maximum depth value of the viewport.
void Renderer::SetViewport(f32 X, f32 Y, f32 Width, f32 Height, f32 MinDepth /* = 0.0f */, f32 MaxDepth /* = 1.0f */)
{
	if (m_RenderPass)
	{
		SDL_GPUViewport Viewport = {};

		Viewport.x = X;
		Viewport.y = Y;
		Viewport.w = Width;
		Viewport.h = Height;
		Viewport.min_depth = MinDepth;
		Viewport.max_depth = MaxDepth;

		SDL_SetGPUViewport(m_RenderPass, &Viewport);
	}
}

/// @brief Sets the scissor rectangle for rendering.
/// @param X The x-coordinate of the top-left corner of the scissor rectangle.
/// @param Y The y-coordinate of the top-left corner of the scissor rectangle.
/// @param Width The width of the scissor rectangle.
/// @param Height The height of the scissor rectangle.
void Renderer::SetScissor(int32 X, int32 Y, uint32 Width, uint32 Height)
{
	if (m_RenderPass)
	{
		SDL_Rect Scissor = {};

		Scissor.x = X;
		Scissor.y = Y;
		Scissor.w = static_cast<int>(Width);
		Scissor.h = static_cast<int>(Height);

		SDL_SetGPUScissor(m_RenderPass, &Scissor);
	}
}

/// @brief Binds a vertex buffer for rendering.
/// @param Slot The slot to bind the vertex buffer to.
/// @param Buffer The vertex buffer to bind.
/// @param Offset The offset in the vertex buffer.
void Renderer::BindVertexBuffer(uint32 Slot, GPUBuffer* Buffer, uint32 Offset /* = 0 */)
{
	if (m_RenderPass && Buffer && Buffer->Handle)
	{
		SDL_GPUBufferBinding Binding = {};

		Binding.buffer = Buffer->Handle;
		Binding.offset = Offset;

		SDL_BindGPUVertexBuffers(m_RenderPass, Slot, &Binding, 1);
	}
}

/// @brief Binds an index buffer for rendering.
/// @param Buffer The index buffer to bind.
/// @param Offset The offset in the index buffer.
/// @param Use16Bit True if the index buffer uses 16-bit indices, false for 32-bit indices.
void Renderer::BindIndexBuffer(GPUBuffer* Buffer, uint32 Offset /* = 0 */, bool Use16Bit /* = false */)
{
	if (m_RenderPass && Buffer && Buffer->Handle)
	{
		SDL_GPUBufferBinding Binding = {};

		Binding.buffer = Buffer->Handle;
		Binding.offset = Offset;

		SDL_BindGPUIndexBuffer(m_RenderPass, &Binding, Use16Bit ? SDL_GPU_INDEXELEMENTSIZE_16BIT : SDL_GPU_INDEXELEMENTSIZE_32BIT);
	}
}

/// @brief Binds a texture and sampler for rendering.
/// @param Slot The slot to bind the texture to.
/// @param Texture The texture to bind.
/// @param Sampler The sampler to use with the texture.
void Renderer::BindTexture(uint32 Slot, GPUTexture* Texture, GPUSampler* Sampler)
{
	if (m_RenderPass && Texture && Texture->Handle && Sampler && Sampler->Handle)
	{
		SDL_GPUTextureSamplerBinding Binding = {};

		Binding.texture = Texture->Handle;
		Binding.sampler = Sampler->Handle;

		SDL_BindGPUFragmentSamplers(m_RenderPass, Slot, &Binding, 1);
	}
}

/// @brief Pushes data to a vertex uniform buffer.
/// @param Slot The slot to push the data to.
/// @param Data The data to push.
/// @param Size The size of the data in bytes.
void Renderer::PushVertexUniform(uint32 Slot, const void* Data, uint32 Size)
{
	if (m_CommandBuffer && Data && Size > 0)
	{
		SDL_PushGPUVertexUniformData(m_CommandBuffer, Slot, Data, Size);
	}
}

/// @brief Pushes data to a fragment uniform buffer.
/// @param Slot The slot to push the data to.
/// @param Data The data to push.
/// @param Size The size of the data in bytes.
void Renderer::PushFragmentUniform(uint32 Slot, const void* Data, uint32 Size)
{
	if (m_CommandBuffer && Data && Size > 0)
	{
		SDL_PushGPUFragmentUniformData(m_CommandBuffer, Slot, Data, Size);
	}
}

/// @brief Draws non-indexed geometry.
/// @param VertexCount The number of vertices to draw.
/// @param InstanceCount The number of instances to draw.
/// @param FirstVertex The index of the first vertex to draw.
/// @param FirstInstance The index of the first instance to draw.
void Renderer::Draw(uint32 VertexCount, uint32 InstanceCount /* = 1 */, uint32 FirstVertex /* = 0 */, uint32 FirstInstance /* = 0 */)
{
	if (m_RenderPass)
	{
		SDL_DrawGPUPrimitives(m_RenderPass, VertexCount, InstanceCount, FirstVertex, FirstInstance);
	}
}

/// @brief Draws indexed geometry.
/// @param IndexCount The number of indices to draw.
/// @param InstanceCount The number of instances to draw.
/// @param FirstIndex The index of the first index to draw.
/// @param VertexOffset The offset to add to each index.
/// @param FirstInstance The index of the first instance to draw.
void Renderer::DrawIndexed(uint32 IndexCount, uint32 InstanceCount /* = 1 */, uint32 FirstIndex /* = 0 */, int32 VertexOffset /* = 0 */, uint32 FirstInstance /* = 0 */)
{
	if (m_RenderPass)
	{
		SDL_DrawGPUIndexedPrimitives(m_RenderPass, IndexCount, InstanceCount, FirstIndex, VertexOffset, FirstInstance);
	}
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
GPUShader* Renderer::CreateShader(ShaderStage Stage, std::span<const uint8> Code, const char* EntryPoint /* = "main" */, uint32 UniformBufferCount /* = 0 */, uint32 StorageBufferCount /* = 0 */, uint32 StorageTextureCount /* = 0 */, uint32 SamplerCount /* = 0 */)
{
	if (!m_Device || Code.empty())
	{
		if (!m_Device)
		{
			SDL_Log("[Renderer] CreateShader() called but renderer is not initialized!");

			if (Code.empty())
			{
				SDL_Log("[Renderer] The shader code is also empty!");
			}
		}
		else if (Code.empty())
		{
			SDL_Log("[Renderer] CreateShader() called but shader code is empty!");
		}

		return nullptr;
	}

	SDL_GPUShaderCreateInfo Info = {};

	Info.code = Code.data();
	Info.code_size = Code.size();
	Info.entrypoint = EntryPoint;
	Info.format = SDL_GPU_SHADERFORMAT_SPIRV;
	Info.stage = (Stage == ShaderStage::Vertex) ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT;
	Info.num_uniform_buffers = UniformBufferCount;
	Info.num_storage_buffers = StorageBufferCount;
	Info.num_storage_textures = StorageTextureCount;
	Info.num_samplers = SamplerCount;

	SDL_GPUShader* ShaderHandle = SDL_CreateGPUShader(m_Device, &Info);
	if (!ShaderHandle)
	{
		SDL_Log("[Renderer] Failed to create shader: %s", SDL_GetError());
		return nullptr;
	}

	GPUShader* Shader = new GPUShader();

	Shader->Handle = ShaderHandle;
	Shader->Stage = Stage;

	return Shader;
}

/// @brief Creates a pipeline.
/// @param Configuration The pipeline configuration.
/// @param ColorFormat The color format.
/// @param DepthFormat The depth format.
/// @return The created pipeline.
GPUPipeline* Renderer::CreatePipeline(const PipelineConfiguration& Configuration, SDL_GPUTextureFormat ColorFormat, SDL_GPUTextureFormat DepthFormat /* = SDL_GPU_TEXTUREFORMAT_INVALID */)
{
	if (!m_Device || !Configuration.VertexShader || !Configuration.FragmentShader)
	{
		if (!m_Device)
		{
			SDL_Log("[Renderer] CreatePipeline() called but renderer is not initialized!");

			if (!Configuration.VertexShader)
			{
				SDL_Log("[Renderer] Vertex shader is null!");
			}

			if (!Configuration.FragmentShader)
			{
				SDL_Log("[Renderer] Fragment shader is null!");
			}
		}
		else
		{
			if (!Configuration.VertexShader)
			{
				SDL_Log("[Renderer] CreatePipeline() called but vertex shader is null!");
			}

			if (!Configuration.FragmentShader)
			{
				SDL_Log("[Renderer] CreatePipeline() called but fragment shader is null!");
			}
		}

		return nullptr;
	}

	SDL_GPUGraphicsPipelineCreateInfo Info = {};

	Info.vertex_shader = Configuration.VertexShader->Handle;
	Info.fragment_shader = Configuration.FragmentShader->Handle;

	SDL_GPUVertexBufferDescription VertexBufferDescription = {};

	VertexBufferDescription.slot = 0;
	VertexBufferDescription.pitch = sizeof(Vertex3D);
	VertexBufferDescription.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	VertexBufferDescription.instance_step_rate = 0;

	SDL_GPUVertexAttribute VertexAttributes[4] = {};

	//
	// Position
	//
	VertexAttributes[0].location = 0;
	VertexAttributes[0].buffer_slot = 0;
	VertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	VertexAttributes[0].offset = offsetof(Vertex3D, Position);

	//
	// Normal
	//
	VertexAttributes[1].location = 1;
	VertexAttributes[1].buffer_slot = 0;
	VertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	VertexAttributes[1].offset = offsetof(Vertex3D, Normal);

	//
	// TexCoord
	//
	VertexAttributes[2].location = 2;
	VertexAttributes[2].buffer_slot = 0;
	VertexAttributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	VertexAttributes[2].offset = offsetof(Vertex3D, TexCoord);

	//
	// Color
	//
	VertexAttributes[3].location = 3;
	VertexAttributes[3].buffer_slot = 0;
	VertexAttributes[3].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	VertexAttributes[3].offset = offsetof(Vertex3D, Color);

	Info.vertex_input_state.vertex_buffer_descriptions = &VertexBufferDescription;
	Info.vertex_input_state.num_vertex_buffers = 1;
	Info.vertex_input_state.vertex_attributes = VertexAttributes;
	Info.vertex_input_state.num_vertex_attributes = 4;

	// Primitive type
	Info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

	// Rasterizer state
	Info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	Info.rasterizer_state.cull_mode = _ConvertCullMode(Configuration.Cull);
	Info.rasterizer_state.front_face = _ConvertFrontFace(Configuration.Face);
	Info.rasterizer_state.depth_bias_constant_factor = 0.0f;
	Info.rasterizer_state.depth_bias_clamp = 0.0f;
	Info.rasterizer_state.depth_bias_slope_factor = 0.0f;
	Info.rasterizer_state.enable_depth_bias = false;
	Info.rasterizer_state.enable_depth_clip = true;

	// Multisample state
	Info.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
	Info.multisample_state.sample_mask = 0xFFFFFFFF;
	Info.multisample_state.enable_mask = false;

	// Depth stencil state
	Info.depth_stencil_state.compare_op = _ConvertCompareFunction(Configuration.DepthCompare);
	Info.depth_stencil_state.enable_depth_test = Configuration.DepthTest;
	Info.depth_stencil_state.enable_depth_write = Configuration.DepthWrite;
	Info.depth_stencil_state.enable_stencil_test = false;

	// Color target
	SDL_GPUColorTargetDescription ColorTargetDescription = {};
	ColorTargetDescription.format = ColorFormat;

	if (Configuration.Blend != BlendMode::None)
	{
		ColorTargetDescription.blend_state.enable_blend = true;
		ColorTargetDescription.blend_state.src_color_blendfactor = _GetSourceBlendFactor(Configuration.Blend);
		ColorTargetDescription.blend_state.dst_color_blendfactor = _GetDestinationBlendFactor(Configuration.Blend);
		ColorTargetDescription.blend_state.color_blend_op = _GetBlendOperation(Configuration.Blend);
		ColorTargetDescription.blend_state.src_alpha_blendfactor = _GetSourceBlendFactor(Configuration.Blend);
		ColorTargetDescription.blend_state.dst_alpha_blendfactor = _GetDestinationBlendFactor(Configuration.Blend);
		ColorTargetDescription.blend_state.alpha_blend_op = _GetBlendOperation(Configuration.Blend);
	}

	ColorTargetDescription.blend_state.color_write_mask = SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G | SDL_GPU_COLORCOMPONENT_B | SDL_GPU_COLORCOMPONENT_A;

	Info.target_info.color_target_descriptions = &ColorTargetDescription;
	Info.target_info.num_color_targets = 1;
	Info.target_info.depth_stencil_format = DepthFormat;
	Info.target_info.has_depth_stencil_target = (DepthFormat != SDL_GPU_TEXTUREFORMAT_INVALID);

	SDL_GPUGraphicsPipeline* PipelineHandle = SDL_CreateGPUGraphicsPipeline(m_Device, &Info);
	if (!PipelineHandle)
	{
		SDL_Log("[Renderer] Failed to create graphics pipeline: %s", SDL_GetError());
		return nullptr;
	}

	GPUPipeline* Pipeline = new GPUPipeline();
	Pipeline->Handle = PipelineHandle;
	return Pipeline;
}

/// @brief Creates a vertex buffer.
/// @param Size The size of the buffer in bytes.
/// @param Data The initial data for the buffer.
/// @return The created vertex buffer.
GPUBuffer* Renderer::CreateVertexBuffer(uint32 Size, const void* Data /* = nullptr */)
{
	if (!m_Device || Size == 0)
	{
		if (!m_Device)
		{
			SDL_Log("[Renderer] CreateVertexBuffer() called but renderer is not initialized!");

			if (Size == 0)
			{
				SDL_Log("[Renderer] The buffer size is also zero!");
			}
		}
		else if (Size == 0)
		{
			SDL_Log("[Renderer] CreateVertexBuffer() called but size is zero!");
		}

		return nullptr;
	}

	SDL_GPUBufferCreateInfo Info = {};

	Info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	Info.size = Size;

	SDL_GPUBuffer* BufferHandle = SDL_CreateGPUBuffer(m_Device, &Info);
	if (!BufferHandle)
	{
		SDL_Log("[Renderer] Failed to create vertex buffer: %s", SDL_GetError());
		return nullptr;
	}

	GPUBuffer* Buffer = new GPUBuffer();
	Buffer->Handle = BufferHandle;
	Buffer->Size = Size;

	if (Data)
	{
		UpdateBuffer(Buffer, Data, Size, 0);
	}

	return Buffer;
}

/// @brief Creates an index buffer.
/// @param Size The size of the buffer in bytes.
/// @param Data The initial data for the buffer.
/// @return The created index buffer.
GPUBuffer* Renderer::CreateIndexBuffer(uint32 Size, const void* Data /* = nullptr */)
{
	if (!m_Device || Size == 0)
	{
		if (!m_Device)
		{
			SDL_Log("[Renderer] CreateIndexBuffer() called but renderer is not initialized!");

			if (Size == 0)
			{
				SDL_Log("[Renderer] The buffer size is also zero!");
			}
		}
		else if (Size == 0)
		{
			SDL_Log("[Renderer] CreateIndexBuffer() called but size is zero!");
		}

		return nullptr;
	}

	SDL_GPUBufferCreateInfo Info = {};

	Info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
	Info.size = Size;

	SDL_GPUBuffer* BufferHandle = SDL_CreateGPUBuffer(m_Device, &Info);
	if (!BufferHandle)
	{
		SDL_Log("[Renderer] Failed to create index buffer: %s", SDL_GetError());
		return nullptr;
	}

	GPUBuffer* Buffer = new GPUBuffer();
	Buffer->Handle = BufferHandle;
	Buffer->Size = Size;

	if (Data)
	{
		UpdateBuffer(Buffer, Data, Size, 0);
	}

	return Buffer;
}

/// @brief Creates a uniform buffer.
/// @param Size The size of the buffer in bytes.
/// @return The created uniform buffer.
GPUBuffer* Renderer::CreateUniformBuffer(uint32 Size)
{
	// SDL3 GPU API uses push constants for uniforms...
	//
	// This creates a regular buffer that can be used for storage or other purposes.
	if (!m_Device || Size == 0)
	{
		if (!m_Device)
		{
			SDL_Log("[Renderer] CreateUniformBuffer() called but renderer is not initialized!");

			if (Size == 0)
			{
				SDL_Log("[Renderer] The buffer size is also zero!");
			}
		}
		else if (Size == 0)
		{
			SDL_Log("[Renderer] CreateUniformBuffer() called but size is zero!");
		}

		return nullptr;
	}

	SDL_GPUBufferCreateInfo Info = {};

	Info.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
	Info.size = Size;

	SDL_GPUBuffer* BufferHandle = SDL_CreateGPUBuffer(m_Device, &Info);
	if (!BufferHandle)
	{
		SDL_Log("[Renderer] Failed to create uniform buffer: %s", SDL_GetError());
		return nullptr;
	}

	GPUBuffer* Buffer = new GPUBuffer();
	Buffer->Handle = BufferHandle;
	Buffer->Size = Size;

	return Buffer;
}

/// @brief Creates a texture.
/// @param Width The width of the texture.
/// @param Height The height of the texture.
/// @param Format The format of the texture.
/// @param MipLevels The number of mip levels.
/// @param Data The initial data for the texture.
/// @return The created texture.
GPUTexture* Renderer::CreateTexture(uint32 Width, uint32 Height, TextureFormat Format, uint32 MipLevels, /* = 1 */ const void* Data /* = nullptr */)
{
	if (!m_Device || Width == 0 || Height == 0)
	{
		if (!m_Device)
		{
			SDL_Log("[Renderer] CreateTexture() called but renderer is not initialized!");

			if (Width == 0 && Height != 0)
			{
				SDL_Log("[Renderer] The texture width is also zero!");
			}

			if (Height == 0 && Width != 0)
			{
				SDL_Log("[Renderer] The texture height is also zero!");
			}

			if (Width == 0 && Height == 0)
			{
				SDL_Log("[Renderer] The texture width and height are also zero!");
			}
		}
		else
		{
			if (Width == 0 && Height != 0)
			{
				SDL_Log("[Renderer] CreateTexture() called but texture width is zero!");
			}

			if (Height == 0 && Width != 0)
			{
				SDL_Log("[Renderer] CreateTexture() called but texture height is zero!");
			}

			if (Width == 0 && Height == 0)
			{
				SDL_Log("[Renderer] CreateTexture() called but texture width and height are zero!");
			}
		}

		return nullptr;
	}

	SDL_GPUTextureCreateInfo TextureCreateInfo = {};
	TextureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
	TextureCreateInfo.format = _ConvertFormat(Format);
	TextureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	TextureCreateInfo.width = Width;
	TextureCreateInfo.height = Height;
	TextureCreateInfo.layer_count_or_depth = 1;
	TextureCreateInfo.num_levels = MipLevels;
	TextureCreateInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;

	SDL_GPUTexture* TextureHandle = SDL_CreateGPUTexture(m_Device, &TextureCreateInfo);
	if (!TextureHandle)
	{
		SDL_Log("[Renderer] Failed to create texture: %s", SDL_GetError());
		return nullptr;
	}

	GPUTexture* Texture = new GPUTexture();
	Texture->Handle = TextureHandle;
	Texture->Width = Width;
	Texture->Height = Height;
	Texture->Depth = 1;
	Texture->MipLevels = MipLevels;
	Texture->Format = Format;

	if (Data)
	{
		uint32 BytesPerPixel = (Format == TextureFormat::R8) ? 1 : 4;
		UpdateTexture(Texture, Data, Width * Height * BytesPerPixel);
	}

	return Texture;
}

/// @brief Creates a sampler.
/// @param Configuration The sampler configuration.
/// @return The created sampler.
GPUSampler* Renderer::CreateSampler(const SamplerConfiguration& Configuration /* = {} */)
{
	if (!m_Device)
	{
		SDL_Log("[Renderer] CreateSampler() called but renderer is not initialized!");
		return nullptr;
	}

	SDL_GPUSamplerCreateInfo SamplerCreateInfo = {};
	SamplerCreateInfo.min_filter = _ConvertFilter(Configuration.MinFilter);
	SamplerCreateInfo.mag_filter = _ConvertFilter(Configuration.MagFilter);
	SamplerCreateInfo.mipmap_mode = (Configuration.MipFilter == TextureFilter::Linear) ? SDL_GPU_SAMPLERMIPMAPMODE_LINEAR : SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
	SamplerCreateInfo.address_mode_u = _ConvertWrap(Configuration.WrapU);
	SamplerCreateInfo.address_mode_v = _ConvertWrap(Configuration.WrapV);
	SamplerCreateInfo.address_mode_w = _ConvertWrap(Configuration.WrapW);
	SamplerCreateInfo.mip_lod_bias = Configuration.MipLodBias;
	SamplerCreateInfo.max_anisotropy = Configuration.MaxAnisotropy;
	SamplerCreateInfo.enable_anisotropy = Configuration.MaxAnisotropy > 1.0f;
	SamplerCreateInfo.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
	SamplerCreateInfo.min_lod = 0.0f;
	SamplerCreateInfo.max_lod = 1000.0f;

	SDL_GPUSampler* SamplerHandle = SDL_CreateGPUSampler(m_Device, &SamplerCreateInfo);
	if (!SamplerHandle)
	{
		SDL_Log("[Renderer] Failed to create sampler: %s", SDL_GetError());
		return nullptr;
	}

	GPUSampler* Sampler = new GPUSampler();
	Sampler->Handle = SamplerHandle;
	return Sampler;
}

/// @brief Destroys a shader.
/// @param Shader The shader to destroy.
void Renderer::DestroyShader(GPUShader* Shader)
{
	if (Shader)
	{
		if (m_Device && Shader->Handle)
		{
			SDL_ReleaseGPUShader(m_Device, Shader->Handle);
		}
		delete Shader;
	}
}

/// @brief Destroys a pipeline.
/// @param Pipeline The pipeline to destroy.
void Renderer::DestroyPipeline(GPUPipeline* Pipeline)
{
	if (Pipeline)
	{
		if (m_Device && Pipeline->Handle)
		{
			SDL_ReleaseGPUGraphicsPipeline(m_Device, Pipeline->Handle);
		}
		delete Pipeline;
	}
}

/// @brief Destroys a buffer.
/// @param Buffer The buffer to destroy.
void Renderer::DestroyBuffer(GPUBuffer* Buffer)
{
	if (Buffer)
	{
		if (m_Device && Buffer->Handle)
		{
			SDL_ReleaseGPUBuffer(m_Device, Buffer->Handle);
		}
		delete Buffer;
	}
}

/// @brief Destroys a texture.
/// @param Texture The texture to destroy.
void Renderer::DestroyTexture(GPUTexture* Texture)
{
	if (Texture)
	{
		if (m_Device && Texture->Handle)
		{
			SDL_ReleaseGPUTexture(m_Device, Texture->Handle);
		}
		delete Texture;
	}
}

/// @brief Destroys a sampler.
/// @param Sampler The sampler to destroy.
void Renderer::DestroySampler(GPUSampler* Sampler)
{
	if (Sampler)
	{
		if (m_Device && Sampler->Handle)
		{
			SDL_ReleaseGPUSampler(m_Device, Sampler->Handle);
		}
		delete Sampler;
	}
}

/// @brief Updates a buffer.
/// @param Buffer The buffer to update.
/// @param Data The data to update the buffer with.
/// @param Size The size of the data in bytes.
/// @param Offset The offset in the buffer to start updating.
void Renderer::UpdateBuffer(GPUBuffer* Buffer, const void* Data, uint32 Size, uint32 Offset /* = 0 */)
{
	if (!m_Device || !Buffer || !Buffer->Handle || !Data || Size == 0)
	{
		if (!m_Device)
		{
			SDL_Log("[Renderer] UpdateBuffer() called but renderer is not initialized!");

			if (!Buffer)
			{
				SDL_Log("[Renderer] The buffer is also null!");
			}
			else if (!Buffer->Handle)
			{
				SDL_Log("[Renderer] The buffer handle is null!");
			}

			if (!Data)
			{
				SDL_Log("[Renderer] The data pointer is also null!");
			}

			if (Size == 0)
			{
				SDL_Log("[Renderer] The data size is also zero!");
			}
		}
		else
		{
			if (!Buffer)
			{
				SDL_Log("[Renderer] UpdateBuffer() called but buffer is null!");
			}
			else if (!Buffer->Handle)
			{
				SDL_Log("[Renderer] UpdateBuffer() called but buffer handle is null!");
			}

			if (!Data)
			{
				SDL_Log("[Renderer] UpdateBuffer() called but data pointer is null!");
			}

			if (Size == 0)
			{
				SDL_Log("[Renderer] UpdateBuffer() called but data size is zero!");
			}
		}

		return ;
	}

	// Create transfer buffer.
	SDL_GPUTransferBufferCreateInfo TransferInfo = {};
	TransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	TransferInfo.size = Size;

	SDL_GPUTransferBuffer* TransferBuffer = SDL_CreateGPUTransferBuffer(m_Device, &TransferInfo);
	if (!TransferBuffer)
	{
		SDL_Log("[Renderer] Failed to create transfer buffer: %s", SDL_GetError());
		return;
	}

	// Map and copy data.
	void* MappedData = SDL_MapGPUTransferBuffer(m_Device, TransferBuffer, false);
	if (MappedData)
	{
		SDL_memcpy(MappedData, Data, Size);
		SDL_UnmapGPUTransferBuffer(m_Device, TransferBuffer);
	}

	// Upload to GPU.
	SDL_GPUCommandBuffer* CopyCommand = SDL_AcquireGPUCommandBuffer(m_Device);
	if (CopyCommand)
	{
		SDL_GPUCopyPass* CopyPass = SDL_BeginGPUCopyPass(CopyCommand);
		if (CopyPass)
		{
			SDL_GPUTransferBufferLocation SourceLocation = {};
			SourceLocation.transfer_buffer = TransferBuffer;
			SourceLocation.offset = 0;

			SDL_GPUBufferRegion DestinationRegion = {};
			DestinationRegion.buffer = Buffer->Handle;
			DestinationRegion.offset = Offset;
			DestinationRegion.size = Size;

			SDL_UploadToGPUBuffer(CopyPass, &SourceLocation, &DestinationRegion, false);
			SDL_EndGPUCopyPass(CopyPass);
		}
		SDL_SubmitGPUCommandBuffer(CopyCommand);
	}

	SDL_ReleaseGPUTransferBuffer(m_Device, TransferBuffer);
}

/// @brief Updates a texture.
/// @param Texture The texture to update.
/// @param Data The data to update the texture with.
/// @param Size The size of the data in bytes.
void Renderer::UpdateTexture(GPUTexture* Texture, const void* Data, uint32 Size)
{
	if (!m_Device || !Texture || !Texture->Handle || !Data || Size == 0)
	{
		if (!m_Device)
		{
			SDL_Log("[Renderer] UpdateTexture() called but renderer is not initialized!");

			if (!Texture)
			{
				SDL_Log("[Renderer] The texture is also null!");
			}
			else if (!Texture->Handle)
			{
				SDL_Log("[Renderer] The texture handle is null!");
			}

			if (!Data)
			{
				SDL_Log("[Renderer] The data pointer is also null!");
			}

			if (Size == 0)
			{
				SDL_Log("[Renderer] The data size is also zero!");
			}
		}
		else
		{
			if (!Texture)
			{
				SDL_Log("[Renderer] UpdateTexture() called but texture is null!");
			}
			else if (!Texture->Handle)
			{
				SDL_Log("[Renderer] UpdateTexture() called but texture handle is null!");
			}

			if (!Data)
			{
				SDL_Log("[Renderer] UpdateTexture() called but data pointer is null!");
			}

			if (Size == 0)
			{
				SDL_Log("[Renderer] UpdateTexture() called but data size is zero!");
			}
		}

		return;
	}

	// Create transfer buffer.
	SDL_GPUTransferBufferCreateInfo TransferInfo = {};
	TransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	TransferInfo.size = Size;

	SDL_GPUTransferBuffer* TransferBuffer = SDL_CreateGPUTransferBuffer(m_Device, &TransferInfo);
	if (!TransferBuffer)
	{
		SDL_Log("[Renderer] Failed to create transfer buffer: %s", SDL_GetError());
		return;
	}

	// Map and copy data.
	void* MappedData = SDL_MapGPUTransferBuffer(m_Device, TransferBuffer, false);
	if (MappedData)
	{
		SDL_memcpy(MappedData, Data, Size);
		SDL_UnmapGPUTransferBuffer(m_Device, TransferBuffer);
	}

	// Upload to GPU.
	SDL_GPUCommandBuffer* CopyCommand = SDL_AcquireGPUCommandBuffer(m_Device);
	if (CopyCommand)
	{
		SDL_GPUCopyPass* CopyPass = SDL_BeginGPUCopyPass(CopyCommand);
		if (CopyPass)
		{
			SDL_GPUTextureTransferInfo SourceInfo = {};

			SourceInfo.transfer_buffer = TransferBuffer;
			SourceInfo.offset = 0;

			SDL_GPUTextureRegion DestinationRegion = {};
			DestinationRegion.texture = Texture->Handle;
			DestinationRegion.mip_level = 0;
			DestinationRegion.layer = 0;
			DestinationRegion.x = 0;
			DestinationRegion.y = 0;
			DestinationRegion.z = 0;
			DestinationRegion.w = Texture->Width;
			DestinationRegion.h = Texture->Height;
			DestinationRegion.d = 1;

			SDL_UploadToGPUTexture(CopyPass, &SourceInfo, &DestinationRegion, false);
			SDL_EndGPUCopyPass(CopyPass);
		}
		SDL_SubmitGPUCommandBuffer(CopyCommand);
	}

	SDL_ReleaseGPUTransferBuffer(m_Device, TransferBuffer);
}

REDLYNX_NAMESPACE_END_ENGINE
