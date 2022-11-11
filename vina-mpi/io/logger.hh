//============================================================================
// Name        : logger.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __IO_LOGGER_HH__
#define __IO_LOGGER_HH__

#include <filesystem>
#include <iostream>
#include <string>
#include <iomanip>

#include "../definitions.hh"
#include "io.hh"

namespace MPIBatch {
class Logger {
private:
    NodeType __node_type__;
    std::string __hostname__;
    int __rank__;
    std::filesystem::path __filepath__;
    std::ofstream file_stream;
    bool __std_out__;
    std::ostream &stdout_stream;
    std::ostream &stderr_stream;
    bool __file_openQ__ = false;

    void clear();
    void create_directory();
    std::string date();
public:
    Logger(NodeType type = NodeType::unknown, std::string hostname = "", int rank = -1,
            bool std_out = true, std::ostream &stdout_stream = std::cout,
            std::ostream &stderr_stream = std::cerr);
    ~Logger();
    Logger(Logger &&other);
    Logger(const Logger &other) = delete;
    Logger& operator=(Logger &&other);
    Logger& operator=(const Logger &other) = delete;

    template <typename ... Args>
    void operator()(LogType log_type, const Args &... args);

    void open(std::string file_path, std::ios_base::openmode openMode = std::ios_base::out);
    void close();

    NodeType node_type() const;
    std::string hostname() const;
    int rank() const;
    std::string filename() const;
    std::filesystem::path file_path() const;
    bool std_out() const;

    void set_info(NodeType type, std::string hostname, int rank);
    void set_std_out(bool std_out);

    template <typename ... Args>
    void log(LogType log_type, const Args &... args);
};
}

template <typename ... Args>
inline void MPIBatch::Logger::log(LogType log_type, const Args &... args) {
    if (__file_openQ__ || __std_out__) {
        using __io__::operator <<;
        std::ostringstream ost;
        ost.fill('0');
        ost << "[" << log_type << "]";
        ost << "[" << __node_type__ << "]";
        ost << "[" << __hostname__ << "]";
        ost << "[" << std::setw(2) << __rank__ << "]";
        ost << "[" << date() << "]";
        ost << " ";
        __io__::print(ost, "", args...) << std::endl;
        std::string msg = ost.str();
        if (__file_openQ__ && file_stream.is_open())
            file_stream << msg;
        if (__std_out__) {
            if (log_type == LogType::error)
                stderr_stream << msg;
            else
                stdout_stream << msg;
        }
    }
}

template <typename ... Args>
inline void MPIBatch::Logger::operator ()(LogType log_type, const Args &... args) {
    log(log_type, args...);
}

#endif
