#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "sys.h"
#include "dbg.h"
#include "core.h"
#include "io.h"
#include "cpu.h"
#include "clock.h"


class Mix {
public:
  // In-memory core (owned by caller)
  Mix(MixCore *core);
  // Mapped core (owned by class)
  Mix(std::string core_file);
  ~Mix();
  /*
   * Load a core dump or program listing into the current
   * Mix machine. Skip all invalid lines.
   */
  void load(std::string filename);
  /*
   * Dump core fields of the Mix machine to a file.
   * See above.
   */
  void dump(std::string filename);
  /*
   * Convert core fields of the Mix machine to a string
   * If include_registers is set, include registers in the string.
   * If include_memory is set, include memory in the string.
   * If include_zeros is set, keep lines for memory
   * rows that are zero.
   */
  std::string to_str(
      bool include_registers = true,
      bool include_memory = false,
      bool include_zeros = false);
  // manually set some values for orchestration test
  void test();
  void step(int i);
  void run();
  void do_repl();
private:
  MixCore *core;
  MixCPU *cpu = nullptr;
  MixIO *io = nullptr;
  MixClock *clock = nullptr;
  int core_fd = -1;
};

Mix::Mix(MixCore *core) {
  this->core = core;
  cpu = new MixCPU(core);
  io = new MixIO(core);
  clock = new MixClock(cpu, io);
  cpu->init(clock, io);
  io->init(clock);
}

Mix::Mix(std::string core_file) {
  void *raw_core = nullptr;
  open_and_map(
    core_file,
    sizeof(MixCore),
    raw_core,
    this->core_fd
  );
  this->core = (MixCore *) raw_core;
  cpu = new MixCPU(core);
  io = new MixIO(core);
  clock = new MixClock(cpu, io);
  cpu->init(clock, io);
  io->init(clock);
}

Mix::~Mix() {
  if (clock != nullptr)
    delete clock;
  if (io != nullptr)
    delete io;
  if (cpu != nullptr)
    delete cpu;
  if (core_fd != -1) {
    unmap_and_close(core, sizeof(MixCore), core_fd);
  }
}

void Mix::load(std::string filename) {
  D2("loading ", filename);
  std::ifstream fs {filename};
  for (std::string s; fs >> s; ) {
    // Registers
    if (s[0] == 'A') {
      fs >> core->a;
    } else if (s[0] == 'X') {
      fs >> core->x;
    } else if (s[0] == 'I') {
      int i = stoi(s.substr(2, 1));
      if (i >= 1 && i <= 6) {
        fs >> core->i[i-1];
      } else {
        fs.setstate(std::ios_base::failbit);
      }
    } else if (s[0] == 'J') {
      fs >> core->j;
    // Memory
    } else {
      int i = stoi(s);
      if (i >= 0 && i < MEM_SIZE) {
        fs >> core->memory[i];
      } else {
        fs.setstate(std::ios_base::failbit);
      }
    }
    if (fs.fail()) {
      fs.clear();
      // discard remainder of line and continue
      fs.unget();
      getline(fs, s);
    }
  }
  fs.close();
}

std::string Mix::to_str(
    bool include_registers,
    bool include_memory,
    bool include_zeros) {
  std::stringstream ss;
  if (include_registers) {
    ss << "   A: " << core->a << std::endl;
    ss << "   X: " << core->x << std::endl;
    for (int i = 0; i < 6; i++) {
      ss << "I[" << (i+1) << "]: " << core->i[i] << std::endl;
    }
    ss << "   J: " << core->j << std::endl;
  }
  if (include_memory) {
    for (int i = 0; i < MEM_SIZE; i++) {
      if (core->memory[i] != 0 || include_zeros) {
        ss.width(4);
        ss.fill('0');
        ss << i << ": " << core->memory[i] << std::endl;
      }
    }
  }
  return ss.str();
}

void Mix::dump(std::string filename) {
  D2("dumping to ", filename);
  std::ofstream fs {filename};
  fs << to_str(true, true, true);
  fs.close();
}

void Mix::step(int i) {
  D2("Stepping through i operations, i = ", i);
  while (--i >= 0) {
    int next_ts = clock->next_ts();
    D2("Next operation occurs at clock time ts", next_ts);
    D2("Setting clock time to this ts and running tick", next_ts);
    int ret = clock->tick_at(next_ts);
    if (ret < 0) {
      D2("Failure/halt in clock tick, halting, code ", ret);
      return;
    }
  }
}

void Mix::run() {
  D("Running until halt or error...");
  while(true) {
    int next_ts = clock->next_ts();
    D2("Next operation occurs at clock time ts", next_ts);
    D2("Setting clock time to this ts and running tick", next_ts);
    int ret = clock->tick_at(next_ts);
    if (ret < 0) {
      D2("Failure/halt in clock tick, stopping, code ", ret);
      return;
    }
  }
}

void Mix::test() {
  core->a = 4;
  core->x = 5;
  core->i[0] = 3;
  core->i[1] = 9;
  core->i[2] = 27;
  core->i[3] = 81;
  core->overflow = Overflow::ON;
  core->memory[0] = 0xdeadbeef;
  core->memory[3999] = 0xdeadbeef;

}

void test_core() {
  D("test_core");
  Mix mix("./out/test.core");
  mix.test();
  // manually verify core file to check that it's good
  // $ xxd ./test/test.core | less
}

void test_dump() {
  D("test_dump");
  MixCore core;
  Mix m(&core);
  // set some values
  m.test();
  m.dump("./out/dump_out.mix");
  // load into new machine
  Mix m2("./out/dump.core");
  m2.load("./out/dump_out.mix");
  // manually verify core file to check that it's good
  // $ xxd test/dump.core | less
}

void test_lda() {
  D("test_lda");
  MixCore core;
  Mix m(&core);
  // set some values
  m.load("./test/lda.mix");
  m.step(11);
  m.dump("./out/lda_out.mix");
}

void test_max() {
  D("test_max");
  MixCore core;
  Mix m(&core);
  // set some values
  m.load("./test/max.mix");
  m.run();
  m.dump("./out/max_out.mix");
}

int main() {
  DBG_INIT();
  // test_core();
  // test_dump();
  // test_lda();
  test_max();
  DBG_CLOSE();
  return 0;
}
