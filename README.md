# Vina MPI

<!-- TOC -->

- [Vina MPI](#vina-mpi)
  - [Introduction](#introduction)
  - [Command: vina-mpi-batch](#command-vina-mpi-batch)
    - [Compilation](#compilation)
    - [Usage:](#usage)
  - [Command: vina-srun](#command-vina-srun)
  - [Using Vina MPI on the Tara cluster](#using-vina-mpi-on-the-tara-cluster)
  - [Example](#example)

<!-- /TOC -->

## Introduction
Vina MPI is a MPI ready version of the docking software [Autodock Vina](http://vina.scripps.edu/).
Vina MPI parallelizes across multiple ligands, scheduling a new ligand each time there are available resources.

The execution model is comprised of a master process keeping track of progress
and scheduling tasks (MPI rank 0) and a collection of workers executing Autodock Vina.

MPI Vina produces to executables:
  - **_vina-mpi-batch_** the C++-MPI version of Vina
  - **_vina-srun_** a SLURM wrapper for **_vina-mpi-batch_**

## Command: vina-mpi-batch

#### Compilation
**_vina-mpi-batch_** requires the following libraries and programs:
  - OpenMPI 4.0 or later
  - Boost 1.65 or later
  - C++ compiler with -std=c++17 support, like g++ 9 or later
  - CMake 3.10 or later

To compile **_vina-mpi-batch_** `cd` to the root directory and perform:

```
mkdir -v build && cd build
cmake -DCMAKE_CXX_COMPILER=g++-9 ../
```

#### Usage:

```
vina-mpi-batch [--help] [--mpi-log-dir <dir-path>] [--std-out] [--std-err] \
               [--report-frequency num] --vina-ligand-dir <dir-path>       \
               --vina-out-dir <dir-path> [--vina-log-dir <dir-path>]       \
               [--vina-out-suffix <str>]                                   \
               vina <vina options>
```

The last set of arguments `vina <vina options>` corresponds to [Autodock Vina](http://vina.scripps.edu/) options, for more information use `vina-mpi-batch --help`.


Vina MPI options description:

```
  --help                                                    print help message
  --help-advanced                                           print advanced help message
  -O [ --std-out ]                                          print vina stdout to the logger
  -E [ --std-err ]                                          print vina stderr to the logger
  -p [ --print-clients ]                                    print clients logs to stdout
  -L [ --mpi-log-dir ] arg                                  directory to write the mpi logs
  -r [ --report-frequency ] arg (=60)                       print queue status every [r] seconds
  -i [ --vina-ligand-dir ] arg                              directory containing the ligands in PDBQT format
  -o [ --vina-out-dir ] arg (=vina-models)                  directory to write the vina output models (PDBQT)
  -l [ --vina-log-dir ] arg                                 directory to write the vina logs
  -s [ --vina-out-suffix ] arg                              output models (PDBQT) & logs suffix:
                                                            <ligand-name><suffix>.pdbqt, <ligand-name><suffix>.log
```

**The recommend set of options to run vina-mpi-batch with are:**

```
vina-mpi-batch --mpi-log-dir mpi-logs -E -p -r 240                 \
               --vina-ligand-dir <dir path to ligands>             \
               --vina-out-dir <dir path for storing results>       \
               --vina-log-dir <dir path for storing vina logs>     \
               --vina-out-suffix <suffix>                          \
               vina <vina options>
```

## Command: vina-srun
Usage:

```
nodes=<num of nodes>              \  # Number of SLURM nodes to use
threads=<num of threads>          \  # Number of threads that Vina will use
partition=<slurm partition>       \  # SLURM partition to use
time=<requested time>             \  # Max time for the run, format: hh:mm:ss
vina-srun <vina-mpi-batch-args>      # Args used in vina-mpi-batch
```

## Example
```
vina-srun -p -E -r 240 -L ./mpi-logs -i ./pdbqt -l ./vina-logs -s _out \
    vina --receptor receptor.pdbqt                                     \
    --center_x -4.169 --center_y -10.438 --center_z 14.527             \
    --size_x 30 --size_y 34 --size_z 32 --seed 1123581321
```
