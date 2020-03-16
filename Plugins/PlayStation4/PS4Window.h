#pragma once
#ifdef _ORBIS
#include "../../Common/Window.h"

#include <..\samples\sample_code\graphics\api_gnm\toolkit\allocators.h>
#include <..\samples\sample_code\graphics\api_gnm\toolkit\stack_allocator.h>
#include <..\samples\sample_code\graphics\api_gnm\toolkit\toolkit.h>

#include "PS4MemoryAware.h"

namespace NCL {
	namespace PS4 {			
		struct PS4ScreenBuffer {
				sce::Gnm::RenderTarget		colourTarget;
				sce::Gnm::DepthRenderTarget depthTarget;
			};
		class PS4Window	: public Window, public PS4MemoryAware {
			friend class PS4RendererBase;
		public:

			PS4Window(const std::string& title, int sizeX, int sizeY, bool fullScreen, int offsetX, int offsetY);
			~PS4Window();

			void	LockMouseToWindow(bool lock) override {
			}

			void	ShowOSPointer(bool show) override {
			}


		protected:			
			int GetVideoHandle();
			int SwapBuffers(PS4ScreenBuffer** newBuffer);

			int GetScreenBufferCount();

			PS4ScreenBuffer* GenerateScreenBuffer(uint width, uint height, bool colour = true, bool depth = true, bool stencil = false);


			void	InitialiseVideoSystem();
			void	DestroyVideoSystem();

			bool InternalUpdate() override;

		protected:
			const int			_bufferCount;	//How many screen buffers should we have
			int					currentScreenBuffer;
			int					prevScreenBuffer;
			//VIDEO SYSTEM VARIABLES
			int videoHandle;		//Handle to video system

			PS4ScreenBuffer** screenBuffers;
		};
	}
}
#endif
