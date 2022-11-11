//============================================================================
// Name        : vina-mpi.cc
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#include <chrono>
#include <iostream>
#include "vina-mpi/mpi_batch.hh"

using namespace std;

void master(MPIBatch::options_t &opts) {
    using namespace MPIBatch;
    std::vector <std::string> pdbqts = init_vina_files(opts);
    try {
        StringSerializer serializer;
        MasterProcess <task_queue <std::string>, ServerMode::autocontained> master;
        if (!pdbqts.empty())
            master.queue().insert(pdbqts.begin(), pdbqts.end());
        if (opts.vm.count("mpi-log-dir") > 0) {
            std::filesystem::path log_dir = std::filesystem::path(
                    opts.vm["mpi-log-dir"].as <std::string>()).string();
            log_dir /= master.logger().hostname() + "-" + std::to_string(master.logger().rank())
                    + ".log";
            master.logger().open(log_dir.string());
        }
        int report = opts.vm["report-frequency"].as <int>();
        if (report < 0)
            report = 0;
        time_point start = now();
        master.run(serializer, std::chrono::seconds(report));
        master.logger()(LogType::trace, "Total execution time: ", display_duration(elapsed(now(), start)));
        if (opts.vm.count("mpi-log-dir") > 0) {
            master.logger().close();
        }
    } catch (std::exception &exc) {
        std::cerr << exc.what() << std::endl;
    }
}

void worker(MPIBatch::options_t &opts) {
    using namespace MPIBatch;
    bool outQ = opts.vm["std-out"].as <bool>();
    bool errQ = opts.vm["std-err"].as <bool>();
    bool logQ = opts.vm.count("vina-log-dir") > 0;
    std::string suffix;
    std::string receptor;
    if (opts.vm.count("vina-out-suffix") > 0)
        suffix = opts.vm["vina-out-suffix"].as <std::string>();
    if (opts.vm_vina.count("receptor") > 0)
        receptor = std::filesystem::path(opts.vina_opts.args.rigid_name).stem().string();
    auto task = [&opts, outQ, errQ, logQ, suffix, receptor](Logger &logger, std::string str,
            WorkerStatus *status) {
        try {
            std::filesystem::path out_path = std::filesystem::path(
                    opts.vm["vina-out-dir"].as <std::string>());
            std::filesystem::path ligand_path = std::filesystem::path(str);
            std::string ligand_name = ligand_path.stem().string();
            out_path /= ligand_name + suffix + ".pdbqt";
            if (logQ) {
                std::filesystem::path log_path = std::filesystem::path(
                        opts.vm["vina-log-dir"].as <std::string>());
                log_path /= ligand_name + suffix + ".log";
                opts.vina_opts.args.log_name = log_path.string();
            } else
                opts.vina_opts.args.log_name = "";
            opts.vina_opts.args.out_name = out_path.string();
            opts.vina_opts.args.ligand_name = str;
            std::string msg = "ligand: " + ligand_name;
            if (!receptor.empty())
                msg += " & receptor: " + receptor;
            vina_std_out.str("");
            vina_std_err.str("");
            time_point start = now();
            int err = __vina__::run(opts.vina_opts.desc, opts.vina_opts.desc_config,
                    opts.vina_opts.desc_simple, opts.vina_opts.search_area, opts.vm_vina,
                    opts.vina_opts.args);
            duration eps = elapsed(now(), start);
            if (err == 0) {
                if (outQ) {
                    logger(LogType::trace, "Vina stdout for ", msg, "\n", vina_std_out.str());
                }
                logger(LogType::trace, "Execution time for ", msg, ": ", display_duration(eps));
            } else {
                logger(LogType::warn, "Vina stderr for ", msg, "\n", vina_std_err.str());
            }
        } catch (std::exception &exc) {
            logger(LogType::error, "Vina unexpectedly failed, error message: ", exc.what());
        }
        *status = WorkerStatus::finished;
    };
    task_container <void(Logger&, std::string, WorkerStatus*)> container;
    container.task = task;
    try {
        StringSerializer serializer;
        WorkerProcess <ServerMode::autocontained> worker;
        if (opts.vm.count("mpi-log-dir") > 0) {
            std::filesystem::path log_dir = std::filesystem::path(
                    opts.vm["mpi-log-dir"].as <std::string>()).string();
            log_dir /= worker.logger().hostname() + "-" + std::to_string(worker.logger().rank())
                    + ".log";
            worker.logger().open(log_dir.string());
        }
        worker.logger().set_std_out(opts.vm["print-clients"].as <bool>());
        worker.run(serializer, container);
        if (opts.vm.count("mpi-log-dir") > 0) {
            worker.logger().close();
        }
    } catch (std::exception &exc) {
        std::cerr << exc.what() << std::endl;
    }
}

int main(int argc, char *argv[]) {
    using namespace MPIBatch;
    using namespace __io__;
    options_t opts;
    try {
        auto args = argument_parse(argc, argv, opts);
        if (args == 0) {
            int rank;
            MPI_Init(&argc, &argv);
            MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
            MPI_Comm_rank(MPI_COMM_WORLD, &rank);
            if (rank == 0) {
                master(opts);
            } else {
                worker(opts);
            }
            MPI_Finalize();
        }
    } catch (std::exception &exc) {
        std::cerr << "An exception has occurred, msg: " << exc.what() << std::endl;
    }
    return 0;
}
