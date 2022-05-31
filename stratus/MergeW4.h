#ifndef SOBEL_FILTER_H_
#define SOBEL_FILTER_H_
#include <systemc>
using namespace sc_core;

#ifndef NATIVE_SYSTEMC
#include <cynw_p2p.h>
#endif

#include "filter_def.h"

class MergeW4: public sc_module
{
public:
	sc_in_clk i_clk;
	sc_in < bool >  i_rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_dt::sc_biguint<128> >::in i_rgb;
	cynw_p2p< sc_dt::sc_biguint<128> >::out o_result;
#else
	sc_fifo_in< sc_dt::sc_biguint<128> > i_rgb;
	sc_fifo_out< sc_dt::sc_biguint<128> > o_result;
#endif

	//void merge(int first, int second, int width);
	SC_HAS_PROCESS( MergeW4 );
	MergeW4( sc_module_name n );
	~MergeW4();
private:
	void do_merge();
	sc_dt::sc_uint<8> A1[8], A2[8], C[16];
};
#endif
