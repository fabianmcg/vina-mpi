//============================================================================
// Name        : arguments.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __ARGUMENTS_HH__
#define __ARGUMENTS_HH__

#include <utility>
#include <boost/program_options.hpp>

#include "vina.hh"
namespace MPIBatch {

struct vina_options_t {
    __vina__::vina_options_desc_t desc;
    __vina__::vina_options_desc_t desc_config;
    __vina__::vina_options_desc_t desc_simple;
    __vina__::vina_options_desc_t search_area = __vina__::vina_options_desc_t("Search area (required, except with --score_only)", 120);
    __vina__::vina_args_t args;
};

struct options_t {
    boost::program_options::options_description batch_options =
            boost::program_options::options_description("Vina MPI options");
    boost::program_options::options_description batch_desc =
            boost::program_options::options_description("Vina MPI options", 120);
    boost::program_options::positional_options_description positional;
    const std::string usage =
            "Usage: ./program [--help] [--mpi-log-dir <dir-path>] [--std-out] [--std-err] [--report-frequency num] \\\n"
            "\t\t--vina-ligand-dir <dir-path> [--vina-out-dir <dir-path>] [--vina-log-dir <dir-path>] [--vina-out-suffix <str>] \\\n\t\t vina <vina options>";
    vina_options_t vina_opts;
    boost::program_options::variables_map vm;
    boost::program_options::variables_map vm_vina;
};

void help(options_t& program_opts);
void help_advanced(options_t& program_opts);
void init(options_t& program_opts);
int argument_parse(int argc, char *argv[], options_t& program_opts);

}
#endif
