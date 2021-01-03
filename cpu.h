struct MixCore;
class MixClock;

class MixCPU {
public:
  MixCPU(MixCore *core);
  void init(MixClock *clock, MixIO *io);
  /*
   * Given a word, execute that word as though it's the current
   * instruction. Return the new value of the program counter.
   * ie, the address of the next instruction to execute.
   * Note that the word can be something other than the current
   * instruction (for debugging purposes).
   */
  int execute(Word w);
  /*
   * Perform the instruction (if any) corresponding to
   * the current clock tick.
   */
  int tick();
  /*
   * Lookup the next clock tick on which the CPU will execute
   * an instruction.
   */
  int next_ts();
private:
  MixCore *core;
  MixIO *io = nullptr;
  MixClock *clock = nullptr;
  // program counter (current instruction)
  int pc = 0;
  // ts of previous exected instruction
  // (used for timing purposes)
  int previous_ts = 0;
  // business logic to compute ts at which
  // instruction will complete after previous ts
  int get_ts(Word w);
};

