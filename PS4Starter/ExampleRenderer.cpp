#include "ExampleRenderer.h"
#include "..\8503\RenderObject.h"
#include "../8503/PhysicsSystem.h"

#include "../Plugins/PlayStation4/PS4Window.h"
#include "../Plugins/PlayStation4/PS4Shader.h"
#include "../Plugins/PlayStation4/PS4Mesh.h"


using namespace NCL;
using namespace NCL::PS4;

ExampleRenderer::ExampleRenderer(PS4Window* window, GameWorld& world) : PS4RendererBase(window),gameworld(world)	{
	defaultShader = PS4Shader::GenerateShader(
		"/app0/Assets/Shaders/PS4/VertexShader.sb",
		"/app0/Assets/Shaders/PS4/PixelShader.sb"
	);
	
	computeTest = new PS4ComputeShader("/app0/Assets/Shaders/PS4/ComputeShader.sb");

	defaultMesh = PS4Mesh::GenerateTriangle();
	defaultTexture = PS4Texture::LoadTextureFromFile("/app0/Assets/Textures/doge.gnf");

	viewProjMat = (Matrix4*)onionAllocator.allocate(sizeof(Matrix4), Gnm::kEmbeddedDataAlignment4);
	*viewProjMat = Matrix4();

	cameraBuffer.initAsConstantBuffer(viewProjMat, sizeof(Matrix4));
	cameraBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // it's a constant buffer, so read-only is OK

	defaultObject[0] = new RenderObject((MeshGeometry*)defaultMesh, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);
	defaultObject[1] = new RenderObject((MeshGeometry*)defaultMesh, (ShaderBase*)defaultShader, (TextureBase*)defaultTexture);

	computeResult = (float*)onionAllocator.allocate(4, Gnm::kEmbeddedDataAlignment4);


	Gnm::Texture* shadow= (Gnm::Texture*)onionAllocator.allocate(sizeof(Gnm::Texture), Gnm::kResourceTypeGenericBuffer);
}

ExampleRenderer::~ExampleRenderer()	{
	delete defaultObject[0];
	delete defaultObject[1];
	delete defaultMesh;
	delete defaultTexture;
	delete defaultShader;
	delete computeTest;
}

void ExampleRenderer::Update(float dt)	{
	time = dt;

	defaultObject[0]->SetLocalTransform(Matrix4::Translation(Vector3(-0.4, 0, 0)) * Matrix4::Rotation(*computeResult, Vector3(0,0,1))*Matrix4::Scale(Vector3(10,10,10)));
	defaultObject[1]->SetLocalTransform(Matrix4::Translation(Vector3(0.4, 0, 0))*Matrix4::Scale(Vector3(10, 10, 10)));
}

void ExampleRenderer::UpdateRotationAmount(float dt) {
	float* tempTime = (float*)currentGFXContext->allocateFromCommandBuffer(sizeof(float), Gnm::kEmbeddedDataAlignment4);
	*tempTime = dt;

	Gnm::Buffer inputBuffer;
	inputBuffer.initAsConstantBuffer(tempTime, sizeof(float)); //since newValue is in the command buffer, it'll persist past this function
	inputBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // it's a constant buffer, so read-only is OK

	Gnm::Buffer outputBuffer;
	outputBuffer.initAsRegularBuffer(computeResult, sizeof(float), 1); //since newValue is in the command buffer, it'll persist past this function
	outputBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypeSC); // Make this buffer coherent

	int rotationIndex	= computeTest->GetRegularBuffer("rotationAmount");
	int timeIndex		= computeTest->GetConstantBuffer("InputData");

	currentGFXContext->setRwBuffers(Gnm::kShaderStageCs, rotationIndex, 1, &outputBuffer);
	currentGFXContext->setConstantBuffers(Gnm::kShaderStageCs, timeIndex, 1, &inputBuffer);

	computeTest->Bind(*currentGFXContext);
	computeTest->Execute(*currentGFXContext, 1,1,1);
	computeTest->Synchronise(*currentGFXContext);	//at this point, our rotation value will have been calculated by the compute shader...
}

void ExampleRenderer::RenderActiveScene() {
	DrawRenderObject(defaultObject[0]);
	DrawRenderObject(defaultObject[1]);
}

void ExampleRenderer::DrawRenderObject(RenderObject* o) {
	
	//float screenAspect= (float)currentWidth / (float)currentHeight;
	Matrix4* transformMat = (Matrix4*)currentGFXContext->allocateFromCommandBuffer(sizeof(Matrix4), Gnm::kEmbeddedDataAlignment4);
	*transformMat = (gameworld.GetMainCamera()->BuildProjectionMatrix(1920/1080)) *(gameworld.GetMainCamera()->BuildViewMatrix())* o->GetLocalTransform();

	Gnm::Buffer constantBuffer;
	constantBuffer.initAsConstantBuffer(transformMat, sizeof(Matrix4));
	constantBuffer.setResourceMemoryType(Gnm::kResourceMemoryTypeRO); // it's a constant buffer, so read-only is OK

	PS4Shader* realShader = (PS4Shader*)o->GetShader();

	int objIndex = realShader->GetConstantBuffer("RenderObjectData");
	int camIndex = realShader->GetConstantBuffer("CameraData");

	currentGFXContext->setConstantBuffers(Gnm::kShaderStageVs, objIndex, 1, &constantBuffer);
	currentGFXContext->setConstantBuffers(Gnm::kShaderStageVs, camIndex, 1, &cameraBuffer);

	realShader->SubmitShaderSwitch(*currentGFXContext);
	DrawMesh(*defaultMesh);
}

void ExampleRenderer::RenderFrame() {
	UpdateRotationAmount(time);
	defaultShader->SubmitShaderSwitch(*currentGFXContext);

	//Primitive Setup State
	Gnm::PrimitiveSetup primitiveSetup;
	primitiveSetup.init();
	primitiveSetup.setCullFace(Gnm::kPrimitiveSetupCullFaceNone);
	primitiveSetup.setFrontFace(Gnm::kPrimitiveSetupFrontFaceCcw);
	//primitiveSetup.setPolygonMode()
	currentGFXContext->setPrimitiveSetup(primitiveSetup);

	////Screen Access State
	Gnm::DepthStencilControl dsc;
	dsc.init();
	dsc.setDepthControl(Gnm::kDepthControlZWriteEnable, Gnm::kCompareFuncLessEqual);
	dsc.setDepthEnable(true);
	currentGFXContext->setDepthStencilControl(dsc);

	Gnm::Sampler trilinearSampler;
	trilinearSampler.init();
	trilinearSampler.setMipFilterMode(Gnm::kMipFilterModeLinear);

	currentGFXContext->setTextures(Gnm::kShaderStagePs, 0, 1, &defaultTexture->GetAPITexture());
	currentGFXContext->setSamplers(Gnm::kShaderStagePs, 0, 1, &trilinearSampler);

	*viewProjMat = Matrix4();

	RenderActiveScene();
}