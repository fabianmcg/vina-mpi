//============================================================================
// Name        : io.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __IO_HH__
#define __IO_HH__

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
#include <filesystem>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "../definitions.hh"

namespace __io__ {
namespace __private__ {
template <size_t i = 0, typename T0 = void, typename ...T,
        std::enable_if_t <(i < sizeof...(T)), int> = 0>
std::ostream& print(const std::tuple <T0, T...> &tuple, std::ostream &ost, std::string separator);

template <size_t i, typename T0, typename ...T, std::enable_if_t <(i >= sizeof...(T)), int> = 0>
std::ostream& print(const std::tuple <T0, T...> &tuple, std::ostream &ost, std::string separator);

template <size_t i = 0, typename T0 = void, typename ...T,
        std::enable_if_t <(i < sizeof...(T)), int> = 0>
std::ostream& print(const std::tuple <T0, T...> &tuple, std::ostream &ost, std::string separator) {
    ost << std::get <i>(tuple) << separator;
    return print <i + 1>(tuple, ost, separator);
}

template <size_t i, typename T0, typename ...T, std::enable_if_t <(i >= sizeof...(T)), int> = 0>
std::ostream& print(const std::tuple <T0, T...> &tuple, std::ostream &ost, std::string separator) {
    ost << std::get <i>(tuple);
    return ost;
}

}
template <typename T, typename U = T>
std::ostream& print(const std::vector <T> &v, std::ostream &ost, std::size_t size =
        std::numeric_limits <size_t>::max(), std::string separator = "\n", std::string begin = "",
        std::string end = "") {
    size = std::min(size, v.size());
    ost << begin;
    for (size_t i = 0; i < size; ++i) {
        if (i < (size - 1))
            ost << static_cast <U>(v[i]) << separator;
        else
            ost << static_cast <U>(v[i]);
    }
    ost << end;
    return ost;
}

template <typename K, typename V, typename C>
std::ostream& print(const std::map <K, V, C> &map, std::ostream &ost, std::size_t size =
        std::numeric_limits <size_t>::max(), std::string separator = "\n", std::string begin = "",
        std::string end = "",
        std::function <void(std::ostream&, K, V)> pprinter = [](std::ostream &__ost__, K key,
                V val) -> void {
            __ost__ << "[" << key << "]->" << val;
        }) {
    size = std::min(size, map.size());
    ost << begin;
    size_t c = 0;
    for (typename std::map <K, V, C>::const_iterator i = map.cbegin();
            i != map.cend() && (c < size); ++i) {
        pprinter(ost, i->first, i->second);
        if ((c++) != (map.size() - 1))
            ost << separator;
    }
    ost << end;
    return ost;
}

template <typename T>
std::ostream& print(const std::set <T> &set, std::ostream &ost, std::size_t size =
        std::numeric_limits <size_t>::max(), std::string separator = ", ", std::string begin = "{",
        std::string end = "}") {
    size = std::min(size, set.size());
    size_t k = 0;
    ost << begin;
    for (auto i = set.cbegin(); i != set.cend() && (k < size); ++i) {
        if (k == (size - 1))
            ost << (*i);
        else
            ost << (*i) << separator;
        ++k;
    }
    ost << end;
    return ost;
}

template <typename T0, typename ...T>
std::ostream& print(const std::tuple <T0, T...> &tuple, std::ostream &ost, std::string separator =
        ", ", std::string begin = "{", std::string end = "}") {
    ost << begin;
    __private__::print <0>(tuple, ost, separator);
    ost << end;
    return ost;
}

inline std::ostream& print(std::ostream &ost, const std::string &separator) {
    return ost;
}

template <typename T0, typename ...T>
std::ostream& print(std::ostream &ost, const std::string &separator, const T0 &arg0,
        const T &... args) {
    if (sizeof...(T) == 0) {
        ost << arg0;
        return ost;
    } else {
        ost << arg0 << separator;
        return print(ost, separator, args...);
    }
}

template <typename T>
std::ostream& operator<<(std::ostream &ost, const std::vector <T> &vector) {
    return print(vector, ost, vector.size());
}

template <typename K, typename V, typename C>
std::ostream& operator<<(std::ostream &ost, const std::map <K, V, C> &map) {
    return print(map, ost);
}

template <typename U, typename V>
std::ostream& operator<<(std::ostream &ost, const std::pair <U, V> &pair) {
    ost << "{" << pair.first << ", " << pair.second << "}";
    return ost;
}

template <typename T>
std::ostream& operator<<(std::ostream &ost, const std::set <T> &set) {
    return print(set, ost);
}

template <typename T0, typename ...T>
std::ostream& operator<<(std::ostream &ost, const std::tuple <T0, T...> &tuple) {
    return print(tuple, ost);
}

std::ostream& operator<<(std::ostream &ost, const MPIBatch::NodeType &type);

std::ostream& operator<<(std::ostream &ost, const MPIBatch::LogType &type);

template <unsigned int mode = 0, typename std::enable_if <mode == 0, int>::type = 0>
std::ifstream open(std::string file_name, std::ios_base::openmode openMode = std::ios_base::in) {
    std::ifstream file;
    file.open(file_name, openMode);
    if (file.is_open())
        return file;
    else
        throw(std::ios_base::failure("Couldn't open the file: " + file_name));
}

template <unsigned int mode = 0, typename std::enable_if <mode == 1, int>::type = 0>
std::ofstream open(std::string file_name, std::ios_base::openmode openMode = std::ios_base::out) {
    std::ofstream file;
    file.open(file_name, openMode);
    if (file.is_open())
        return file;
    else
        throw(std::ios_base::failure("Couldn't open the file: " + file_name));
}

template <unsigned int mode = 0, typename std::enable_if <(mode >= 2), int>::type = 0>
std::fstream open(std::string file_name,
        std::ios_base::openmode openMode = std::ios_base::in | std::ios_base::out) {
    std::fstream file;
    file.open(file_name, openMode);
    if (file.is_open())
        return std::move(file);
    else
        throw(std::ios_base::failure("Couldn't open the file: " + file_name));
}

void close(std::ifstream &file);

void close(std::ofstream &file);

void close(std::fstream &file);

std::vector <std::string> read(const std::string &file_name);

void write(const std::vector <std::string> &lines, const std::string &file_name);

bool create_directory(const std::string &file_name);

bool remove_file(const std::string &file_name);

bool remove_directory(const std::string &file_name);

template <typename T>
std::string to_string(const T &value) {
    std::ostringstream ost;
    ost << value;
    return ost.str();
}

std::vector <std::string> to_lines(const std::string &string);
}
#endif
