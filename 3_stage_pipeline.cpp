//----------------------------------------------------------------------------
//
//  Copyright Information...
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Module           : 3_stage_pipeline.cpp
//
// Description      : This file implements the solution of riscv 3-stage
//                    pipeline by creating fetch, execute and write_back unit.
//
// Platforms        : OS independent
//
// Compiler Options : N/A
//
// Linked Options   : N/A
//----------------------------------------------------------------------------

#include "3_stage_pipeline.h"

namespace riscv {

    //----------------------------------------------------------------------------
    // Description  : This method provides interface to trigger riscv_core.
    //
    // Inputs       : file_name   // input file holding riscv assembly code
    //
    // Outputs      : N/A
    //
    // Return value : clock_num   // total clock taken for core execution
    //----------------------------------------------------------------------------
    uint32_t riscv_core::trigger_core(char* file_name)
    {
        uint32_t clock_num = 0;

        parser(file_name);

#if DEBUG_ENABLE
        std::cout << "clock_num, " << "fetch, " << "execute, " << "write_back, ";
        std::cout << "fetch_status, " << "execution_status" << "\t";
        std::cout << std::endl << std::endl;
#endif

        while (wr_back_pos != (parsed_data.size() - 1))
        {

#if DEBUG_ENABLE
            std::cout << clock_num << "\t" << parsed_data[fetch_pos][1] << "\t";
            std::cout << parsed_data[exec_pos][1] << "\t";
            std::cout << parsed_data[wr_back_pos][1] << "\t";
            std::cout << fetch_status << "\t" << execution_status << std::endl;
#endif

            write_back();
            execute();
            fetch();
            clock_num++;
        }

#if DEBUG_ENABLE
        std::cout << clock_num << "\t" << parsed_data[fetch_pos][1] << "\t";
        std::cout << parsed_data[exec_pos][1] << "\t";
        std::cout << parsed_data[wr_back_pos][1] << "\t";
        std::cout << fetch_status << "\t" << execution_status;
        std::cout << std::endl << std::endl;
#endif

        return clock_num;
    }

    //----------------------------------------------------------------------------
    // Description  : This method parses input assembly file to creates kind of
    //                2D-array paresed_data considering split logic on ' '.
    //                It also create symbol_map and counter_map to keep trake of
    //                symbol line number as well as conditional counter status
    //                while executing condition check or branch instructions.
    //
    // Inputs       : file_name   // input file holding riscv assembly code
    //
    // Outputs      : N/A
    //
    // Return value : N/A
    //----------------------------------------------------------------------------
    void riscv_core::parser(char* file_name)
    {
        std::fstream newfile;
        newfile.open(file_name, std::ios::in);

        if (newfile.is_open())
        {
            std::string  line;
            uint32_t     line_num = 0;

            while (getline(newfile, line))
            {
                std::stringstream          linestream(line);
                std::string                data;
                std::vector<std::string>   parsed_line;

                getline(linestream, data, ' ');

                if (data.find(':') == std::string::npos)  // true if no address symbol
                {
                    if (data == "bge" || data == "ble")
                    {
                        counter_map[line_num] = 0;        // counter for branch instruction
                    }
                    parsed_line.push_back(" ");
                }
                else                                      // true if has address symbol
                {
                    data.pop_back();                      // removing ":" from word
                    symbol_map[data] = line_num;          // symbol map to jump
                }

                parsed_line.push_back(data);

                while (getline(linestream, data, ' '))
                {
                    data = data.substr(0, data.find(','));
                    parsed_line.push_back(data);          // removing "," from word

                    if (data == "bge" || data == "ble")
                    {
                        counter_map[line_num] = 0;        // counter for branch instruction
                    }
                }

                parsed_line.back().pop_back();            // fix for linux (g++)
                parsed_data.push_back(parsed_line);
                line_num++;
            }
            newfile.close();
        }
    }

    //----------------------------------------------------------------------------
    // Description  : This method implements fetch unit which fetches an
    //                instruction from the Instruction Buffer, decodes it, and
    //                then fetches 2 associated operands from the Register File.
    //
    //                Implementation..
    //                Condition to "STALL" fetch unit:
    //                    1) when execution unit status is "STALL"
    //                    2) when current operation using dependent operands
    //                    3) when fetching "jal", "bge" or "ble" instruction
    //                If not "STALL" and FREE to fetch:
    //                    1) design for branch or condition check instruction
    //                    2) design for normal instruction, increment PC register
    //
    // Inputs       : N/A
    // 
    // Outputs      : N/A
    //
    // Return value : N/A
    //----------------------------------------------------------------------------
    void riscv_core::fetch(void)
    {
        if (execution_status == "STALL")
        {
            fetch_status = "STALL";
        }
        else
        {
            // loop to find out operand dependancy on previous instruction and
            // stall fetch execution till dependancy resolves
            for (uint32_t pos = wr_back_pos; pos < fetch_pos; pos++)
            {
                if (parsed_data[pos][1] == "bge" || parsed_data[pos][1] == "ble")
                {
                    continue;   // avoiding operand dependacy on previous branch inst.
                }

                for (uint32_t index = 0; index < parsed_data[fetch_pos].size(); index++)
                {
                    if (parsed_data[fetch_pos][index].find(parsed_data[pos][2])
                        != std::string::npos)
                    {
                        fetch_status = "STALL";
                        return; // if operand dependent than wait till dependancy resolve.
                    }
                }
            }

            // stall fetch execution unit when jump or branch instructions
            // condition check become true
            if (parsed_data[fetch_pos][1] == "jal")
            {
                if (parsed_data[wr_back_pos][1] != "jal")
                {
                    fetch_status = "STALL";
                }
            }
            else if (parsed_data[fetch_pos][1] == "bge")
            {
                if (counter_map[fetch_pos] >= stoi(parsed_data[fetch_pos][3]))
                {
                    if (parsed_data[wr_back_pos][1] != "bge")
                    {
                        fetch_status = "STALL";
                    }
                }
            }
            else if (parsed_data[fetch_pos][1] == "ble")
            {
                if (counter_map[fetch_pos] <= stoi(parsed_data[fetch_pos][3]))
                {
                    if (parsed_data[wr_back_pos][1] != "ble")
                    {
                        fetch_status = "STALL";
                    }
                }
            }
        }

        // processing fetch unit when status is FREE
        if (fetch_status == "FREE" && fetch_pos < (parsed_data.size() - 1))
        {
            if (parsed_data[fetch_pos][1] == "jal")
            {
                fetch_pos = symbol_map[parsed_data[fetch_pos][3]];
            }
            else if (parsed_data[fetch_pos][1] == "bge")
            {
                if (counter_map[fetch_pos] >= stoi(parsed_data[fetch_pos][3]))
                {
                    counter_map[fetch_pos] = 0;
                    fetch_pos = symbol_map[parsed_data[fetch_pos][4]];
                }
                else
                {
                    counter_map[fetch_pos] += 1;
                    fetch_pos++;
                }
            }
            else if (parsed_data[fetch_pos][1] == "ble")
            {
                if (counter_map[fetch_pos] <= stoi(parsed_data[fetch_pos][3]))
                {
                    counter_map[fetch_pos] += 1;
                    fetch_pos = symbol_map[parsed_data[fetch_pos][4]];
                }
                else
                {
                    counter_map[fetch_pos] = 0;
                    fetch_pos++;
                }
            }
            else
            {
                fetch_pos++;
            }
        }

        return;
    }

    //----------------------------------------------------------------------------
    // Description  : This method implements execution unit which also considers
    //                Load/Store Unit delay.
    //
    //                Implementation..
    //                Condition to "STALL" execution unit:
    //                    1) when current operation using Load/Store Unit than
    //                       add 1 cycle delay by puting process status on "STALL"
    //                If "STALL" or FREE:
    //                    1) jump to next instruction which already fetched
    //
    // Inputs       : N/A
    //
    // Outputs      : N/A
    //
    // Return value : N/A
    //----------------------------------------------------------------------------
    void riscv_core::execute(void)
    {
        if (execution_status == "STALL")  // To add 1 cycle delay for Load/Store
        {
            execution_status = "FREE";
            exec_pos = fetch_pos;
        }
        else                              // processing execution unit on status FREE
        {
            if (parsed_data[exec_pos].back().find("0x") != std::string::npos)
            {
                execution_status = "STALL";
            }
            else if (parsed_data[exec_pos][1] == "lw" ||
                     parsed_data[exec_pos][1] == "sw")
            {
                execution_status = "STALL";
            }
            else
            {
                exec_pos = fetch_pos;
            }
        }

        return;
    }

    //----------------------------------------------------------------------------
    // Description  : This method implements write_back stage which write a single
    //                operand to the Register File each cycle.
    // 
    //                Condition to "FREE" fetch unit:
    //                    1) when execution unit is stall due to write_back unit
    //                       providing back_pressure
    //                    2) if fetch_status == "STALL" due to operand dependancy
    //                       and waiting to complete write_back stage
    //
    // Inputs       : N/A
    //
    // Outputs      : N/A
    //
    // Return value : N/A
    //----------------------------------------------------------------------------
    void riscv_core::write_back(void)
    {
        // condition to free fetch unit which is in "STALL" state due to resource
        // dependancy or due to jump/branch instruction execution
        if ((wr_back_pos == exec_pos) &&
            ((fetch_status == "STALL") || (execution_status == "STALL")))
        {
            fetch_status = "FREE";
        }

        wr_back_pos = exec_pos;
    }

} // namespace riscv
