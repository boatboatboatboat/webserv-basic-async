//
// Created by boat on 7/3/20.
//

#ifndef WEBSERV_MEM_COPY_HPP
#define WEBSERV_MEM_COPY_HPP

#include <cstddef>

namespace util {
	template<typename T>
	void mem_copy(T &destination, T &source) {
		if (&destination == &source)
			return;

		unsigned char* dest = reinterpret_cast<unsigned char*>(&destination);
		unsigned char* src = reinterpret_cast<unsigned char*>(&src);
		for (size_t n = 0; n < sizeof(T); n += 1) {
			dest[n] = src[n];
		}
	}
}

#endif //WEBSERV_MEM_COPY_HPP
