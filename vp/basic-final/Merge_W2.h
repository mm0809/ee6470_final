#ifndef MERGEW2_H
#define MERGEW2_H
#include <systemc>
#include <cmath>
#include <iomanip>
#include "merge_def.h"
using namespace sc_core;

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

//#include "filter_def.h"

struct MergeW2 : public sc_module {

    sc_fifo_in<unsigned char> i_array;
    sc_fifo_out<unsigned char> o_array;

    SC_HAS_PROCESS(MergeW2);

    MergeW2(sc_module_name n): 
        sc_module(n)
        //base_offset(0) 
    {
        SC_THREAD(do_merge);
    }

    ~MergeW2() {  }

    int A[16];
    int C[16];
    const int W = 2;


    void do_merge(){
        { wait(CLOCK_PERIOD, SC_NS); }
        while (true) {
            // read inpput
            for (int i = 0; i < 16; i++) {
                A[i] = i_array.read();
                //printf("%d ", A[i]);
            }
            //printf("\n ");

            // do Merge
            for (int first = 0; first < 16; first += W * 2) {
                int second = first + W;
                int end = second + W - 1;
                int l1 = first, l2 = second;
                int index = first;

                for (int i = 0; i < W * 2; i++) {
                    if (l2 <= end && (A[l1] > A[l2] || l1 >= second)) {
                        C[index++] = A[l2++];
                    } else {
                        C[index++] = A[l1++];
                    }
                }
            }

            // write output
            for (int i = 0; i < 16; i++) {
                //printf("%d ", C[i]);
                o_array.write(C[i]);
            }
            //printf("\n ");
        }
    }
};
#endif
