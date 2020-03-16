#pragma once
#ifdef _ORBIS
#include <gnm.h>
#include <gnmx\context.h>
#include <..\samples\sample_code\graphics\api_gnm\toolkit\stack_allocator.h>

namespace sce{
	namespace Gnmx {
		namespace Toolkit {
			struct IAllocator;
		}
	}
}

namespace NCL {
	namespace PS4 {
		enum MemoryLocation {
			GARLIC,
			ONION,
			MEMORYMAX
		};
		class PS4MemoryAware {
		public:
			static void InitialiseMemory(int garlicMem, int onionMem);
			static void DestroyMemory();

			PS4MemoryAware() {}
			~PS4MemoryAware() {}
		protected:
			static sce::Gnmx::Toolkit::IAllocator		onionAllocator;
			static sce::Gnmx::Toolkit::IAllocator		garlicAllocator;
			static sce::Gnm::OwnerHandle				ownerHandle;
			static sce::Gnmx::Toolkit::StackAllocator	stackAllocators[MEMORYMAX];

			//Memory Allocation
			static int _GarlicMemory;
			static int _OnionMemory;
		};
	}
}
#endif
