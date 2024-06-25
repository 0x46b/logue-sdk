/*
Copyright 2024 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include "unit_osc.h"

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),                  	// Size of this header. Leave as is.
    .target = UNIT_TARGET_PLATFORM | k_unit_module_osc,    	// Target platform and module pair for this unit
    .api = UNIT_API_VERSION,                               	// API version for which unit was built. See runtime.h
    .dev_id = 0x46b,                                       	// Developer ID. See https://github.com/korginc/logue-sdk/blob/master/developer_ids.md
    .unit_id = 0x0U,                                       	// ID for this unit. Scoped within the context of a given dev_id.
    .version = 0x00010000U,                                	// This unit's version: major.minor.patch (major<<16 minor<<8 patch).
    .name = "r4bb1t",                                      	// Name for this unit, will be displayed on device
    .num_params = 6,                                       	// Number of valid parameter descriptors. (max. 10)
    .params = {
        // Format:
        // min, max, center, default, type, frac. bits, frac. mode, <reserved>, name

        // See common/runtime.h for type enum and unit_param_t structure

        // Fixed/direct UI parameters
        // A knob
        {0, 1023, 0, 0, k_unit_param_type_none, 0, 0, 0, {"SHPE"}},

        // B knob
        {0, 15, 0, 0, k_unit_param_type_cents, 0, 0, 0, {"D3TN"}},

        // 8 Edit menu parameters
        {0, 2, 0, 1, k_unit_param_type_strings, 0, 0, 0, {"TYP1"}}, // Example of a strings type parameter
        {0, 2, 0, 1, k_unit_param_type_strings, 0, 0, 0, {"TYP2"}}, // Example of a strings type parameter
        {0, 6, 0, 0, k_unit_param_type_db, 0, 0, 0, {"LVL1"}},
        {0, 6, 0, 0, k_unit_param_type_db, 0, 0, 0, {"LVL2"}},
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}},
        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}}},
};
