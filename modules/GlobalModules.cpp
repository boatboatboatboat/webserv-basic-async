//
// Created by boat on 10/30/20.
//

#include "GlobalModules.hpp"
#include "LuaModule.hpp"
#include "ESModule.hpp"

namespace modules {

optional<std::map<std::string, GlobalModules::ModuleState>> GlobalModules::_body_modules = option::nullopt;

modules::GlobalModules::GlobalModules()
{
    if (!_body_modules.has_value()) {
        _body_modules = std::map<std::string, GlobalModules::ModuleState>();
        _body_modules->insert(std::make_pair("lua", ModuleState { true, BoxPtr<LuaModule>::make() }));
        _body_modules->insert(std::make_pair("es", ModuleState { true, BoxPtr<ESModule>::make() }));
    }
}

auto GlobalModules::get_body_module(std::string const& module_name) -> optional<BoxPtr<IModule>*>
{
    auto module = _body_modules->find(module_name);
    if (module == _body_modules->end()) {
        return option::nullopt;
    }
    if (module->second.enabled) {
        return &module->second.module;
    }
    return option::nullopt;
}

void GlobalModules::disable_module(const std::string& module_name)
{
    auto module = _body_modules->find(module_name);
    if (module == _body_modules->end()) {
        WARNPRINT("No such module: " << module_name);
        return;
    }
    if (!module->second.enabled) {
        WARNPRINT("Module is not loaded");
    } else {
        INFOPRINT("Unloaded module");
    }
    module->second.enabled = false;
}

void GlobalModules::enable_module(const std::string& module_name)
{
    auto module = _body_modules->find(module_name);
    if (module == _body_modules->end()) {
        WARNPRINT("No such module: " << module_name);
        return;
    }
    if (module->second.enabled) {
        WARNPRINT("Module is already loaded");
    } else {
        INFOPRINT("Loaded module");
    }
    module->second.enabled = true;
}

}