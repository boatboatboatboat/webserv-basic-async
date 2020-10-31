//
// Created by boat on 10/30/20.
//

#ifndef WEBSERV_MODULES_ESMODULE_HPP
#define WEBSERV_MODULES_ESMODULE_HPP

#include "IModule.hpp"

namespace modules {

class ESModule final : public IModule {
public:
    ESModule() = default;
    ~ESModule() override = default;

    void execute(http::IncomingRequest&, http::OutgoingResponseBuilder&, ModuleLoaderStateInfo) override;
};

}

#endif //WEBSERV_MODULES_ESMODULE_HPP
