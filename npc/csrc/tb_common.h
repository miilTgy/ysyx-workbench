/*
 * @Author: Zeng GuangYi tgy_scut2021@outlook.com
 * @Date: 2025-01-15 20:31:21
 * @LastEditors: Zeng GuangYi tgy_scut2021@outlook.com
 * @LastEditTime: 2025-01-15 23:52:11
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

    /**
     * @description: Constructor:
     *                  1. instantiate dut contextp m_trace
     *                  2. set trace and trace file name.
     * @param {int}   argc to Get initial value.
     * @param {char} *argv to Get initial value.
     * @return {*}
     */
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

    /**
     * @description: Destructor
     * @return {*}
     */
    ~TESTBENCH(void) {
        m_trace->close();
        delete m_trace;
        delete dut;
        delete contextp;
    }

    /**
     * @description: Init sim, must called before any sim.
     * @param {  } dut
     * @return {*}
     */
    void inline sim_init() {
        dut->eval();
        m_trace->dump(contextp->time());
    }

    /**
     * @description: Verify clk and set clk to value i.
     * @param {CData} i Value passed to clk.
     * @return {*}
     */
    void inline set_clk(CData i) {
        if (module_type == sequential) {
            dut->clk = i;
        }
    }

    /**
     * @description: sim 1 cycle (clk=0 and clk=1)
     * @param {int} i # of cycles to sim.
     * @return {*}
     */
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

    /**
     * @description: Generate 64-bit random num.
     * @return {*}
     */
    uint64_t rand64() {
        std::random_device rd;
        uint32_t seed = rd();
        std::mt19937_64 rng(seed);
        return rng();
    }
};

#endif