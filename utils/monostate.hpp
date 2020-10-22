//
// Created by boat on 17-10-20.
//

#ifndef WEBSERV_UTILS_MONOSTATE_HPP
#define WEBSERV_UTILS_MONOSTATE_HPP

namespace utils {

struct monostate {
};

constexpr auto operator==(monostate, monostate) -> bool { return true; }
constexpr auto operator!=(monostate, monostate) -> bool { return false; }
constexpr auto operator>(monostate, monostate) -> bool { return false; }
constexpr auto operator<(monostate, monostate) -> bool { return false; }
constexpr auto operator<=(monostate, monostate) -> bool { return true; }
constexpr auto operator>=(monostate, monostate) -> bool { return true; }

}
#endif //WEBSERV_UTILS_MONOSTATE_HPP
