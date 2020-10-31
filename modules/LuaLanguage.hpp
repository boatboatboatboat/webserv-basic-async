//
// Created by boat on 18-10-20.
//

#ifndef WEBSERV_LANGMOD_LUALANGUAGE_HPP
#define WEBSERV_LANGMOD_LUALANGUAGE_HPP

#include "ScriptReader.hpp"
#include <lua.hpp>

namespace modules {

// LuaConcreteLanguageAbstractFactoryInterface

class LuaLanguage final: public ILanguage {
public:
    void load_file(std::string&& file_path) override;
    void load_environment(std::vector<std::string>&& environment) override;
    void execute() override;
    ~LuaLanguage();
private:
    std::string _file_path;
    lua_State* _state;
};

}

#endif //WEBSERV_LANGMOD_LUALANGUAGE_HPP
