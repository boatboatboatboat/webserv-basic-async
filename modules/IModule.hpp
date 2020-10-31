//
// Created by boat on 10/30/20.
//

#ifndef WEBSERV_MODULES_IMODULE_HPP
#define WEBSERV_MODULES_IMODULE_HPP

#include "../http/IncomingRequest.hpp"
#include "../http/OutgoingResponse.hpp"
#include "../cgi/Cgi.hpp"

namespace modules {

struct ModuleLoaderStateInfo {
    std::string current_path;
    cgi::CgiServerForwardInfo csfi;
};

class IModule {
public:
    virtual void execute(http::IncomingRequest&, http::OutgoingResponseBuilder&, ModuleLoaderStateInfo) = 0;
    virtual ~IModule() = default;
};

}

#endif //WEBSERV_MODULES_IMODULE_HPP
