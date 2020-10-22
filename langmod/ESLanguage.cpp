//
// Created by boat on 19-10-20.
//

#include "ESLanguage.hpp"
#include "es/duk_print_alert.h"
#include <fcntl.h>
namespace langmod {

void ESLanguage::load_file(std::string&& path)
{
    _file_path = move(path);
    _context = duk_create_heap_default();
    if (_context == nullptr) {
        throw std::runtime_error("Duktape heap creation failed");
    }
    duk_print_alert_init(_context, 0);
}

void ESLanguage::load_environment(std::vector<std::string>&& environment)
{
    (void)environment;
}

void ESLanguage::execute()
{
    // the previous select loop has been invalidated
    // file i/o is always blocking on basically every OS used in practice
    // a select loop for reading out a file is a no-op
    int fd = open(_file_path.c_str(), O_RDONLY);
    if (fd < 0) {
        throw std::runtime_error("failed to open file");
    }
    int res;
    char buf[8192];
    std::string builder;
    do {
        res = read(fd, buf, sizeof(buf));
        if (res == -1) {
            throw std::runtime_error("failed to read file");
        }
        builder.append(std::string_view(buf, res));
    } while (res > 0);
    close(fd);
    duk_eval_lstring(_context, builder.data(), builder.length());
    exit(0);
}

}