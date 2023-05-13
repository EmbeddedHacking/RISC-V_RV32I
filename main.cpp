//----------------------------------------------------------------------------
//
//  Copyright Information...
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Module           : main.cpp
//
// Description      : This file has implementation needed for functionality
//                    validation of riscv 3-stage pipeline module.
//
// Platforms        : OS independent
//
// Compiler Options : N/A
//
// Linked Options   : N/A
//----------------------------------------------------------------------------

#include <iostream>
#include "3_stage_pipeline.h"

using namespace riscv;

//----------------------------------------------------------------------------
// Description  : This method create riscv_core test object to showcase
//                3-stage pipeline test scenario.
//
// Inputs       : argc      // number of command line arguments
//                argv      // character array holds command line arguments
//
// Outputs      : N/A
//
// Return value : Error or Warning status
//----------------------------------------------------------------------------
int32_t main(int argc, char** argv)
{
    riscv_core riscvObj;

    uint32_t total_clock = riscvObj.trigger_core(argv[1]);

    std::cout << "total_clock = " << total_clock + 1 << std::endl;
    return 0;
}
