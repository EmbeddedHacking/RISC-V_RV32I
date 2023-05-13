//----------------------------------------------------------------------------
//
//  Copyright Information...
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Module        : 3_stage_pipeline.h
//
// Description   : This file describes structure associated with solution of
//                 riscv 3-stage pipeline.
//----------------------------------------------------------------------------

#ifndef __3_STAGE_PIPELINE_H_
#define __3_STAGE_PIPELINE_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace riscv {

    class riscv_core
    {
    private:
        uint32_t       fetch_pos        = 0;       // fetch unit line number
        uint32_t       exec_pos         = 0;       // execution unit line number
        uint32_t       wr_back_pos      = 0;       // wr_back unit line number
        std::string    fetch_status     = "FREE";  // FREE : process , STALL : hold
        std::string    execution_status = "FREE";  // FREE : process , STALL : hold

        std::map<int, int>                     counter_map;
                                                   // keep track of counter for
                                                   // branch condition check
        std::map<std::string, int>             symbol_map;
                                                   // keep track of jump symbol
                                                   // line position
        std::vector<std::vector<std::string>>  parsed_data;
                                                   // holding parsed data in 2-D
                                                   // form of vector of strings

        void parser(char* file_name);
        void fetch(void);
        void execute(void);
        void write_back(void);

    public:
        uint32_t trigger_core(char* file_name);
    };

} // namespace riscv_core

#endif // __3_STAGE_PIPELINE_H_
