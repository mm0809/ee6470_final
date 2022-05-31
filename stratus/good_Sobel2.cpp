#include <cmath>
#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "directive.h"
#include "SobelFilter.h"

#include <iostream>

SobelFilter::SobelFilter( sc_module_name n ): sc_module( n )
{
#ifndef NATIVE_SYSTEMC
	HLS_MAP_TO_REG_BANK( A );
	HLS_MAP_TO_REG_BANK( C );
	HLS_MAP_TO_REG_BANK( w2 );
	HLS_MAP_TO_REG_BANK( w4 );
	HLS_MAP_TO_REG_BANK( w8 );
	HLS_MAP_TO_REG_BANK( reg );
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
		for (int i = 0; i < 16; i++) {
			HLS_UNROLL_LOOP(ON, "w2copy");
			w2[i] =  0 ;
			w4[i] =  0 ;
			w8[i] =  0 ;
			A[i] = 0;
			C[i] = 0;
		}
#endif
		wait();
	}
	while (true) {
		//std::cout << "C[] = ";
		//for (int i = 0; i < 16; i++) {
		//	std::cout << C[i] << " ";	
		//}
		//std::cout << std::endl;
		//std::cout << "w4[] = ";
		//for (int i = 0; i < 16; i++) {
		//	std::cout << w4[i] << " ";	
		//}
		//std::cout << std::endl;
		//std::cout << "w2[] = ";
		//for (int i = 0; i < 16; i++) {
		//	std::cout << w2[i] << " ";	
		//}
		//std::cout << std::endl;
		//std::cout << "A [] = ";
		//for (int i = 0; i < 16; i++) {
		//	std::cout << A[i] << " ";	
		//}
		//std::cout << std::endl;
		// input
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

		for (int  first = 0; first < 16; first += 16) {
			HLS_UNROLL_LOOP(ON, "w16unroll");
			int second = first + 8;
			int end = second + 8 - 1;
			int l1 = first, l2 = second;
			int index = first;
		
			for (int i = 0; i < 16; i++) {
				if (l2 <= end && (w8[l1] > w8[l2] || l1 >= second)) {
					C[index++] = w8[l2++];	
				} else {
					C[index++] = w8[l1++];
				}
			}
		}

		for (int  first = 0; first < 16; first += 8) {
			HLS_UNROLL_LOOP(ON, "w8unroll");
			int second = first + 4;
			int end = second + 4 - 1;
			int l1 = first, l2 = second;
			int index = first;
		
			for (int i = 0; i < 8; i++) {
				if (l2 <= end && (w4[l1] > w4[l2] || l1 >= second)) {
					w8[index++] = w4[l2++];	
				} else {
					w8[index++] = w4[l1++];
				}
			}
		}

		for (int  first = 0; first < 16; first += 4) {
			HLS_UNROLL_LOOP(ON, "w4unroll");
			int second = first + 2;
			int end = second + 2 - 1;
			int l1 = first, l2 = second;
			int index = first;
		
			for (int i = 0; i < 4; i++) {
				if (l2 <= end && (w2[l1] > w2[l2] || l1 >= second)) {
					w4[index++] = w2[l2++];	
				} else {
					w4[index++] = w2[l1++];
				}
			}
		}

		for (int  first = 0; first < 16; first += 2) {
			HLS_UNROLL_LOOP(ON, "w2unroll");
			int second = first + 1;
			int end = second;
			int l1 = first, l2 = second;
			int index = first;
		
			for (int i = 0; i < 2; i++) {
				if (l2 <= end && (A[l1] > A[l2] || l1 >= second)) {
					w2[index++] = A[l2++];	
				} else {
					w2[index++] = A[l1++];
				}
			}
		}



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
	}
}
