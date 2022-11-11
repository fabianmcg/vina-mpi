//============================================================================
// Name        : arguments.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#include "arguments.hh"

#include <iostream>
#include <map>
#include <variant>

#include "arguments.hh"

namespace MPIBatch {

int handle_vina_options(boost::program_options::parsed_options &parsed, vina_options_t &vina_opts,
        boost::program_options::variables_map &vm) {
    namespace po = boost::program_options;
    std::vector <std::string> opts = po::collect_unrecognized(parsed.options,
            po::include_positional);
    opts.erase(opts.begin());
    try {
        po::store( po::command_line_parser(opts)
        .options(vina_opts.desc)
        .style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing)
        .run(),
        vm);
        po::notify(vm);
    } catch (po::error_with_option_name &exc) {
        std::cerr << "Exception parsing command arguments: " << exc.what() << std::endl;
        std::cerr << std::endl;
        return 1;
    }
    return parse_conf_options(vina_opts.desc_config, vina_opts.desc_simple, vm, vina_opts.args);
}

void help(options_t& program_opts) {
    std::cout << program_opts.usage << std::endl;
    program_opts.batch_desc.print(std::cout, 60);
    program_opts.vina_opts.desc_simple.print(std::cout, 60);
    std::cout << std::endl;
}

void help_advanced(options_t& program_opts) {
    std::cout << program_opts.usage << std::endl;
    program_opts.batch_desc.print(std::cout);
    program_opts.vina_opts.desc.print(std::cout);
    std::cout << std::endl;
}

void init(options_t& program_opts) {
    namespace po = boost::program_options;
    program_opts.batch_options.add_options()
        ("help", "print help message")
        ("help-advanced", "print advanced help message")
        ("std-out,O", po::bool_switch(), "print vina stdout to the logger")
        ("std-err,E", po::bool_switch(), "print vina stderr to the logger")
        ("print-clients,p", po::bool_switch(), "print clients logs to stdout")
        ("mpi-log-dir,L",po::value <std::string>(), "directory to write the mpi logs")
        ("report-frequency,r", po::value <int>()->default_value(60), "print queue status every [r] seconds")
        ("vina-ligand-dir,i",po::value <std::string>(), "directory containing the ligands in PDBQT format")
        ("vina-out-dir,o",po::value <std::string>()->default_value("vina-models"), "directory to write the vina output models (PDBQT)")
        ("vina-log-dir,l",po::value <std::string>(), "directory to write the vina logs")
        ("vina-out-suffix,s",po::value <std::string>(), "output models (PDBQT) & logs suffix: <ligand-name><suffix>.pdbqt, <ligand-name><suffix>.log")
        ("vina", po::value <std::string>(), "")
        ("vina_args", po::value <std::vector <std::string>>(), "");

    program_opts.batch_desc.add_options()
        ("help", "print help message")
        ("help-advanced", "print advanced help message")
        ("std-out,O", po::bool_switch(), "print vina stdout to the logger")
        ("std-err,E", po::bool_switch(), "print vina stderr to the logger")
        ("print-clients,p", po::bool_switch(), "print clients logs to stdout")
        ("mpi-log-dir,L",po::value <std::string>(), "directory to write the mpi logs")
        ("report-frequency,r", po::value <int>()->default_value(60), "print queue status every [r] seconds")
        ("vina-ligand-dir,i", po::value <std::string>(), "directory containing the ligands in PDBQT format")
        ("vina-out-dir,o", po::value <std::string>()->default_value("vina-models"), "directory to write the vina output models (PDBQT)")
        ("vina-log-dir,l", po::value <std::string>(), "directory to write the vina logs")
        ("vina-out-suffix,s",po::value <std::string>(), "output models (PDBQT) & logs suffix: <ligand-name><suffix>.pdbqt, <ligand-name><suffix>.log");

    program_opts.positional.add("vina", 1).add("vina_args", -1);
}

int argument_parse(int argc, char *argv[], options_t &program_opts) {
    namespace po = boost::program_options;
    init(program_opts);
    __vina__::vina_options(program_opts.vina_opts.desc, program_opts.vina_opts.desc_config,
            program_opts.vina_opts.desc_simple, program_opts.vina_opts.search_area,
            program_opts.vina_opts.args);
    try {
        po::parsed_options parsed = po::command_line_parser(argc, argv)
            .options(program_opts.batch_options)
            .style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing)
            .positional(program_opts.positional)
            .allow_unregistered()
            .run();
        po::store(parsed, program_opts.vm);
        po::notify(program_opts.vm);
        if (program_opts.vm.count("help")) {
            help(program_opts);
            return 1;
        }
        if (program_opts.vm.count("help-advanced")) {
            help_advanced(program_opts);
            return 1;
        }
        if (program_opts.vm.count("vina-ligand-dir") < 1) {
            std::cout << "Error, missing option --vina-ligand-dir dir" << std::endl;
            help(program_opts);
            return 1;
        }
        if (program_opts.vm.count("vina") > 0) {
            std::string cmd = program_opts.vm["vina"].as <std::string>();
            if (cmd == "vina") {
                if (handle_vina_options(parsed, program_opts.vina_opts, program_opts.vm_vina)
                        != 0) {
                    help(program_opts);
                    return 1;
                }
            } else {
                help(program_opts);
                return 1;
            }
        } else {
            help(program_opts);
            return 1;
        }
    } catch (std::exception &exc) {
        std::cerr << "Exception parsing command arguments: " << exc.what() << std::endl;
        std::cerr << program_opts.usage << std::endl;
        std::cerr << std::endl;
        return 1;
    }
    return 0;
}
}
