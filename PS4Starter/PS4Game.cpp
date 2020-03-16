#include "PS4Game.h"
using namespace NCL;
using namespace PS4;

PS4Game::PS4Game(PS4Window* window) {

	world = new GameWorld();
	renderer = new ExampleRenderer(window,*world);
	physics = new PhysicsSystem(*world);

	InitialiseAssets();
}

PS4Game::~PS4Game() {

	delete physics;
	delete renderer;
	//delete world;
}

void PS4Game::UpdateGame(float dt) {
	world->UpdateWorld(dt);
	renderer->Update(dt);
	renderer->Render();
	physics->Update(dt);

	world->GetMainCamera()->UpdatePS4Camera(dt);

}

void PS4Game::InitialiseAssets() {


	InitCamera();
}

void PS4Game::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.5f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	//lockedObject = nullptr;

}