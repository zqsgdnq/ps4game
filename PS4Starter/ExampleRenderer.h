#pragma once
#include "../Plugins/PlayStation4/PS4RendererBase.h"
#include "../8503/GameWorld.h"
#include "../Plugins/PlayStation4/PS4Mesh.h"
#include "../Plugins/PlayStation4/PS4Texture.h"
#include "../Plugins/PlayStation4/PS4Shader.h"

namespace NCL {
	namespace PS4 {
		class ExampleRenderer : public PS4RendererBase
		{
		public:
			ExampleRenderer(PS4Window* window,GameWorld& world);
			~ExampleRenderer();

			void Update(float dt)	 override;

			GameWorld& gameworld;
		protected:
			void UpdateRotationAmount(float dt);
			void DrawRenderObject(RenderObject* o);
			void	RenderFrame()	override;
			void RenderActiveScene() override;

			RenderObject* defaultObject[2];

			PS4ComputeShader*	computeTest;
			PS4Shader*	defaultShader;
			PS4Mesh*	defaultMesh;
			PS4Texture* defaultTexture;

			NCL::Maths::Matrix4* viewProjMat;
			Gnm::Buffer	cameraBuffer;

			float* computeResult;
			float time;
		};
	}
}

