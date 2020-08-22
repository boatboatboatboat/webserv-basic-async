//
// Created by boat on 8/20/20.
//

#include "utils.hpp"
#include <vector>

// FIXME: illegal header
#include <sstream>

namespace utils {

/**
 * Trim a string.
 * @param str the string to trim.
 * @return the trimmed string.
 */
inline std::string trim(const std::string& str)
{
    auto first = str.find_first_not_of(' ');

    if (std::string::npos == first)
        return str;

    auto last = str.find_last_not_of(' ');

    return str.substr(first, (last - first + 1));
}

/**
 * Lowercase a string.
 * @param value a reference to the string to lowercase.
 * @return the lowercase string.
 */
inline std::string toLower(std::string& value)
{
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    return value;
}

/**
 * Split a string on a given delimiter.
 * @param source the string to split.
 * @param delimiter the delimiter to split on.
 * @return the delimited string in a vector.
 */
inline std::vector<std::string> split(const std::string& source, char delimiter)
{
    std::vector<std::string> strings;
    std::istringstream iss(source);
    std::string s;

    while (std::getline(iss, s, delimiter)) {
        strings.push_back(utils::trim(s));
    }

    return strings;
}
}
