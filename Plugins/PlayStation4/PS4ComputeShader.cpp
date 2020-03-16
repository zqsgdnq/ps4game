#ifdef _ORBIS
#include "PS4ComputeShader.h"
#include "PS4Shader.h"
#include <gnmx\shader_parser.h>

using namespace NCL::PS4;

PS4ComputeShader::PS4ComputeShader(const std::string& filename)
{
	char*	binData = NULL;	//resulting compiled shader bytes
	int		binSize = 0;
	isValid = false;

	Gnmx::ShaderInfo shaderInfo;
	if (PS4Shader::LoadShaderBinary(filename, binData, binSize)) {
		binary.loadFromMemory(binData, binSize);
		Gnmx::parseShader(&shaderInfo, binData);

		void* shaderBinary = garlicAllocator.allocate(shaderInfo.m_gpuShaderCodeSize, Gnm::kAlignmentOfShaderInBytes);
		void* shaderHeader = onionAllocator.allocate(shaderInfo.m_csShader->computeSize(), Gnm::kAlignmentOfBufferInBytes);

		memcpy(shaderBinary, shaderInfo.m_gpuShaderCode, shaderInfo.m_gpuShaderCodeSize);
		memcpy(shaderHeader, shaderInfo.m_csShader, shaderInfo.m_csShader->computeSize());

		shader = (Gnmx::CsShader*)shaderHeader;
		shader->patchShaderGpuAddress(shaderBinary);

		Gnmx::generateInputOffsetsCache(&cache, Gnm::kShaderStageCs, shader);
	
		Gnm::registerResource(nullptr, ownerHandle, shader->getBaseAddress(),
			shaderInfo.m_gpuShaderCodeSize, filename.c_str(), Gnm::kResourceTypeShaderBaseAddress, 0);
		isValid = true;	
	}
}

void PS4ComputeShader::Bind(Gnmx::GnmxGfxContext& cmdList) const	{
	//cmdList.setShaderType(Gnm::kShaderTypeCompute);

	cmdList.setCsShader(shader, &cache);
}

void PS4ComputeShader::Execute(Gnmx::GnmxGfxContext& cmdList, int x, int y, int z) const {
	cmdList.dispatch(x, y, z);
}

void PS4ComputeShader::Synchronise(Gnmx::GnmxGfxContext& cmdList) {
	Gnmx::GnmxDrawCommandBuffer* dcb = &cmdList.m_dcb;

	volatile uint64_t* label = (volatile uint64_t*)dcb->allocateFromCommandBuffer(sizeof(uint64_t), Gnm::kEmbeddedDataAlignment8); // allocate memory from the command buffer
	uint32_t zero = 0;
	dcb->writeDataInline((void*)label, &zero, 1, Gnm::kWriteDataConfirmEnable);
	dcb->writeAtEndOfShader(Gnm::kEosCsDone, const_cast<uint64_t*>(label), 0x1); // tell the CP to write a 1 into the memory only when all compute shaders have finished
	dcb->waitOnAddress(const_cast<uint64_t*>(label), 0xffffffff, Gnm::kWaitCompareFuncEqual, 0x1); // tell the CP to wait until the memory has the val 1
	dcb->flushShaderCachesAndWait(Gnm::kCacheActionWriteBackAndInvalidateL1andL2, 0, Gnm::kStallCommandBufferParserDisable); // tell the CP to flush the L1$ and L2$
}

PS4ComputeShader::~PS4ComputeShader()
{
}
//TODO - handle common funcs with 'normal' shader better?

int		PS4ComputeShader::GetConstantBuffer(const std::string& name) {
	sce::Shader::Binary::Buffer* constantBuffer = binary.getBufferResourceByName(name.c_str());
	if (!constantBuffer) {
		return -1;
	}
	return constantBuffer->m_resourceIndex;
}

int		PS4ComputeShader::GetRegularBuffer(const std::string& name) {
	sce::Shader::Binary::Buffer* buffer = binary.getBufferResourceByName(name.c_str());
	if (!buffer) {
		return -1;
	}
	return buffer->m_resourceIndex;
}
#endif