//
// Created by boat on 17-10-20.
//

#ifndef WEBSERV_LANGMOD_SCRIPTREADER_HPP
#define WEBSERV_LANGMOD_SCRIPTREADER_HPP

#include "../ioruntime/IAsyncRead.hpp"
#include "../ioruntime/IAsyncWrite.hpp"

namespace modules {

// LanguageAbstractFactoryInterface

class ILanguage {
public:
    virtual void load_file(std::string&& file_path) = 0;
    virtual void load_environment(std::vector<std::string>&& environment) = 0;
    virtual void execute() = 0;
};

}

#endif //WEBSERV_LANGMOD_SCRIPTREADER_HPP
