#pragma once
#include"ExampleRenderer.h"
#include"../8503/PhysicsSystem.h"
namespace NCL {
	namespace PS4 {
		
		class PS4Game {
		public:
			PS4Game(PS4Window* window);
			~PS4Game();
			virtual void UpdateGame(float dt);
		

		void InitialiseAssets();

		void InitCamera();
		//void UpdateKeys();

		void InitWorld();
		void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
		void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
		void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
		void BridgeConstraintTest();
		void SimpleGJKTest();

		bool SelectObject();
		void MoveSelectedObject();
		void DebugObjectMovement();
		void LockedObjectMovement();
		void LockedCameraMovement();

		GameObject* AddFloorToWorld(const Vector3& position);
		GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
		GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
		//IT'S HAPPENING
		GameObject* AddGooseToWorld(const Vector3& position);
		GameObject* AddParkKeeperToWorld(const Vector3& position);
		GameObject* AddCharacterToWorld(const Vector3& position);
		GameObject* AddAppleToWorld(const Vector3& position);


		ExampleRenderer* renderer;
		PhysicsSystem* physics;
		GameWorld* world;

		bool useGravity;
		bool inSelectionMode;

		float		forceMagnitude;

		GameObject* selectionObject = nullptr;

		PS4Mesh* cubeMesh = nullptr;
		PS4Mesh* sphereMesh = nullptr;
		PS4Texture* basicTex = nullptr;
		PS4Shader* basicShader = nullptr;

		//Coursework Meshes
		PS4Mesh* gooseMesh = nullptr;
		PS4Mesh* keeperMesh = nullptr;
		PS4Mesh* appleMesh = nullptr;
		PS4Mesh* charA = nullptr;
		PS4Mesh* charB = nullptr;

		//Coursework Additional functionality	
		GameObject* lockedObject = nullptr;
		Vector3 lockedOffset = Vector3(0, 14, 20);
		void LockCameraToObject(GameObject* o) {
			lockedObject = o;

		}
	};
	}
}