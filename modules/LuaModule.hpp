//
// Created by boat on 10/30/20.
//

#ifndef WEBSERV_MODULES_LUAMODULE_HPP
#define WEBSERV_MODULES_LUAMODULE_HPP

#include "IModule.hpp"

namespace modules {

class LuaModule final : public IModule {
public:
    LuaModule() = default;
    void execute(http::IncomingRequest &, http::OutgoingResponseBuilder &, ModuleLoaderStateInfo) override;
    ~LuaModule() override = default;
};

}

#endif //WEBSERV_MODULES_LUAMODULE_HPP
