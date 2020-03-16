#ifdef _ORBIS
#include "PS4Window.h"
#include "PS4MemoryAware.h"
#include <video_out.h>	//Video System

#include <.\graphics\api_gnm\toolkit\allocators.h>
#include <.\graphics\api_gnm\toolkit\stack_allocator.h>
#include <gnmx\basegfxcontext.h>
#include <gnm.h>
#include <gnmx\fetchshaderhelper.h>

using namespace sce;
using namespace NCL;
using namespace NCL::PS4;

PS4Window::PS4Window(const std::string& title, int sizeX, int sizeY, bool fullScreen, int offsetX, int offsetY)
	:
	_bufferCount(2)
{
	InitialiseVideoSystem();
}

PS4Window::~PS4Window()	{
	DestroyVideoSystem();
}

bool PS4Window::InternalUpdate() {
	return true;
}

void	PS4Window::InitialiseVideoSystem() {
	screenBuffers = new PS4ScreenBuffer * [_bufferCount];

	for (int i = 0; i < _bufferCount; ++i) {
		screenBuffers[i] = GenerateScreenBuffer(1920, 1080);
	}

	//Now we can open up the video port
	videoHandle = sceVideoOutOpen(0, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, NULL);

	SceVideoOutBufferAttribute attribute;
	sceVideoOutSetBufferAttribute(&attribute,
		SCE_VIDEO_OUT_PIXEL_FORMAT_B8_G8_R8_A8_SRGB,
		SCE_VIDEO_OUT_TILING_MODE_TILE,
		SCE_VIDEO_OUT_ASPECT_RATIO_16_9,
		screenBuffers[0]->colourTarget.getWidth(),
		screenBuffers[0]->colourTarget.getHeight(),
		screenBuffers[0]->colourTarget.getPitch()
	);

	void* bufferAddresses[_bufferCount];

	for (int i = 0; i < _bufferCount; ++i) {
		bufferAddresses[i] = screenBuffers[i]->colourTarget.getBaseAddress();
	}

	sceVideoOutRegisterBuffers(videoHandle, 0, bufferAddresses, _bufferCount, &attribute);
}

void	PS4Window::DestroyVideoSystem() {
	//for (int i = 0; i < _bufferCount; ++i) {
	//	delete screenBuffers[i];
	//}
	//delete[] screenBuffers;
	sceVideoOutClose(videoHandle);
}
int PS4Window::SwapBuffers(PS4ScreenBuffer** newBuffer) {
	prevScreenBuffer = currentScreenBuffer;
	currentScreenBuffer = (currentScreenBuffer + 1) % _bufferCount;
	sceVideoOutSubmitFlip(videoHandle, prevScreenBuffer, SCE_VIDEO_OUT_FLIP_MODE_VSYNC, 0);

	*newBuffer = screenBuffers[currentScreenBuffer];
	return currentScreenBuffer;
}

int PS4Window::GetScreenBufferCount() {
	return _bufferCount;
}

PS4ScreenBuffer* PS4Window::GenerateScreenBuffer(uint width, uint height, bool colour, bool depth, bool stencil) {
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

		void* colourMemory = stackAllocators[GARLIC].allocate(colourAlign);

		Gnm::registerResource(nullptr, ownerHandle, colourMemory, colourAlign.m_size,
			"Colour", Gnm::kResourceTypeDepthRenderTargetBaseAddress, 0);

		buffer->colourTarget.setAddresses(colourMemory, NULL, NULL);
	}

	if (depth) {
		Gnm::DepthRenderTargetSpec spec;
		spec.init();
		spec.m_width = width;
		spec.m_height = height;
		spec.m_numFragments = Gnm::kNumFragments1;
		spec.m_zFormat = Gnm::ZFormat::kZFormat32Float;
		spec.m_stencilFormat = (stencil ? Gnm::kStencil8 : Gnm::kStencilInvalid);

		GpuAddress::computeSurfaceTileMode(Gnm::GpuMode::kGpuModeBase, &spec.m_tileModeHint, GpuAddress::kSurfaceTypeDepthTarget, Gnm::DataFormat::build(spec.m_zFormat), 1);

		void* stencilMemory = 0;

		int32_t success = buffer->depthTarget.init(&spec);

		if (success != SCE_GNM_OK) {
			bool a = true;
		}

		void* depthMemory = stackAllocators[GARLIC].allocate(buffer->depthTarget.getZSizeAlign());

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

int PS4Window::GetVideoHandle() {
	return videoHandle;
}

#endif