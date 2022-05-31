#ifndef SYSTEM_H_
#define SYSTEM_H_
#include <systemc>
using namespace sc_core;

#include "Testbench.h"
#ifndef NATIVE_SYSTEMC
#include "MergeW1_wrap.h"
#include "MergeW2_wrap.h"
#include "MergeW4_wrap.h"
#include "MergeW8_wrap.h"
#else
#include "MergeW1.h"
#include "MergeW2.h"
#include "MergeW4.h"
#include "MergeW8.h"
#endif

class System: public sc_module
{
public:
	SC_HAS_PROCESS( System );
	System( sc_module_name n, std::string input_bmp, std::string output_bmp );
	~System();
private:
  Testbench tb;
#ifndef NATIVE_SYSTEMC
	MergeW1_wrapper merge_w1;
	MergeW2_wrapper merge_w2;
	MergeW4_wrapper merge_w4;
	MergeW8_wrapper merge_w8;
#else
	MergeW1 merge_w1;
	MergeW2 merge_w2;
	MergeW4 merge_w4;
	MergeW8 merge_w8;
#endif
	sc_clock clk;
	sc_signal<bool> rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_dt::sc_biguint<128> > rgb;
	cynw_p2p< sc_dt::sc_biguint<128> > w1_w2;
	cynw_p2p< sc_dt::sc_biguint<128> > w2_w4;
	cynw_p2p< sc_dt::sc_biguint<128> > w4_w8;
	cynw_p2p< sc_dt::sc_biguint<128> > result;
#else
	sc_fifo< sc_dt::sc_biguint<128> > rgb;
	sc_fifo< sc_dt::sc_biguint<128> > w1_w2;
	sc_fifo< sc_dt::sc_biguint<128> > w2_w4;
	sc_fifo< sc_dt::sc_biguint<128> > w4_w8;
	sc_fifo< sc_dt::sc_biguint<128> > result;
#endif

	std::string _output_bmp;
};
#endif
