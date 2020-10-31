//
// Created by boat on 10/30/20.
//

#include "LuaModule.hpp"
#include "LuaLanguage.hpp"

using cgi::Cgi;
using std::move;

void modules::LuaModule::execute(http::IncomingRequest& req, http::OutgoingResponseBuilder& res, ModuleLoaderStateInfo mlsi)
{
    res.cgi(Cgi(mlsi.current_path, move(req), mlsi.csfi, modules::LuaLanguage()));
}
