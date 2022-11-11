//============================================================================
// Name        : vina_util.cc
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <regex>
#include <string>
#include "vina_util.hh"

namespace MPIBatch {
void init_dirs(options_t &options) {
    namespace fs = std::filesystem;
    if (options.vm.count("vina-out-dir") > 0) {
        std::string dir_name = options.vm["vina-out-dir"].as <std::string>();
        if (!fs::exists(fs::path(dir_name)))
            fs::create_directories(std::filesystem::path(dir_name));
    }
    if (options.vm.count("vina-log-dir") > 0) {
        std::string dir_name = options.vm["vina-log-dir"].as <std::string>();
        if (!fs::exists(fs::path(dir_name)))
            fs::create_directories(std::filesystem::path(dir_name));
    }
    if (options.vm.count("mpi-log-dir") > 0) {
        std::string dir_name = options.vm["mpi-log-dir"].as <std::string>();
        if (!fs::exists(fs::path(dir_name)))
            fs::create_directories(std::filesystem::path(dir_name));
    }
}

std::vector <std::string> pdbqt_files(options_t &options) {
    namespace fs = std::filesystem;
    std::vector <std::string> files;
    if (options.vm.count("vina-ligand-dir") > 0) {
        fs::path path = options.vm["vina-ligand-dir"].as <std::string>();
        if (fs::is_directory(path)) {
            for (auto &p : fs::directory_iterator(path)) {
                std::string ext = p.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
                    return std::tolower(c);
                });
                if (ext == ".pdbqt")
                    files.push_back(p.path().string());
            }
        }
    }
    return files;
}

std::vector <std::string> init_vina_files(options_t &options) {
    std::vector <std::string> files = pdbqt_files(options);
    if (!files.empty())
        init_dirs(options);
    return files;
}
}
