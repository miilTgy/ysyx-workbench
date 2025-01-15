/*
 * @Author: Zeng GuangYi tgy_scut2021@outlook.com
 * @Date: 2025-01-15 20:31:21
 * @LastEditors: Zeng GuangYi tgy_scut2021@outlook.com
 * @LastEditTime: 2025-01-16 01:45:54
 * @FilePath: /npc/csrc/tb_common.h
 * @Description: Common Verilator testbench headder
 * 
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved. 
 */

#ifndef __TB_COM__ // dependency defender
#define __TB_COM__

#define NDEBUG

#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <typeinfo>
#include <string>
#include <random>
#include <assert.h>

#define __TB__ tb
#define __DUT__ dut

#ifndef __TB__
#error ERROR: NO __TB__ DEFINITION.
#endif

#define TB(a) __TB__->a
#define DUT(a) __DUT__->a

#define step(statements) do { \
            TB(toggle_clk()); \
            TB(__DUT__->eval()); \
            TB(contextp->timeInc(1)); \
            TB(m_trace->dump(TB(contextp->time()))); \
                TB(toggle_clk()); \
                TB(__DUT__->eval();) \
                {statements} \
                TB(__DUT__->eval()); \
                TB(contextp->timeInc(1)); \
                TB(m_trace->dump(TB(contextp->time()))); \
        } while (0)


enum Module_type {
    sequential, combinational
};

template<class MODULE> class TESTBENCH {
private:
    Module_type module_type;
    
public:
    MODULE *__DUT__;
    VerilatedContext *contextp;
    VerilatedVcdC *m_trace;

    /**
     * @description: Constructor:
     *                  1. specify module type (seq or com).
     *                  2. instantiate __DUT__ contextp m_trace
     *                     with default value.
     *                  3. set trace and trace file name.
     * @param {int}         argc to Get initial value.
     * @param {char}       *argv to Get initial value.
     * @param {Module_type} module_type sepcify simulation type.
     *                      (sequential or combinational?)
     * @return {*}
     */
    TESTBENCH(int argc, char *argv[], Module_type module_type) {
        std::cout << "start constructiog" << std::endl;
        this->module_type = module_type;
        Verilated::traceEverOn(true);
        Verilated::commandArgs(argc, argv);
        __DUT__ = new MODULE;
        contextp = new VerilatedContext;
        m_trace = new VerilatedVcdC;

        contextp->traceEverOn(true);
        __DUT__->trace(m_trace, 5);
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
        std::cout << "sim finished." << std::endl;
        m_trace->close();
        delete m_trace;
        delete __DUT__;
        delete contextp;
    }

    /**
     * @description: Init sim, must called before any sim.
     * @return {*}
     */
    void inline sim_init() {
        __DUT__->eval();
        if (module_type == sequential) {
            __DUT__->clk = 1;
        }
        m_trace->dump(contextp->time());
    }

    /**
     * @description: Verify clk and set clk to value i.
     * @param {CData} i Value passed to clk.
     * @return {*}
     */
    void inline set_clk(CData i) {
        if (module_type == sequential) {
            __DUT__->clk = i;
        }
    }

    /**
     * @description: Verify clk and toggle clk.
     * @return {*}
     */
    void inline toggle_clk() {
        if (module_type == sequential) {
            set_clk(!__DUT__->clk);
        }
    }

    /**
     * @description: sim 1 cycle (clk=0 and clk=1)
     * @param {int} i # of cycles to sim.
     * @return {*}
     */
    void inline cycles(int i) {
        for (int j=0; j<i; j++) {
            toggle_clk();
            __DUT__->eval();
            contextp->timeInc(1);
            m_trace->dump(contextp->time());
            if (module_type == sequential) {
                toggle_clk();
                __DUT__->eval();
                contextp->timeInc(1);
                m_trace->dump(contextp->time());
            }
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

    void inline check_eq(uint64_t a, uint64_t b) {
        if (a != b) {
            m_trace->close();
            delete m_trace;
            delete __DUT__;
            delete contextp;
            std::cout << "Assertion: a=" << a <<  "b=" << b << std::endl;
            assert(0);
        }
    }
};

#endif