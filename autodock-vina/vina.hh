/*

 Copyright (c) 2006-2010, The Scripps Research Institute

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 Author: Dr. Oleg Trott <ot14@columbia.edu>,
 The Olson Lab,
 The Scripps Research Institute

 */

#ifndef __VINA_HH__
#define __VINA_HH__

#include <string>
#include <boost/program_options.hpp>
#include "std-out.hh"
#include "common.h"

namespace __vina__ {
using vina_options_desc_t = boost::program_options::options_description;
using variables_map = boost::program_options::variables_map;

struct vina_args_t {
    std::string rigid_name, ligand_name, flex_name, config_name, out_name, log_name;
    fl center_x, center_y, center_z, size_x, size_y, size_z;
    int cpu = 0, seed, exhaustiveness, verbosity = 2, num_modes = 9;
    fl energy_range = 2.0;

    // -0.035579, -0.005156, 0.840245, -0.035069, -0.587439, 0.05846
    fl weight_gauss1 = -0.035579;
    fl weight_gauss2 = -0.005156;
    fl weight_repulsion = 0.840245;
    fl weight_hydrophobic = -0.035069;
    fl weight_hydrogen = -0.587439;
    fl weight_rot = 0.05846;
    bool score_only = false, local_only = false, randomize_only = false, help = false,
            help_advanced = false, version = false, ligand_Q = false;
};

void vina_options(vina_options_desc_t &desc, vina_options_desc_t &desc_config, vina_options_desc_t &desc_simple,
        vina_options_desc_t &search_area, vina_args_t &args);

int parse_conf_options(vina_options_desc_t &desc_config, vina_options_desc_t &desc_simple, variables_map &vm,
        vina_args_t &args);

int run(vina_options_desc_t &desc, vina_options_desc_t &desc_config, vina_options_desc_t &desc_simple,
        vina_options_desc_t &search_area, variables_map &vm, vina_args_t &args);
}
#endif
