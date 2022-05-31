
#ifdef CONSTRAINLOOP
	#define CONSTRAINA HLS_CONSTRAIN_LATENCY(0, 1, "readA_CONSTRAIN");
	#define UNROLLA HLS_UNROLL_LOOP( ON, "readA_UNROLL" );
	#define CONSTRAINCOPY HLS_CONSTRAIN_LATENCY(0, 1, "copyCtoA_CONSTRAIN");
	#define UNROLLCOPY HLS_UNROLL_LOOP( ON, "copyCtoA_UNROLL" );
	#define CONSTRAINOUT HLS_CONSTRAIN_LATENCY(0, 1, "outputC_CONSTRAIN");
	#define UNROLLOUT HLS_UNROLL_LOOP( ON, "outputC_UNROLL" );
#else
	#define CONSTRAINA
	#define UNROLLA
	#define CONSTRAINCOPY
	#define UNROLLCOPY
	#define CONSTRAINOUT
	#define UNROLLOUT
#endif

#ifdef UNROLLPIPE
	#define UNROLLFORLOOP_A  HLS_UNROLL_LOOP(ON, "w1unroll");
    #define PIPEFORLOOP_A    HLS_PIPELINE_LOOP(HARD_STALL, 1, "w1");
	#define UNROLLFORLOOP_B  HLS_UNROLL_LOOP(ON, "w2unroll");
    #define PIPEFORLOOP_B    HLS_PIPELINE_LOOP(HARD_STALL, 1, "w2");
    #define PIPEFORLOOP_C1   HLS_PIPELINE_LOOP(HARD_STALL, 1, "w41");
    #define PIPEFORLOOP_C2   HLS_PIPELINE_LOOP(HARD_STALL, 1, "w42");
	#define UNROLLFORLOOP_D  HLS_UNROLL_LOOP(ON, "w8unroll");
    #define PIPEFORLOOP_D    HLS_PIPELINE_LOOP(HARD_STALL, 1, "w8");
#else
	#define UNROLLFORLOOP_A  
    #define PIPEFORLOOP_A    
	#define UNROLLFORLOOP_B  
    #define PIPEFORLOOP_B    
    #define PIPEFORLOOP_C1   
    #define PIPEFORLOOP_C2   
	#define UNROLLFORLOOP_D  
    #define PIPEFORLOOP_D    
#endif
