#pragma once
#ifdef _ORBIS
#include "PS4MemoryAware.h"

#include <shader\binary.h>
#include <gnmx\shaderbinary.h>

#include <iostream>

using namespace sce;

namespace NCL {
	namespace PS4 {
		class PS4ComputeShader : public PS4MemoryAware
		{
		public:
			PS4ComputeShader(const std::string& filename);
			~PS4ComputeShader();

			void Bind(Gnmx::GnmxGfxContext& cmdList) const;
			void Execute(Gnmx::GnmxGfxContext& cmdList, int x, int y = 1, int z = 1) const;

			void Synchronise(Gnmx::GnmxGfxContext& cmdList);

			int		GetConstantBuffer(const std::string& name);
			int		GetRegularBuffer(const std::string& name);

		protected:
			sce::Shader::Binary::Program	binary;
			Gnmx::InputOffsetsCache			cache;
			Gnmx::CsShader*					shader;
			bool isValid;
		};
	}
}
#endif
