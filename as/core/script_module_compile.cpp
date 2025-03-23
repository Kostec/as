//
// Created by ivn on 21.05.2024.
//

#include "llvm/IR/Constants.h"

#include "cpp_interface_parser.h"
#include "language_script.h"

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

#include "ir.h"

#include "script_module_compile.h"

namespace as {

ScriptModuleCompile::ScriptModuleCompile(const std::string& export_name,
        const ScriptInterface& interface,
        const std::unordered_map<std::string, std::shared_ptr<ScriptInterface>>& externalRequires,
        std::shared_ptr<ILanguageScript> language_script,
        llvm::LLVMContext& context,
        bool add_init):
    m_export_name(export_name),
    m_language_script(std::move(language_script))
{
    compile(interface, externalRequires, context, add_init);
}

void ScriptModuleCompile::dump(llvm::raw_ostream& stream) const
{
    stream << *m_module;
}

static llvm::orc::JITDylib* createJitLib(std::shared_ptr<llvm::orc::LLJIT>& jit, std::string export_name)
{
    llvm::outs() << "CreateJitLib: " << export_name << "\n";
    auto jd = jit->createJITDylib(export_name);
    if (!jd)
    {
        llvm::errs() << "Cannot create " << export_name << " library." << jd.takeError() << "\n ";
        return nullptr;
    }

    if (jd)
    {
        llvm::outs() << "Add MainJITDyLib" << "\n";
        jd->addToLinkOrder(jit->getMainJITDylib());
    }
    return &(jd.get());
}

llvm::orc::JITDylib* ScriptModuleCompile::getModuleLib(std::shared_ptr<llvm::orc::LLJIT>& jit)
{
    llvm::outs() << "getModuleLib: " << m_export_name << "\n";
    auto jd = jit->getJITDylibByName(m_export_name);
    if (!jd)
    {
        jd = createJitLib(jit, m_export_name);
        return jd;
    }

    llvm::outs() << "Clear existing library " << m_export_name << "\n";
    auto error_jd_clear = jd->clear();
    if (error_jd_clear)
    {
        llvm::errs() << "Cannot clear existing" << m_export_name << " library." << error_jd_clear << "\n ";
        return nullptr;
    }

    llvm::outs() << "Remove existing library " << m_export_name << "\n";
    auto error_remove = jit->getExecutionSession().removeJITDylib(*jd);
    if (error_remove)
    {
        llvm::errs() << "Cannot remove existing " << m_export_name << " library. " << error_remove << "\n";
        return nullptr;
    }

    jd = createJitLib(jit, m_export_name);
    return jd;
}

InitFunction ScriptModuleCompile::materialize(std::shared_ptr<llvm::orc::LLJIT>& jit,
    llvm::orc::ThreadSafeContext ts_context)
{
    auto lib = getModuleLib(jit);
    if (!lib)
    {
        return nullptr;
    }

    const auto init_name = "init_" + m_export_name;

    auto initFn = m_module.get()->getFunction(init_name);
    if (!initFn)
    {
        llvm::outs() << initFn << " Not Found\n";
    }

    std::error_code error;
    llvm::raw_fd_ostream ll_out_stream("__test.ll", error);
    m_module->print(ll_out_stream, nullptr);

    auto& context = *ts_context.getContext();
    auto error_add = jit->addIRModule(*lib, llvm::orc::ThreadSafeModule(std::move(m_module), ts_context));
    
    if (error_add)
    {
        llvm::errs() << "Cannot add module. " << error_add << "\n";
        return nullptr;
    }

    m_language_script->materialize(jit, *lib, *m_module, ts_context);
    
    auto init_func_addr = jit->lookup(*lib, init_name);
    if (!init_func_addr)
    {
        llvm::errs() << "Cannot get init function (" << init_name << "). " << init_func_addr.takeError() << "\n";
        return nullptr;
    }

    return init_func_addr.get().toPtr<void(void*)>();
}

void ScriptModuleCompile::compile(
    const ScriptInterface& interface,
    const std::unordered_map<std::string, std::shared_ptr<ScriptInterface>>& externalRequires,
    llvm::LLVMContext& context,
    bool add_init)
{
    m_module = std::move(m_language_script->createModule(context));
    const auto init_name = add_init ? "" : "init_" + m_export_name;
    m_language_script->buildModule(init_name, m_export_name, interface, externalRequires, *m_module);
}

} // as