//
// Created by boat on 10/30/20.
//

#ifndef WEBSERV_MODULES_GLOBALMODULES_HPP
#define WEBSERV_MODULES_GLOBALMODULES_HPP

#include <string>
#include <map>
#include "IModule.hpp"

namespace modules {

class GlobalModules final {
public:
    struct ModuleState {
        bool enabled;
        BoxPtr<IModule> module;
    };
    GlobalModules();
    ~GlobalModules() = default;

    auto get_body_module(std::string const& module_name) -> optional<BoxPtr<IModule>*>;
    void disable_module(std::string const& module_name);
    void enable_module(std::string const& module_name);
private:
    static optional<std::map<std::string, ModuleState>> _body_modules;
};

}

#endif //WEBSERV_MODULES_GLOBALMODULES_HPP
