#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "directive.h"
#include "MergeW8.h"

#include <iostream>

MergeW8::MergeW8( sc_module_name n ): sc_module( n )
{
#ifndef NATIVE_SYSTEMC
	HLS_MAP_TO_REG_BANK( A );
	HLS_MAP_TO_REG_BANK( C );
#endif
	SC_THREAD( do_merge );
	sensitive << i_clk.pos();
	dont_initialize();
	reset_signal_is(i_rst, false);

#ifndef NATIVE_SYSTEMC
	i_rgb.clk_rst(i_clk, i_rst);
	o_result.clk_rst(i_clk, i_rst);
#endif
}

MergeW8::~MergeW8() {}

//void MergeW8::merge(int first, int second, int width)
//{
//}

void MergeW8::do_merge() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_rgb.reset();
		o_result.reset();
		for (int i = 0; i < 16; i++) {
			HLS_UNROLL_LOOP(ON, "w8copy");
			A[i] = 0;
			C[i] = 0;
		}
#endif
		wait();
	}
	while (true) {
		// input
		sc_dt::sc_biguint<128> array;
		int iMSB, iLSB;
#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("input");
			array = i_rgb.get();
			wait();
		}	
#else
		array = i_rgb.read();
#endif
		iMSB = 7;
		iLSB = 0;
		{
			CONSTRAINA;
			for (int i = 0; i < 16; i++) {
				UNROLLA;
				A[i] = array.range(iMSB, iLSB);		
				iMSB += 8;
				iLSB += 8;
			}
		}



		{
		for (int  first = 0; first < 16; first += 8 * 2) {
			//HLS_UNROLL_LOOP(ON, "w8unroll");
			UNROLLFORLOOP_D
			int second = first + 8;
			int end = second + 8 - 1;
			int l1 = first, l2 = second;
			int index = first;
			HLS_CONSTRAIN_LATENCY (0, 300, "Merge8W_constrain");
		
			for (int i = 0; i < 8 * 2; i++) {
				//HLS_UNROLL_LOOP(ON, "w8");
				//HLS_PIPELINE_LOOP(HARD_STALL, 1, "w8");
				PIPEFORLOOP_D
				if (l2 <= end && (A[l1] > A[l2] || l1 >= second)) {
					C[index++] = A[l2++];	
				} else {
					C[index++] = A[l1++];
				}
			}
		}
		}


		// output
		sc_dt::sc_biguint<128> result;
		int MSB = 7;
		int LSB = 0;
		{
			CONSTRAINOUT;
			for (int i = 0; i < 16; i++) {
				UNROLLOUT;
				result.range(MSB, LSB) = C[i];
				MSB += 8;
				LSB += 8;
			}
		}
#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("output");
			o_result.put(result);
			wait();
		}
#else
		o_result.write(result);
#endif
	}
}
