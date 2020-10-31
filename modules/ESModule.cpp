//
// Created by boat on 10/30/20.
//

#include "ESModule.hpp"
#include "ESLanguage.hpp"

using std::move;
using cgi::Cgi;

void modules::ESModule::execute(http::IncomingRequest& req, http::OutgoingResponseBuilder& res, modules::ModuleLoaderStateInfo mlsi)
{
    res.cgi(Cgi(mlsi.current_path, move(req), mlsi.csfi, modules::ESLanguage()));
}
