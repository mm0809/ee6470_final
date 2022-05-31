#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "directive.h"
#include "SobelFilter.h"

SobelFilter::SobelFilter( sc_module_name n ): sc_module( n )
{
#ifndef NATIVE_SYSTEMC
	HLS_MAP_TO_REG_BANK( A );
	HLS_MAP_TO_REG_BANK( C );
#endif
	SC_THREAD( do_filter );
	sensitive << i_clk.pos();
	dont_initialize();
	reset_signal_is(i_rst, false);

#ifndef NATIVE_SYSTEMC
	i_rgb.clk_rst(i_clk, i_rst);
	o_result.clk_rst(i_clk, i_rst);
#endif
}

SobelFilter::~SobelFilter() {}

//void SobelFilter::merge(int first, int second, int width)
//{
//}

void SobelFilter::do_filter() {
	{
#ifndef NATIVE_SYSTEMC
		HLS_DEFINE_PROTOCOL("main_reset");
		i_rgb.reset();
		o_result.reset();
#endif
		wait();
	}
	while (true) {
		// input
		sc_dt::sc_biguint<128> array;
		int MSB, LSB;
#ifndef NATIVE_SYSTEMC
		{
			HLS_DEFINE_PROTOCOL("input");
			array = i_rgb.get();
			wait();
		}	
#else
		array = i_rgb.read();
#endif
		MSB = 7;
		LSB = 0;
		{
			CONSTRAINA;
			for (int i = 0; i < 16; i++) {
				UNROLLA;
				A[i] = array.range(MSB, LSB);		
				MSB += 8;
				LSB += 8;
			}
		}

		// caculate
		for (int w = 1; w < 16; w = w * 2) {
#ifdef UNROLL1
			HLS_UNROLL_LOOP( ON, "UNROLL w" );
#endif
			// make pair
			for (int first = 0; first < 16; first += 2 * w) {
#ifdef UNROLL1
				HLS_UNROLL_LOOP( ON, "UNROLL all" );
#endif
				int second = first + w;
				int end = second + w - 1;
				int l1 = first, l2 = second;
				int index = first;
				// merge
				for (int i = 0; i < 2 * w; i++) {
					//ifdef UNROLL1
					//   		HLS_UNROLL_LOOP( ON, "UNROLL merge" );
					//endif

#ifdef PIPELINE1
					HLS_PIPELINE_LOOP(HARD_STALL, 1, "pipe loop");
#endif
					if (l2 <= end && (A[l1] > A[l2] || l1 >= second)) {
						C[index++] = A[l2++];
					} else {
						C[index++] = A[l1++];
					}
				}
			}

			{
				CONSTRAINCOPY;
				for (int i = 0; i < 16; i++) {
					UNROLLCOPY;
					A[i] = C[i];
				}
			}
		}

		// output
		sc_dt::sc_biguint<128> result;
		MSB = 7;
		LSB = 0;
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
