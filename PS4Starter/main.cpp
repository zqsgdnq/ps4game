#include <stdlib.h>
unsigned int sceLibcHeapExtendedAlloc = 1;			/* Switch to dynamic allocation */
size_t       sceLibcHeapSize = 256 * 1024 * 1024;	/* Set up heap area upper limit as 256 MiB */

#include "../Plugins/PlayStation4/PS4Window.h"
#include "../Plugins/PlayStation4/Ps4AudioSystem.h"

#include <iostream>
#include "PS4Game.h"
#include "ExampleRenderer.h"

using namespace NCL;
using namespace NCL::PS4;



int main(void) {
	//512MB of each -> You'll probably want more of this as you start making your game!
	PS4MemoryAware::InitialiseMemory(512 << 20, 512 * 1024 * 1024); //2 ways of getting the same num!
	PS4Window*w = (PS4Window*)Window::CreateGameWindow("PS4 Example Code", 1920, 1080);

	//ExampleRenderer renderer(w);
	//
	//PS4Input		input		= PS4Input();
	
	Ps4AudioSystem*	audioSystem = new Ps4AudioSystem(8);
	
	GameTimer t;
	PS4Game* g = new PS4Game(w);
	while (w->UpdateWindow()) {
		float time = w->GetTimer()->GetTimeDeltaSeconds();
		g->UpdateGame(time);
		//input.Poll();

		//renderer.Update(time);
		//renderer.Render();
	
		/*if (input.GetButton(0)) {
			std::cout << "LOL BUTTON" << std::endl;
		}
	
		if (input.GetButton(1)) {
			return 1;
		}*/
	}

	delete audioSystem;
	
	return 1;
}

