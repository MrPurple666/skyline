#include <new>
#include "nnvk.h"
#include "buffer.h"

namespace nnvk {
    Program::Program(ApiVersion version, Device *device) : device{device} {
        NNVK_FILL_VERSIONED_STRUCT(Program);
    }

    Program::~Program() {}

    void Program::SetDebugLabel(const char *label) {
        debugLabel = label;
    }

    bool Program::SetShaders(i32 count, const ShaderData *stageData) {
        return true;
    }

    bool Program::SetSubroutineLinkage(i32 i, const SubroutineLinkageMapPtr *ptr) {
        return true;
    }

    /* Wrappers */
    bool Context::ProgramInitialize(Program *program, Device *device) {
        new (program) Program(apiVersion, device);
        return true;
    }

    void Context::ProgramFinalize(Program *pool) {
        pool->~Program();
    }

    NNVK_CONTEXT_WRAP_TRIVIAL_1(void, Program, SetDebugLabel, const char *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(bool, Program, SetShaders, i32, const ShaderData *)
    NNVK_CONTEXT_WRAP_TRIVIAL_2(bool, Program, SetSubroutineLinkage, i32, const SubroutineLinkageMapPtr *)
    /* End wrappers */
}