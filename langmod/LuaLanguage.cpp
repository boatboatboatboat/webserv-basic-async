//
// Created by boat on 18-10-20.
//

#include "LuaLanguage.hpp"

const char* SCRIPT_ENV_NAME = "__SCRIPT_ENV";

static int l_getenv(lua_State* state)
{
    luaL_checkstring(state, 1);
    lua_getglobal(state, SCRIPT_ENV_NAME);
    lua_pushvalue(state, -2);
    lua_rawget(state, -2);
    return 1;
}

namespace langmod {

LuaLanguage::~LuaLanguage()
{
    if (_state != nullptr) {
        lua_close(_state);
    }
}

void LuaLanguage::load_file(std::string&& file_path)
{
    _file_path = std::move(file_path);
    _state = luaL_newstate();
    if (_state == nullptr) {
        throw std::runtime_error("Lua failed to create a state");
    }
    luaL_openlibs(_state);
}

void LuaLanguage::load_environment(std::vector<std::string>&& environment)
{
    // the use of a global env table instead of an upvalue for getenv
    //  is intentional
    // local SCRIPT_ENV_NAME = {}
    lua_newtable(_state);
    for (auto const& env_var : environment) {
        auto env_end = env_var.find('=');
        auto name = env_var.substr(0, env_end);
        auto var = env_var.substr(env_end + 1);
        // SCRIPT_ENV_NAME[name] = var
        lua_pushstring(_state, name.c_str());
        lua_pushstring(_state, var.c_str());
        lua_settable(_state, -3);
    }
    // _G.SCRIPT_ENV_NAME = SCRIPT_ENV_NAME
    lua_setglobal(_state, SCRIPT_ENV_NAME);
    // os.getenv = l_getenv
    lua_getglobal(_state, "os");
    lua_pushstring(_state, "getenv");
    lua_pushcfunction(_state, l_getenv);
    lua_settable(_state, -3);
}

void LuaLanguage::execute()
{
    int res = luaL_dofile(_state, _file_path.c_str());
    if (res != 0) {
        throw std::runtime_error("lua dofile returned non-zero");
    }
    exit(res);
}

}