#pragma once
#ifdef _ORBIS
#include "../../Common/RendererBase.h"

#include "PS4MemoryAware.h"
#include "PS4Frame.h"
#include "PS4Texture.h"
#include "PS4Window.h"
#include "PS4ComputeShader.h"

#include <gnm.h>
#include <gnmx\fetchshaderhelper.h>

using namespace sce;
using namespace Gnmx;
using namespace Gnmx::Toolkit;

namespace NCL {
	namespace Maths {
		class Matrix4;
	}
	namespace PS4 {
		class PS4Window;
		class PS4Shader;
		class PS4Mesh;

		class RenderObject;

		class PS4RendererBase :
			public RendererBase, public PS4MemoryAware
		{
		public:
			PS4RendererBase(PS4Window* window);
			~PS4RendererBase();

		protected:
			virtual void RenderActiveScene() = 0;

			void	OnWindowResize(int w, int h) override;
			void	BeginFrame()    override;
			void	EndFrame()		override;
			
			void	DrawMesh(PS4Mesh& mesh);

		private:
			void	InitialiseGCMRendering();
			void	DestroyGCMRendering();

			void	SetRenderBuffer(PS4ScreenBuffer*buffer, bool clearColour, bool clearDepth, bool clearStencil);
			void	ClearBuffer(PS4ScreenBuffer* buffer, bool colour, bool depth, bool stencil);

			static PS4ScreenBuffer* GenerateScreenBuffer(uint width, uint height, bool colour = true, bool depth = true, bool stencil = false);

		protected:
			int currentGPUBuffer;
			//Individual Frames
			PS4Frame*	frames;

			int framesSubmitted;

			//Per frame pointers...
			PS4ScreenBuffer*		backBuffer;  //Pointer to the current back buffer...
			PS4ScreenBuffer*		drawBuffer;	 //Pointer to whichever buffer we're currently drawing to...
			Gnmx::GnmxGfxContext*	currentGFXContext;
			PS4Frame*				currentFrame;
		};
	}
}
#endif
