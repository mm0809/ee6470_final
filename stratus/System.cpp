#include "System.h"
#include <iostream>
System::System( sc_module_name n, string input_bmp, string output_bmp ): sc_module( n ), 
	tb("tb"), merge_w1("merge_w1"), merge_w2("merge_w2"), merge_w4("merge_w4"),merge_w8("merge_w8"), clk("clk", CLOCK_PERIOD, SC_NS), rst("rst"), _output_bmp(output_bmp)
{
	tb.i_clk(clk);
	tb.o_rst(rst);
	merge_w1.i_clk(clk);
	merge_w1.i_rst(rst);
	merge_w2.i_clk(clk);
	merge_w2.i_rst(rst);
	merge_w4.i_clk(clk);
	merge_w4.i_rst(rst);
	merge_w8.i_clk(clk);
	merge_w8.i_rst(rst);

	tb.o_array(rgb);
	tb.i_result(result);
	merge_w1.i_rgb(rgb);
	merge_w1.o_result(w1_w2);
	merge_w2.i_rgb(w1_w2);
	merge_w2.o_result(w2_w4);
	merge_w4.i_rgb(w2_w4);
	merge_w4.o_result(w4_w8);
	merge_w8.i_rgb(w4_w8);
	merge_w8.o_result(result);

	std::cout << input_bmp << std::endl;
  	tb.read_array(input_bmp);
}

System::~System() {
	;
}
