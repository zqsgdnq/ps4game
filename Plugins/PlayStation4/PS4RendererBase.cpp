#ifdef _ORBIS

#include <iostream>
#include <gnmx\basegfxcontext.h>

#include "PS4RendererBase.h"
#include "PS4Window.h"
#include "PS4Mesh.h"

namespace SonyMath = sce::Vectormath::Scalar::Aos;
using namespace NCL;
using namespace NCL::PS4;

PS4RendererBase::PS4RendererBase(PS4Window*window) : RendererBase(*window)	{
	framesSubmitted		 = 0;
	currentGPUBuffer	 = 0;
	currentGFXContext	 = nullptr;

	InitialiseGCMRendering();

	currentFrame		= &frames[currentGPUBuffer];	
	currentGFXContext	= &currentFrame->GetCommandBuffer();
	EndFrame(); //always swap at least once...
}

PS4RendererBase::~PS4RendererBase()	{
	DestroyGCMRendering();
}

void	PS4RendererBase::InitialiseGCMRendering() {
	PS4Window* win = (PS4Window*)&hostWindow;

	frames = (PS4Frame*)onionAllocator.allocate(sizeof(PS4Frame) * win->GetScreenBufferCount(), alignof(PS4Frame));

	for (int i = 0; i < win->GetScreenBufferCount(); ++i) {
		new (&frames[i])PS4Frame();
	}
}

PS4ScreenBuffer*	PS4RendererBase::GenerateScreenBuffer(uint width, uint height, bool colour, bool depth, bool stencil) {
	PS4ScreenBuffer* buffer = new PS4ScreenBuffer();

	if (colour) {	
		Gnm::RenderTargetSpec spec;
		spec.init();
		spec.m_width		= width;
		spec.m_height		= height;
		spec.m_numSamples	= Gnm::kNumSamples1;
		spec.m_numFragments = Gnm::kNumFragments1;
		spec.m_colorFormat	= Gnm::kDataFormatB8G8R8A8UnormSrgb;	

		GpuAddress::computeSurfaceTileMode(Gnm::GpuMode::kGpuModeBase, &spec.m_colorTileModeHint, GpuAddress::kSurfaceTypeColorTargetDisplayable, spec.m_colorFormat, 1);

		int32_t success = buffer->colourTarget.init(&spec);

		if (success != SCE_GNM_OK) {
			bool a = true;
		}

		const Gnm::SizeAlign colourAlign = buffer->colourTarget.getColorSizeAlign();

		void *colourMemory = stackAllocators[GARLIC].allocate(colourAlign);

		Gnm::registerResource(nullptr, ownerHandle, colourMemory, colourAlign.m_size,
			"Colour", Gnm::kResourceTypeDepthRenderTargetBaseAddress, 0);

		buffer->colourTarget.setAddresses(colourMemory, NULL, NULL);
	}

	if (depth) {
		Gnm::DepthRenderTargetSpec spec;
		spec.init();
		spec.m_width			= width;
		spec.m_height			= height;
		spec.m_numFragments		= Gnm::kNumFragments1;
		spec.m_zFormat			= Gnm::ZFormat::kZFormat32Float;
		spec.m_stencilFormat	= (stencil ? Gnm::kStencil8 : Gnm::kStencilInvalid);

		GpuAddress::computeSurfaceTileMode(Gnm::GpuMode::kGpuModeBase, &spec.m_tileModeHint, GpuAddress::kSurfaceTypeDepthTarget, Gnm::DataFormat::build(spec.m_zFormat), 1);

		void* stencilMemory = 0;

		int32_t success = buffer->depthTarget.init(&spec);

		if (success != SCE_GNM_OK) {
			bool a = true;
		}

		void *depthMemory = stackAllocators[GARLIC].allocate(buffer->depthTarget.getZSizeAlign());

		Gnm::registerResource(nullptr, ownerHandle, depthMemory, buffer->depthTarget.getZSizeAlign().m_size,
			"Depth", Gnm::kResourceTypeDepthRenderTargetBaseAddress, 0);


		if (stencil) {
			stencilMemory = stackAllocators[GARLIC].allocate(buffer->depthTarget.getStencilSizeAlign());

			Gnm::registerResource(nullptr, ownerHandle, stencilMemory, buffer->depthTarget.getStencilSizeAlign().m_size,
				"Stencil", Gnm::kResourceTypeDepthRenderTargetBaseAddress, 0);
		}

		buffer->depthTarget.setAddresses(depthMemory, stencilMemory);
	}

	return buffer;
}

void	PS4RendererBase::DestroyGCMRendering() {
	//onionAllocator->release(frames);
}
void	PS4RendererBase::OnWindowResize(int w, int h)  {
	//?????
}

void	PS4RendererBase::BeginFrame()   {
	currentFrame->StartFrame();

	PS4Window* win = (PS4Window*)&hostWindow;

	currentGFXContext->waitUntilSafeForRendering(win->GetVideoHandle(), currentGPUBuffer);

	SetRenderBuffer(backBuffer, true, true, true);
}

void PS4RendererBase::EndFrame()			{	
	currentFrame->EndFrame();
	framesSubmitted++;

	PS4Window* win = (PS4Window*)&hostWindow;
	currentGPUBuffer = win->SwapBuffers(&backBuffer);

	drawBuffer = backBuffer;

	if (currentGFXContext) {
		if (currentGFXContext->submit() != sce::Gnm::kSubmissionSuccess) {
			std::cerr << "Graphics queue submission failed?" << std::endl;
		}
		Gnm::submitDone();
	}
	currentFrame = &frames[currentGPUBuffer];
	currentGFXContext = &currentFrame->GetCommandBuffer();
}
 
void	PS4RendererBase::SetRenderBuffer(PS4ScreenBuffer*buffer, bool clearColour, bool clearDepth, bool clearStencil) {
	drawBuffer = buffer;
	currentGFXContext->setRenderTargetMask(0xF);
	currentGFXContext->setRenderTarget(0, &drawBuffer->colourTarget);
	currentGFXContext->setDepthRenderTarget(&drawBuffer->depthTarget);

	currentGFXContext->setupScreenViewport(0, 0, drawBuffer->colourTarget.getWidth(), drawBuffer->colourTarget.getHeight(), 0.5f, 0.5f);
	currentGFXContext->setScreenScissor(0, 0, drawBuffer->colourTarget.getWidth(), drawBuffer->colourTarget.getHeight());
	currentGFXContext->setWindowScissor(0, 0, drawBuffer->colourTarget.getWidth(), drawBuffer->colourTarget.getHeight(), sce::Gnm::WindowOffsetMode::kWindowOffsetDisable);
	currentGFXContext->setGenericScissor(0, 0, drawBuffer->colourTarget.getWidth(), drawBuffer->colourTarget.getHeight(), sce::Gnm::WindowOffsetMode::kWindowOffsetDisable);

	ClearBuffer(drawBuffer, clearColour, clearDepth, clearStencil);
}

void	PS4RendererBase::ClearBuffer(PS4ScreenBuffer* buffer, bool colour, bool depth, bool stencil) {
	if (colour && buffer->colourTarget.getBaseAddress()) {
		//Vector4 defaultClearColour(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, 1.0f);
		SonyMath::Vector4 defaultClearColour(0.0f, 0.0f, 0.0f, 1.0f);
		SurfaceUtil::clearRenderTarget(*currentGFXContext, &buffer->colourTarget, defaultClearColour);
	}

	if (depth && buffer->depthTarget.getZReadAddress()) {
		float defaultDepth = 1.0f;
		SurfaceUtil::clearDepthTarget(*currentGFXContext, &buffer->depthTarget, defaultDepth);
	}

	if (stencil && buffer->depthTarget.getStencilReadAddress()) {
		int defaultStencil = 0;
		SurfaceUtil::clearStencilTarget(*currentGFXContext, &buffer->depthTarget, defaultStencil);
	}
}

void PS4RendererBase::DrawMesh(PS4Mesh& mesh) {
	mesh.SubmitDraw(*currentGFXContext, Gnm::ShaderStage::kShaderStageVs);
}
#endif
