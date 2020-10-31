//
// Created by boat on 19-10-20.
//

#ifndef WEBSERV_LANGMOD_ESLANGUAGE_HPP
#define WEBSERV_LANGMOD_ESLANGUAGE_HPP

#include "ScriptReader.hpp"
#include "es/duktape.h"

namespace modules {

class ESLanguage: public ILanguage {
public:
    void load_file(std::string&& path) override;
    void load_environment(std::vector<std::string>&& environment) override;
    void execute() override;
private:
    std::string _file_path;
    duk_context* _context;
};

}

#endif //WEBSERV_LANGMOD_ESLANGUAGE_HPP
