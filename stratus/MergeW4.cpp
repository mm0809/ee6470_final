#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "directive.h"
#include "MergeW4.h"

#include <iostream>

MergeW4::MergeW4( sc_module_name n ): sc_module( n )
{
#ifndef NATIVE_SYSTEMC
	HLS_MAP_TO_REG_BANK( A1 );
	HLS_MAP_TO_REG_BANK( A2 );
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

MergeW4::~MergeW4() {}

//void MergeW4::merge(int first, int second, int width)
//{
//}

void MergeW4::do_merge() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_rgb.reset();
		o_result.reset();
		for (int i = 0; i < 16; i++) {
			HLS_UNROLL_LOOP(ON, "w4copy");
			//A[i] = 0;
			C[i] = 0;
		}
		for (int i = 0; i < 8; i++) {
			HLS_UNROLL_LOOP(ON, "w4copy2");
			A1[i] = 0;
			A2[i] = 0;
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
			for (int i = 0; i < 8; i++) {
				UNROLLA;
				A1[i] = array.range(iMSB, iLSB);		
				iMSB += 8;
				iLSB += 8;
			}
			for (int i = 0; i < 8; i++) {
				UNROLLA;
				A2[i] = array.range(iMSB, iLSB);		
				iMSB += 8;
				iLSB += 8;
			}
		}


		//{
		//for (int  first = 0; first < 16; first += 4 * 2) {
		//	HLS_UNROLL_LOOP(ON, "w4unroll");
		//	int second = first + 4;
		//	int end = second + 4 - 1;
		//	int l1 = first, l2 = second;
		//	int index = first;
        //    //HLS_CONSTRAIN_LATENCY (0, 10, "Merge4W_constrain");
		//
		//}
		//}
			{
            HLS_CONSTRAIN_LATENCY (0, 10, "Merge4W_constrain");
			int l1 = 0, l2 = 4;
			for (int i = 0; i < 4 * 2; i++) {
                //HLS_UNROLL_LOOP(ON, "w4");
                //HLS_PIPELINE_LOOP(HARD_STALL, 1, "w41");
				PIPEFORLOOP_C1
				if (l2 <= 7 && (A1[l1] > A1[l2] || l1 >= 4)) {
					C[i] = A1[l2++];
				} else {
					C[i] = A1[l1++];
				}
			}

			int l1b = 0, l2b = 4;
			for (int i = 8; i < 16; i++) {
                //HLS_UNROLL_LOOP(ON, "w4");
                //HLS_PIPELINE_LOOP(HARD_STALL, 1, "w42");
				PIPEFORLOOP_C2
				if (l2b <= 7 && (A2[l1b] > A2[l2b] || l1b >= 4)) {
					C[i] = A2[l2b++];
				} else {
					C[i] = A2[l1b++];
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
