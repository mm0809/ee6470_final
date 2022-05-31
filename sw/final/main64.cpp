#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "stdint.h"
#include "irq.h"

static volatile char * const TERMINAL_ADDR = (char * const)0x20000000;

static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x70000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x70000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x70000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x7000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x70000010;

static const uint32_t DMA_OP_NOP = 0;
static const uint32_t DMA_OP_MEMCPY = 1;

int _is_using_dma = 1;

static char* const PE_START_ADDR_0 = reinterpret_cast<char* const>(0x73000000);
static char* const PE_READ_ADDR_0  = reinterpret_cast<char* const>(0x74000004);
static char* const PE_START_ADDR_1 = reinterpret_cast<char* const>(0x75000000);
static char* const PE_READ_ADDR_1  = reinterpret_cast<char* const>(0x76000004);
static char* const PE_START_ADDR_2 = reinterpret_cast<char* const>(0x77000000);
static char* const PE_READ_ADDR_2  = reinterpret_cast<char* const>(0x78000004);
static char* const PE_START_ADDR_3 = reinterpret_cast<char* const>(0x79000000);
static char* const PE_READ_ADDR_3  = reinterpret_cast<char* const>(0x80000004);

union word {
    int sint;
    unsigned int uint;
    unsigned char uc[4];
};

void write_data_to_ACC(char* ADDR, unsigned char* buffer, int len){
    if(_is_using_dma){  
        // Using DMA 
        *DMA_SRC_ADDR = (uint32_t)(buffer);
        *DMA_DST_ADDR = (uint32_t)(ADDR);
        *DMA_LEN_ADDR = len;
        *DMA_OP_ADDR  = DMA_OP_MEMCPY;
    }else{
        // Directly Send
        memcpy(ADDR, buffer, sizeof(unsigned char)*len);
    }
}
void read_data_from_ACC(char* ADDR, unsigned char* buffer, int len){
    if(_is_using_dma){
        // Using DMA 
        *DMA_SRC_ADDR = (uint32_t)(ADDR);
        *DMA_DST_ADDR = (uint32_t)(buffer);
        *DMA_LEN_ADDR = len;
        *DMA_OP_ADDR  = DMA_OP_MEMCPY;
    }else{
        // Directly Read
        memcpy(buffer, ADDR, sizeof(unsigned char)*len);
    }
}

void dma_irq_handler() {
    printf("dma_complete\n");
}

void timer_irq_handler() {
    //Do nothing
    //printf("1ms reached\n.");
}

int sem_init (uint32_t *__sem, uint32_t count) __THROW
{
    *__sem=count;
    return 0;
}

int sem_wait (uint32_t *__sem) __THROW
{
    uint32_t value, success; //RV32A
    __asm__ __volatile__("\
            L%=:\n\t\
            lr.w %[value],(%[__sem])            # load reserved\n\t\
            beqz %[value],L%=                   # if zero, try again\n\t\
            addi %[value],%[value],-1           # value --\n\t\
            sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
            bnez %[success], L%=                # if the store failed, try again\n\t\
            "
            : [value] "=r"(value), [success]"=r"(success)
            : [__sem] "r"(__sem)
            : "memory");
    return 0;
}

int sem_post (uint32_t *__sem) __THROW
{
    uint32_t value, success; //RV32A
    __asm__ __volatile__("\
            L%=:\n\t\
            lr.w %[value],(%[__sem])            # load reserved\n\t\
            addi %[value],%[value], 1           # value ++\n\t\
            sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
            bnez %[success], L%=                # if the store failed, try again\n\t\
            "
            : [value] "=r"(value), [success]"=r"(success)
            : [__sem] "r"(__sem)
            : "memory");
    return 0;
}

int barrier(uint32_t *__sem, uint32_t *__lock, uint32_t *counter, uint32_t thread_count) {
    sem_wait(__lock);
    if (*counter == thread_count - 1) { //all finished
        *counter = 0;
        sem_post(__lock);
        for (int j = 0; j < thread_count - 1; ++j) sem_post(__sem);
    } else {
        (*counter)++;
        sem_post(__lock);
        sem_wait(__sem);
    }
    return 0;
}


//Total number of cores
//static const int PROCESSORS = 2;
#define PROCESSORS 2
//the barrier synchronization objects
uint32_t barrier_counter=0; 
uint32_t barrier_lock; 
uint32_t barrier_sem; 
uint32_t DMA_flag[PROCESSORS]; 
//the mutex object to control global summation
uint32_t lock;  
//print synchronication semaphore (print in core order)
uint32_t print_flag[PROCESSORS]; 

//if (hart_id == 0) {
//    uint8_t src[32] = { [1]=1, 2, 3, 4, [28]=28, 29, 30, 31};
//    uint8_t dst[32] = { 0 };
//
//    for (int i = 0; i < 32; ++i) { 
//        printf("src[%d]=%d\n ", i, src[i]);
//    }
//
//    *DMA_SRC_ADDR = (uint32_t)(&src[0]);
//    *DMA_DST_ADDR = (uint32_t)(&dst[0]);
//    *DMA_LEN_ADDR = 32;
//    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
//
//
//    char buf[20];
//    for (int i = 0; i < 32; ++i) {
//        printf("dst[%d]=%d\n ", i, dst[i]);
//    }
//
//}

unsigned char in[64] = {41, 39, 14, 10, 49, 62, 37, 1, 17, 22, 9, 38, 28, 33, 8, 26, 46, 23, 64, 13, 54, 55, 27, 12, 56, 36,  3, 4, 15, 32, 19, 45, 21, 48, 34, 63, 52, 53, 16, 11, 44, 59, 43, 20, 57, 60, 30, 24, 29, 7, 47, 18, 6, 58, 25, 50, 31, 61, 51, 35, 40, 2, 5, 42};
//unsigned char in[128] = {41, 39, 14, 10, 49, 62, 37, 1, 17, 22, 9, 38, 28, 33, 8, 26, 46, 23, 64, 13, 54, 55, 27, 12, 56, 36,  3, 4, 15, 32, 19, 45, 21, 48, 34, 63, 52, 53, 16, 11, 44, 59, 43, 20, 57, 60, 30, 24, 29, 7, 47, 18, 6, 58, 25, 50, 31, 61, 51, 35, 40, 2, 5, 42, 41, 39, 14, 10, 49, 62, 37, 1, 17, 22, 9, 38, 28, 33, 8, 26, 46, 23, 64, 13, 54, 55, 27, 12, 56, 36,  3, 4, 15, 32, 19, 45, 21, 48, 34, 63, 52, 53, 16, 11, 44, 59, 43, 20, 57, 60, 30, 24, 29, 7, 47, 18, 6, 58, 25, 50, 31, 61, 51, 35, 40, 2, 5, 42};
//unsigned char in[64] = {0};
unsigned char tmp0[32] = {0};
unsigned char out0[32] = {0};

unsigned char tmp1[32] = {0};
unsigned char out1[32] = {0};

unsigned char result[64] = {0};
//unsigned char out[64] = {0};

void show(unsigned char *n, int k)
{
    for (int i = 0; i < k; i++) {
        printf("%3d", n[i]);
    }
    printf("\n");
}

int main(unsigned hart_id) {
    //register_interrupt_handler(4, dma_irq_handler);
    //register_interrupt_handler(3, timer_irq_handler);


    FILE *f = NULL;
    //if (hart_id == 0) {
    //    sem_init(&barrier_lock, 1);
    //    sem_init(&barrier_sem, 0); //lock all cores initially
    //    // Create mutex lock
    //    sem_init(&lock, 1);

    //    for(int i=0; i< PROCESSORS; ++i){
    //        sem_init(&DMA_flag[i], 0);
    //        sem_init(&print_flag[i], 0); //lock printing initially
    //    }
        //f = fopen("in.txt", "rb");


    //    printf("HI\n");
    //}

    //for (int i = 0; i < 10; i++) {
    if (hart_id == 0) {
        // create a barrier object with a count of PROCESSORS
        sem_init(&barrier_lock, 1);
        sem_init(&barrier_sem, 0); //lock all cores initially
        // Create mutex lock
        sem_init(&lock, 1);
        for(int i=0; i< PROCESSORS; ++i){
            sem_init(&DMA_flag[i], 0);
            sem_init(&print_flag[i], 0); //lock printing initially
        }
        //if (f != NULL) {
        //    for (int i = 0; i < 64; i++) {
        //        int k = 0;
        //        fscanf(f, "%d", &k); 
        //        in[i] = k;
        //    }
        //}
        //show(in, 64);


        //for (int i = 0; i < 16; i++) {
        //    printf("%d ", buffer[i]);
        //}
        //printf("\n");

        //sem_wait(&DMA_lock);
        write_data_to_ACC(PE_START_ADDR_0, in, 16);
        write_data_to_ACC(PE_START_ADDR_1, in + 16, 16);
        sem_post(&DMA_flag[1]);

        sem_wait(&DMA_flag[0]);
        read_data_from_ACC(PE_READ_ADDR_0, tmp0, 16);
        read_data_from_ACC(PE_READ_ADDR_1, tmp0 + 16, 16);
        sem_post(&DMA_flag[1]);

        //show(tmp0, 128);
        //sem_wait(&print_flag[0]);
        //show(out1, 32);
        //sem_post(&print_flag[1]);
        //

        for (int W = 32; W <= 32; W *= 2) {
            for (int i = 1; i <= 32 / W; i++) {
                int l1 = (i - 1) * W, l2 = (i - 1) * W + W / 2;
                for (int index = l1; index < i * W; index++) {
                    if (l1 < (i - 1) * W + W / 2 && (tmp0[l1] < tmp0[l2]) || l2 >= i * W) {
                        out0[index] = tmp0[l1++];
                    } else {
                        out0[index] = tmp0[l2++];
                    }
                }
            }
            for (int k = 0; k < 32; k++) {
                tmp0[k] = out0[k];
            }
        }

        //for (int l1 = 0, l2 = 16, index = 0; index < 32; index++) {
        //    if (l1 < 16 && (tmp0[l1] < tmp0[l2]) || l2 >= 32) {
        //        out0[index] = tmp0[l1++];
        //    } else {
        //        out0[index] = tmp0[l2++];
        //    }
        //}
        //
        //printf("core0 sort for 64 nums:\n");
        //show(out0, 64);
        //sem_post(&print_flag[1]);

        //sem_wait(&print_flag[0]);
        //printf("core0 sort for 32 nums:\n");
        //show(tmp1, 32);
        //sem_post(&print_flag[1]);
    }

    if (hart_id == 1) {

        sem_wait(&DMA_flag[1]);
        write_data_to_ACC(PE_START_ADDR_2, in + 32, 16);
        write_data_to_ACC(PE_START_ADDR_3, in + 48, 16);
        sem_post(&DMA_flag[0]);

        sem_wait(&DMA_flag[1]);
        read_data_from_ACC(PE_READ_ADDR_2, tmp1, 16);
        read_data_from_ACC(PE_READ_ADDR_3, tmp1 + 16, 16);
        sem_post(&DMA_flag[0]);

        //sem_wait(&print_flag[1]);
        //show(out2, 32);
        //sem_post(&print_flag[0]);
        //
        for (int W = 32; W <= 32; W *= 2) {
            for (int i = 1; i <= 32 / W; i++) {
                int l1 = (i - 1) * W, l2 = (i - 1) * W + W / 2;
                for (int index = l1; index < i * W; index++) {
                    if (l1 < (i - 1) * W + W / 2 && (tmp1[l1] < tmp1[l2]) || l2 >= i * W) {
                        out1[index] = tmp1[l1++];
                    } else {
                        out1[index] = tmp1[l2++];
                    }
                }
            }
            for (int k = 0; k < 32; k++) {
                tmp1[k] = out1[k];
            }
        }


        //int index = 0;
        //for (int l1 = 0, l2 = 16; index < 32; index++) {
        //    if (l1 < 16 && (out2[l1] < out2[l2]) || l2 >= 32) {
        //        tmp2[index] = out2[l1++];
        //    } else {
        //        tmp2[index] = out2[l2++];
        //    }
        //}

        //sem_wait(&print_flag[1]);
        //printf("core1 sort for 64 nums:\n");
        //show(out1, 64);
        //sem_post(&print_flag[0]);
    }

    barrier(&barrier_sem, &barrier_lock, &barrier_counter, PROCESSORS);

    if (hart_id == 0) {
        int W = 64;
        for (int i = 1; i <= 64 / W; i++) {
            int l1 = 0, l2 = 0;
            printf("l1: %d, l2: %d\n", l1, l2);
            for (int index = l1; index < i * W; index++) {
                if (l1 < 32 && (out0[l1] < out1[l2]) || l2 >= 32) {
                    result[index] = out0[l1++];
                } else {
                    result[index] = out1[l2++];
                }
            }
        }
        //show(result, 128);
    }

    //if (hart_id == 0) {
    //    int index = 0;
    //    for (int l1 = 0, l2 = 0; index < 64; index++) {
    //        if (l1 < 32 && (tmp1[l1] < tmp2[l2]) || l2 >= 32) {
    //            result[index] = tmp1[l1++];
    //        } else {
    //            result[index] = tmp2[l2++];
    //        }
    //    }

    //    printf("output:");
    //    show(result, 64);
    //    printf("\n\n");
    //}
    //}

    return 0;
}
