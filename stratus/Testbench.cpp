#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;

#include "Testbench.h"

Testbench::Testbench(sc_module_name n) : sc_module(n) {
	SC_THREAD(feed_array);
	sensitive << i_clk.pos();
	dont_initialize();
	SC_THREAD(fetch_result);
	sensitive << i_clk.pos();
	dont_initialize();
}

Testbench::~Testbench() {
	//cout<< "Max txn time = " << max_txn_time << endl;
	//cout<< "Min txn time = " << min_txn_time << endl;
	//cout<< "Avg txn time = " << total_txn_time/n_txn << endl;
	cout << "time between in and out = " << total_run_time << endl;
}

int Testbench::read_array(string infile_name) {
	FILE *fp_s = NULL; // source file handler
	fp_s = fopen(infile_name.c_str(), "r");
	if (fp_s == NULL) {
		printf("fopen %s error\n", infile_name.c_str());
		return -1;
	}

	// read array A
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 16; j++) {
			fscanf(fp_s, "%d", &A[i][j]);
		}
	}
	//cout << "array A" << endl;
	//for (int i = 0; i < 4; i++) {
	//	for (int j = 0; j < 4; j++) {
	//		cout << A[i][j] << " ";
	//	}
	//}
	cout << endl;

	
	fclose(fp_s);
	return 0;
}

void Testbench::feed_array() {
	int test_cycle = 0;		  // test the time between the first input and first output.
	n_txn = 0;
	max_txn_time = SC_ZERO_TIME;
	min_txn_time = SC_ZERO_TIME;
	total_txn_time = SC_ZERO_TIME;

#ifndef NATIVE_SYSTEMC
	o_array.reset();
#endif

	o_rst.write(false);
	wait(5);
	o_rst.write(true);
	wait(1);


	int cnt = 10;
	int i = 0;
	while (cnt--) { 
		if (test_cycle == 0) {
			total_start_time = sc_time_stamp();
			test_cycle = 1;
		}

		sc_dt::sc_biguint<128> arrays;
		int MSB, LSB;
		MSB = 7;
		LSB = 0;
		for (int j = 0; j < 16; j++) {
			sc_dt::sc_uint<8> tmp;
			if (i < 5) arrays.range(MSB, LSB) = (unsigned char)A[i][j];
			else arrays.range(MSB, LSB) = (unsigned char)0;

			MSB += 8;
			LSB += 8;
		}

		cout << "At  ";
		cout << sc_time_stamp();

		cout << "\tTestbench: input = ";
		for (int j = 0; j < 16; j++) {
			if (i < 5)
				cout << A[i][j] << " ";
			else
				cout << 0 << " ";
		}
		cout << endl;
#ifndef NATIVE_SYSTEMC
		o_array.put(arrays);
#else
		o_array.write(arrays);
#endif
	i++;
	}

}

void Testbench::fetch_result() {
	unsigned int x, y; // for loop counter
	int test_cycle = 0;
	sc_dt::sc_biguint<128> result_array;

#ifndef NATIVE_SYSTEMC
	i_result.reset();
#endif

	wait(5);
	wait(1);

	int cnt = 10;
	int j = 0;

	while (cnt--) {
#ifndef NATIVE_SYSTEMC
		result_array = i_result.get();
#else
		result_array = i_result.read();
#endif
		if (test_cycle == 0) {
			total_run_time = sc_time_stamp() - total_start_time;
			test_cycle = 1;
		}
		cout << "At  ";
		cout << sc_time_stamp();


		int MSB = 7, LSB = 0;
		sc_dt::sc_uint<8> tmp;
		cout << "\tTestbench: output = ";
		for (int i = 0; i < 16; i++) {
			tmp = result_array.range(MSB, LSB);
			cout << tmp << " ";
			MSB += 8;
			LSB += 8;
		}
		cout << endl;
		cout << endl;

	}
	sc_stop();
}
