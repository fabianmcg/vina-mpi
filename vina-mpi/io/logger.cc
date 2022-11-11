//============================================================================
// Name        : logger.cc
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#include <chrono>
#include "logger.hh"

namespace MPIBatch {
void Logger::clear() {
    __node_type__ = NodeType::unknown;
    __hostname__ = "";
    __rank__ = -1;
    __filepath__.clear();
    close();
    __std_out__ = false;
}

void Logger::create_directory() {
    if (!__filepath__.empty()) {
        auto parent = __filepath__.parent_path();
        if (!std::filesystem::exists(parent))
            __io__::create_directory(parent.filename());
    }
}

std::string Logger::date() {
    char buffer[80];
    std::time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto ti = std::localtime(&end_time);
    std::strftime(buffer, 80, "%T %d-%m-%Y %Z", ti);
    return std::string(buffer);
}

Logger::Logger(NodeType type, std::string hostname, int rank, bool std_out,
        std::ostream &stdout_stream, std::ostream &stderr_stream) :
        stdout_stream(stdout_stream), stderr_stream(stderr_stream) {
    set_info(type, hostname, rank);
    __std_out__ = std_out;
    __filepath__ = std::filesystem::path();
}

Logger::~Logger() {
    clear();
}

Logger::Logger(Logger &&other) :
        stdout_stream(other.stdout_stream), stderr_stream(other.stderr_stream) {
    set_info(other.__node_type__, other.__hostname__, other.__rank__);
    __std_out__ = other.__std_out__;
    __filepath__ = other.__filepath__;
    __file_openQ__ = other.__file_openQ__;
    other.clear();
}

Logger& Logger::operator=(Logger &&other) {
    set_info(other.__node_type__, other.__hostname__, other.__rank__);
    __std_out__ = other.__std_out__;
    __filepath__ = other.__filepath__;
    __file_openQ__ = other.__file_openQ__;
    other.clear();
    return *this;
}

void Logger::open(std::string file_path, std::ios_base::openmode openMode) {
    __filepath__ = std::filesystem::path(file_path);
    if (!file_path.empty()) {
        if (__file_openQ__)
            close();
        try {
            create_directory();
            file_stream = __io__::open <1>(__filepath__.string());
            __file_openQ__ = true;
        } catch (std::exception &exc) {
            stderr_stream << exc.what() << std::endl;
            __file_openQ__ = false;
        }
    } else {
        __filepath__.clear();
        __file_openQ__ = false;
    }
}

void Logger::close() {
    if (__file_openQ__) {
        try {
            if (file_stream.is_open())
                __io__::close(file_stream);
        } catch (std::exception &exc) {
            stderr_stream << exc.what() << std::endl;
        }
        __file_openQ__ = false;
    }
}

NodeType Logger::node_type() const {
    return __node_type__;
}

std::string Logger::hostname() const {
    return __hostname__;
}

int Logger::rank() const {
    return __rank__;
}

std::string Logger::filename() const {
    return __filepath__.string();
}

std::filesystem::path Logger::file_path() const {
    return __filepath__;
}

bool Logger::std_out() const {
    return __std_out__;
}

void Logger::set_info(NodeType type, std::string hostname, int rank) {
    __node_type__ = type;
    __hostname__ = hostname;
    __rank__ = rank;
}

void Logger::set_std_out(bool std_out) {
    __std_out__ = std_out;
}
}
