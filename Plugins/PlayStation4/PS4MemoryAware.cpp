#ifdef _ORBIS
#include "PS4MemoryAware.h"
#include <.\graphics\api_gnm\toolkit\allocators.h>
#include <.\graphics\api_gnm\toolkit\stack_allocator.h>

#include <..\samples\sample_code\graphics\api_gnm\toolkit\toolkit.h>
using namespace NCL::PS4;

sce::Gnmx::Toolkit::IAllocator		PS4MemoryAware::onionAllocator;
sce::Gnmx::Toolkit::IAllocator		PS4MemoryAware::garlicAllocator;
sce::Gnm::OwnerHandle				PS4MemoryAware::ownerHandle;

sce::Gnmx::Toolkit::StackAllocator	PS4MemoryAware::stackAllocators[MEMORYMAX];

int PS4MemoryAware::_GarlicMemory	= 0;
int PS4MemoryAware::_OnionMemory	= 0;

void PS4MemoryAware::InitialiseMemory(int garlicMem, int onionMem) {
	_GarlicMemory	= garlicMem;
	_OnionMemory	= onionMem;

	stackAllocators[GARLIC].init(SCE_KERNEL_WC_GARLIC, _GarlicMemory);
	stackAllocators[ONION].init(SCE_KERNEL_WB_ONION, _OnionMemory);

	garlicAllocator = sce::Gnmx::Toolkit::GetInterface(&stackAllocators[ONION]);
	onionAllocator	= sce::Gnmx::Toolkit::GetInterface(&stackAllocators[GARLIC]);
	sce::Gnm::registerOwner(&ownerHandle, "PS4RendererBase");

	sce::Gnmx::Toolkit::Allocators allocators = sce::Gnmx::Toolkit::Allocators(onionAllocator, garlicAllocator, ownerHandle);
	sce::Gnmx::Toolkit::initializeWithAllocators(&allocators);
}

void PS4MemoryAware::DestroyMemory() {
	stackAllocators[GARLIC].deinit();
	stackAllocators[ONION].deinit();
}
#endif