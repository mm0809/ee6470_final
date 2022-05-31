#include <cstdlib>
#include <ctime>

#include "basic_timer.h"
#include "core/common/clint.h"
#include "display.hpp"
#include "dma.h"
#include "elf_loader.h"
#include "ethernet.h"
#include "fe310_plic.h"
#include "flash.h"
#include "debug_memory.h"
#include "iss.h"
#include "mem.h"
#include "memory.h"
#include "mram.h"
#include "sensor.h"
#include "sensor2.h"
#include "syscall.h"
#include "terminal.h"
#include "util/options.h"
#include "platform/common/options.h"

#include "gdb-mc/gdb_server.h"
#include "gdb-mc/gdb_runner.h"

#include "Merge_W1.h"
#include "Merge_W2.h"
#include "Merge_W4.h"
#include "Merge_W8.h"

#include <boost/io/ios_state.hpp>
#include <boost/program_options.hpp>
#include <iomanip>
#include <iostream>

using namespace rv32;
namespace po = boost::program_options;

class BasicOptions : public Options {
public:
	typedef unsigned int addr_t;

	std::string mram_image;
	std::string flash_device;
	std::string network_device;
	std::string test_signature;

    bool quiet = false;

	addr_t mem_size = 1024 * 1024 * 32;  // 32 MB ram, to place it before the CLINT and run the base examples (assume
	                                     // memory start at zero) without modifications
	addr_t mem_start_addr = 0x00000000;
	addr_t mem_end_addr = mem_start_addr + mem_size - 1;
	addr_t clint_start_addr = 0x02000000;
	addr_t clint_end_addr = 0x0200ffff;
	addr_t sys_start_addr = 0x02010000;
	addr_t sys_end_addr = 0x020103ff;
	addr_t term_start_addr = 0x20000000;
	addr_t term_end_addr = term_start_addr + 16;
	addr_t ethernet_start_addr = 0x30000000;
	addr_t ethernet_end_addr = ethernet_start_addr + 1500;
	addr_t plic_start_addr = 0x40000000;
	addr_t plic_end_addr = 0x41000000;
	addr_t sensor_start_addr = 0x50000000;
	addr_t sensor_end_addr = 0x50001000;
	addr_t sensor2_start_addr = 0x50002000;
	addr_t sensor2_end_addr = 0x50004000;
	addr_t mram_start_addr = 0x60000000;
	addr_t mram_size = 0x10000000;
	addr_t mram_end_addr = mram_start_addr + mram_size - 1;
	addr_t dma_start_addr = 0x70000000;
	addr_t dma_end_addr = 0x70001000;
	addr_t flash_start_addr = 0x71000000;
	addr_t flash_end_addr = flash_start_addr + Flashcontroller::ADDR_SPACE;  // Usually 528 Byte
	addr_t display_start_addr = 0x72000000;
	addr_t display_end_addr = display_start_addr + Display::addressRange;

	addr_t mergew1_0_start_addr = 0x73000000;
	addr_t mergew1_0_size = 0x01000000;
	addr_t mergew1_0_end_addr = mergew1_0_start_addr + mergew1_0_size - 1;

	addr_t mergew8_0_start_addr = 0x74000000;
	addr_t mergew8_0_size = 0x01000000;
	addr_t mergew8_0_end_addr = mergew8_0_start_addr + mergew8_0_size - 1;

	addr_t mergew1_1_start_addr = 0x75000000;
	addr_t mergew1_1_size = 0x01000000;
	addr_t mergew1_1_end_addr = mergew1_1_start_addr + mergew1_1_size - 1;

	addr_t mergew8_1_start_addr = 0x76000000;
	addr_t mergew8_1_size = 0x01000000;
	addr_t mergew8_1_end_addr = mergew8_1_start_addr + mergew8_1_size - 1;

	addr_t mergew1_2_start_addr = 0x77000000;
	addr_t mergew1_2_size = 0x01000000;
	addr_t mergew1_2_end_addr = mergew1_2_start_addr + mergew1_2_size - 1;

	addr_t mergew8_2_start_addr = 0x78000000;
	addr_t mergew8_2_size = 0x01000000;
	addr_t mergew8_2_end_addr = mergew8_2_start_addr + mergew8_2_size - 1;

	addr_t mergew1_3_start_addr = 0x79000000;
	addr_t mergew1_3_size = 0x01000000;
	addr_t mergew1_3_end_addr = mergew1_3_start_addr + mergew1_3_size - 1;

	addr_t mergew8_3_start_addr = 0x80000000;
	addr_t mergew8_3_size = 0x01000000;
	addr_t mergew8_3_end_addr = mergew8_3_start_addr + mergew8_3_size - 1;

	bool use_E_base_isa = false;

	OptionValue<unsigned long> entry_point;

	BasicOptions(void) {
        	// clang-format off
		add_options()
			("memory-start", po::value<unsigned int>(&mem_start_addr),"set memory start address")
			("memory-size", po::value<unsigned int>(&mem_size), "set memory size")
			("use-E-base-isa", po::bool_switch(&use_E_base_isa), "use the E instead of the I integer base ISA")
			("entry-point", po::value<std::string>(&entry_point.option),"set entry point address (ISS program counter)")
			("mram-image", po::value<std::string>(&mram_image)->default_value(""),"MRAM image file for persistency")
			("mram-image-size", po::value<unsigned int>(&mram_size), "MRAM image size")
			("flash-device", po::value<std::string>(&flash_device)->default_value(""),"blockdevice for flash emulation")
			("network-device", po::value<std::string>(&network_device)->default_value(""),"name of the tap network adapter, e.g. /dev/tap6")
			("signature", po::value<std::string>(&test_signature)->default_value(""),"output filename for the test execution signature");
        	// clang-format on
	}

	void parse(int argc, char **argv) override {
		Options::parse(argc, argv);

		entry_point.finalize(parse_ulong_option);
		mem_end_addr = mem_start_addr + mem_size - 1;
		assert((mem_end_addr < clint_start_addr || mem_start_addr > display_end_addr) &&
		       "RAM too big, would overlap memory");
		mram_end_addr = mram_start_addr + mram_size - 1;
		assert(mram_end_addr < dma_start_addr && "MRAM too big, would overlap memory");
	}
};

std::ostream& operator<<(std::ostream& os, const BasicOptions& o) {
	os << std::hex;
	os << "mem_start_addr:\t" << +o.mem_start_addr << std::endl;
	os << "mem_end_addr:\t"   << +o.mem_end_addr   << std::endl;
	os << static_cast <const Options&>( o );
	return os;
}

int sc_main(int argc, char **argv) {
	BasicOptions opt;
	opt.parse(argc, argv);

	std::srand(std::time(nullptr));  // use current time as seed for random generator

	tlm::tlm_global_quantum::instance().set(sc_core::sc_time(opt.tlm_global_quantum, sc_core::SC_NS));

	ISS core0(0, opt.use_E_base_isa);
	ISS core1(1, opt.use_E_base_isa);
	SimpleMemory mem("SimpleMemory", opt.mem_size);
	SimpleTerminal term("SimpleTerminal");
	ELFLoader loader(opt.input_program.c_str());
	SimpleBus<4, 20> bus("SimpleBus");
	CombinedMemoryInterface iss_mem_if0("MemoryInterface0", core0);
	CombinedMemoryInterface iss_mem_if1("MemoryInterface1", core1);
	SyscallHandler sys("SyscallHandler");
	FE310_PLIC<1, 64, 96, 32> plic("PLIC");
	CLINT<2> clint("CLINT");
	SimpleSensor sensor("SimpleSensor", 2);
	SimpleSensor2 sensor2("SimpleSensor2", 5);
	BasicTimer timer("BasicTimer", 3);
	SimpleMRAM mram("SimpleMRAM", opt.mram_image, opt.mram_size);
	SimpleDMA dma("SimpleDMA", 4);
	Flashcontroller flashController("Flashcontroller", opt.flash_device);
	EthernetDevice ethernet("EthernetDevice", 7, mem.data, opt.network_device);
	Display display("Display");
	DebugMemoryInterface dbg_if("DebugMemoryInterface");
	MergeW1 mergeW1_0("MergeW1_0");
	MergeW2 mergeW2_0("MergeW2_0");
	MergeW4 mergeW4_0("MergeW4_0");
	MergeW8 mergeW8_0("MergeW8_0");
	MergeW1 mergeW1_1("MergeW1_1");
	MergeW2 mergeW2_1("MergeW2_1");
	MergeW4 mergeW4_1("MergeW4_1");
	MergeW8 mergeW8_1("MergeW8_1");
	MergeW1 mergeW1_2("MergeW1_2");
	MergeW2 mergeW2_2("MergeW2_2");
	MergeW4 mergeW4_2("MergeW4_2");
	MergeW8 mergeW8_2("MergeW8_2");
	MergeW1 mergeW1_3("MergeW1_3");
	MergeW2 mergeW2_3("MergeW2_3");
	MergeW4 mergeW4_3("MergeW4_3");
	MergeW8 mergeW8_3("MergeW8_3");

    // fifo
	sc_fifo<unsigned char> w12_0;
	sc_fifo<unsigned char> w24_0;
	sc_fifo<unsigned char> w48_0;
	sc_fifo<unsigned char> w12_1;
	sc_fifo<unsigned char> w24_1;
	sc_fifo<unsigned char> w48_1;
	sc_fifo<unsigned char> w12_2;
	sc_fifo<unsigned char> w24_2;
	sc_fifo<unsigned char> w48_2;
	sc_fifo<unsigned char> w12_3;
	sc_fifo<unsigned char> w24_3;
	sc_fifo<unsigned char> w48_3;
    // connect with fifo
    mergeW1_0.o_array(w12_0);
    mergeW2_0.i_array(w12_0);
    mergeW2_0.o_array(w24_0);
    mergeW4_0.i_array(w24_0);
    mergeW4_0.o_array(w48_0);
    mergeW8_0.i_array(w48_0);
    mergeW1_1.o_array(w12_1);
    mergeW2_1.i_array(w12_1);
    mergeW2_1.o_array(w24_1);
    mergeW4_1.i_array(w24_1);
    mergeW4_1.o_array(w48_1);
    mergeW8_1.i_array(w48_1);
    mergeW1_2.o_array(w12_2);
    mergeW2_2.i_array(w12_2);
    mergeW2_2.o_array(w24_2);
    mergeW4_2.i_array(w24_2);
    mergeW4_2.o_array(w48_2);
    mergeW8_2.i_array(w48_2);
    mergeW1_3.o_array(w12_3);
    mergeW2_3.i_array(w12_3);
    mergeW2_3.o_array(w24_3);
    mergeW4_3.i_array(w24_3);
    mergeW4_3.o_array(w48_3);
    mergeW8_3.i_array(w48_3);

	MemoryDMI dmi = MemoryDMI::create_start_size_mapping(mem.data, opt.mem_start_addr, mem.size);
	InstrMemoryProxy instr_mem0(dmi, core0);
	InstrMemoryProxy instr_mem1(dmi, core1);

	std::shared_ptr<BusLock> bus_lock = std::make_shared<BusLock>();
	iss_mem_if0.bus_lock = bus_lock;
	iss_mem_if1.bus_lock = bus_lock;

	instr_memory_if *instr_mem_if0 = &iss_mem_if0;
	data_memory_if *data_mem_if0 = &iss_mem_if0;
	instr_memory_if *instr_mem_if1 = &iss_mem_if1;
	data_memory_if *data_mem_if1 = &iss_mem_if1;
    if (opt.use_instr_dmi) {
		instr_mem_if0 = &instr_mem0;
		instr_mem_if1 = &instr_mem1;
    }
	if (opt.use_data_dmi) {
		iss_mem_if0.dmi_ranges.emplace_back(dmi);
		iss_mem_if1.dmi_ranges.emplace_back(dmi);
	}

	uint64_t entry_point = loader.get_entrypoint();
	if (opt.entry_point.available)
		entry_point = opt.entry_point.value;
	try {
		loader.load_executable_image(mem, mem.size, opt.mem_start_addr);
	} catch(ELFLoader::load_executable_exception& e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "Memory map: " << std::endl;
		std::cerr << opt << std::endl;
		return -1;
	}
	core0.init(instr_mem_if0, data_mem_if0, &clint, entry_point, (opt.mem_end_addr-3));
	core1.init(instr_mem_if1, data_mem_if1, &clint, entry_point, (opt.mem_end_addr-32767));

	sys.init(mem.data, opt.mem_start_addr, loader.get_heap_addr());
	sys.register_core(&core0);
	sys.register_core(&core1);

    if (opt.intercept_syscalls) {
		core0.sys = &sys;
		core1.sys = &sys;

    }

	// address mapping
	bus.ports[0] = new PortMapping(opt.mem_start_addr, opt.mem_end_addr);
	bus.ports[1] = new PortMapping(opt.clint_start_addr, opt.clint_end_addr);
	bus.ports[2] = new PortMapping(opt.plic_start_addr, opt.plic_end_addr);
	bus.ports[3] = new PortMapping(opt.term_start_addr, opt.term_end_addr);
	bus.ports[4] = new PortMapping(opt.sensor_start_addr, opt.sensor_end_addr);
	bus.ports[5] = new PortMapping(opt.dma_start_addr, opt.dma_end_addr);
	bus.ports[6] = new PortMapping(opt.sensor2_start_addr, opt.sensor2_end_addr);
	bus.ports[7] = new PortMapping(opt.mram_start_addr, opt.mram_end_addr);
	bus.ports[8] = new PortMapping(opt.flash_start_addr, opt.flash_end_addr);
	bus.ports[9] = new PortMapping(opt.ethernet_start_addr, opt.ethernet_end_addr);
	bus.ports[10] = new PortMapping(opt.display_start_addr, opt.display_end_addr);
	bus.ports[11] = new PortMapping(opt.sys_start_addr, opt.sys_end_addr);
	bus.ports[12] = new PortMapping(opt.mergew1_0_start_addr, opt.mergew1_0_end_addr);
	bus.ports[13] = new PortMapping(opt.mergew8_0_start_addr, opt.mergew8_0_end_addr);
	bus.ports[14] = new PortMapping(opt.mergew1_1_start_addr, opt.mergew1_1_end_addr);
	bus.ports[15] = new PortMapping(opt.mergew8_1_start_addr, opt.mergew8_1_end_addr);
	bus.ports[16] = new PortMapping(opt.mergew1_2_start_addr, opt.mergew1_2_end_addr);
	bus.ports[17] = new PortMapping(opt.mergew8_2_start_addr, opt.mergew8_2_end_addr);
	bus.ports[18] = new PortMapping(opt.mergew1_3_start_addr, opt.mergew1_3_end_addr);
	bus.ports[19] = new PortMapping(opt.mergew8_3_start_addr, opt.mergew8_3_end_addr);

	// connect TLM sockets
	iss_mem_if0.isock.bind(bus.tsocks[0]);
	iss_mem_if1.isock.bind(bus.tsocks[1]);
	dbg_if.isock.bind(bus.tsocks[2]);

	PeripheralWriteConnector dma_connector("SimpleDMA-Connector");  // to respect ISS bus locking
	dma_connector.isock.bind(bus.tsocks[3]);
	dma.isock.bind(dma_connector.tsock);
	dma_connector.bus_lock = bus_lock;

	bus.isocks[0].bind(mem.tsock);
	bus.isocks[1].bind(clint.tsock);
	bus.isocks[2].bind(plic.tsock);
	bus.isocks[3].bind(term.tsock);
	bus.isocks[4].bind(sensor.tsock);
	bus.isocks[5].bind(dma.tsock);
	bus.isocks[6].bind(sensor2.tsock);
	bus.isocks[7].bind(mram.tsock);
	bus.isocks[8].bind(flashController.tsock);
	bus.isocks[9].bind(ethernet.tsock);
	bus.isocks[10].bind(display.tsock);
	bus.isocks[11].bind(sys.tsock);
	bus.isocks[12].bind(mergeW1_0.tsock);
	bus.isocks[13].bind(mergeW8_0.tsock);
	bus.isocks[14].bind(mergeW1_1.tsock);
	bus.isocks[15].bind(mergeW8_1.tsock);
	bus.isocks[16].bind(mergeW1_2.tsock);
	bus.isocks[17].bind(mergeW8_2.tsock);
	bus.isocks[18].bind(mergeW1_3.tsock);
	bus.isocks[19].bind(mergeW8_3.tsock);


	// connect interrupt signals/communication
	plic.target_harts[0] = &core0;
	clint.target_harts[0] = &core0;
	plic.target_harts[1] = &core1;
	clint.target_harts[1] = &core1;
	sensor.plic = &plic;
	dma.plic = &plic;
	timer.plic = &plic;
	sensor2.plic = &plic;
	ethernet.plic = &plic;

	std::vector<debug_target_if *> threads;
	threads.push_back(&core0);
	threads.push_back(&core1);

	core0.trace = opt.trace_mode;  // switch for printing instructions
	core1.trace = opt.trace_mode;  // switch for printing instructions
	if (opt.use_debug_runner) {
		auto server0 = new GDBServer("GDBServer0", threads, &dbg_if, opt.debug_port);
		auto server1 = new GDBServer("GDBServer1", threads, &dbg_if, opt.debug_port);
		new GDBServerRunner("GDBRunner0", server0, &core0);
		new GDBServerRunner("GDBRunner1", server1, &core1);
	} else {
		new DirectCoreRunner(core0);
		new DirectCoreRunner(core1);
	}

	sc_core::sc_start();

    if (!opt.quiet) {
        core0.show();
        core1.show();
    }

	if (opt.test_signature != "") {
		auto begin_sig = loader.get_begin_signature_address();
		auto end_sig = loader.get_end_signature_address();

		{
			boost::io::ios_flags_saver ifs(cout);
			std::cout << std::hex;
			std::cout << "begin_signature: " << begin_sig << std::endl;
			std::cout << "end_signature: " << end_sig << std::endl;
			std::cout << "signature output file: " << opt.test_signature << std::endl;
		}

		assert(end_sig >= begin_sig);
		assert(begin_sig >= opt.mem_start_addr);

		auto begin = begin_sig - opt.mem_start_addr;
		auto end = end_sig - opt.mem_start_addr;

		ofstream sigfile(opt.test_signature, ios::out);

		auto n = begin;
		while (n < end) {
			sigfile << std::hex << std::setw(2) << std::setfill('0') << (unsigned)mem.data[n];
			++n;
		}
	}

	return 0;
}
