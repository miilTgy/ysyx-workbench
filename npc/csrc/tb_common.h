/*
 * @Author: Zeng GuangYi tgy_scut2021@outlook.com
 * @Date: 2025-01-15 20:31:21
 * @LastEditors: Zeng GuangYi tgy_scut2021@outlook.com
 * @LastEditTime: 2025-01-15 23:33:43
 * @FilePath: /npc/csrc/tb_common.h
 * @Description: Common Verilator testbench headder
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */
#ifndef __TB_COM__ // dependency defender
#define __TB_COM__

#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <typeinfo>
#include <string>
#include <random>

#define step(tb, statements) do { \
            tb->set_clk(0); \
            tb->dut->eval(); \
            tb->contextp->timeInc(1); \
            tb->m_trace->dump(tb->contextp->time()); \
            tb->set_clk(1); \
            tb->dut->eval(); \
            {statements} \
            tb->dut->eval(); \
            tb->contextp->timeInc(1); \
            tb->m_trace->dump(tb->contextp->time()); \
        } while (0)


enum Module_type {
    sequential, combinational
};

template<class MODULE> class TESTBENCH {
private:
    Module_type module_type;
    
public:
    MODULE *dut;
    VerilatedContext *contextp;
    VerilatedVcdC *m_trace;

    TESTBENCH(int argc, char *argv[]) {
        std::cout << "start constructiog" << std::endl;
        Verilated::traceEverOn(true);
        Verilated::commandArgs(argc, argv);
        dut = new MODULE;
        contextp = new VerilatedContext;
        m_trace = new VerilatedVcdC;

        contextp->traceEverOn(true);
        dut->trace(m_trace, 5);
        std::string module_name = typeid(MODULE).name();
        std::cout << module_name << std::endl;
        module_name += ".vcd";
        std::cout << "traceEverOn" << std::endl;
        m_trace->open(module_name.c_str());
        std::cout << "Open File" << std::endl;
    }

    ~TESTBENCH(void) {
        m_trace->close();
        delete m_trace;
        delete dut;
        delete contextp;
    }

    void inline sim_init() {
        dut->eval();
        m_trace->dump(contextp->time());
    }

    void inline set_clk(CData i) {
        if (module_type == sequential) {
            dut->clk = i;
        }
    }

    void inline sim_cycles(int i) {
        for (int j=0; j<i; j++) {
            set_clk(0);
            dut->eval();
            contextp->timeInc(1);
            m_trace->dump(contextp->time());
            set_clk(1);
            dut->eval();
            contextp->timeInc(1);
            m_trace->dump(contextp->time());
        }
    }

    uint64_t rand64() {
        std::random_device rd;
        uint32_t seed = rd();
        std::mt19937_64 rng(seed);
        return rng();
    }
};

#endif