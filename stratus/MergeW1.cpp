#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "directive.h"
#include "MergeW1.h"

#include <iostream>

MergeW1::MergeW1( sc_module_name n ): sc_module( n )
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

MergeW1::~MergeW1() {}

//void MergeW1::merge(int first, int second, int width)
//{
//}

void MergeW1::do_merge() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_rgb.reset();
		o_result.reset();
		for (int i = 0; i < 16; i++) {
			HLS_UNROLL_LOOP(ON, "w2copy");
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
		std::cout << "Merge1W: input " << sc_time_stamp() << std::endl;
		
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


		for (int  first = 0; first < 16; first += 1 * 2) {
			UNROLLFORLOOP_A
            //HLS_UNROLL_LOOP(ON, "w1unroll");
			int second = first + 1;
			int end = second + 1 - 1;
			int l1 = first, l2 = second;
			int index = first;
            //HLS_CONSTRAIN_LATENCY (0, 10, "Merge1W_constrain");
		
			for (int i = 0; i < 1 * 2; i++) {
				//HLS_UNROLL_LOOP(ON, "w4");
			 	//HLS_PIPELINE_LOOP(HARD_STALL, 1, "w1");
              	PIPEFORLOOP_A  
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
		std::cout << "Merge1W: output " << sc_time_stamp() << std::endl;
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
