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
  int tick();
  int next_ts();
private:
  MixCore *core;
  MixIO *io = nullptr;
  MixClock *clock = nullptr;
  // program counter (current instruction)
  int pc = 0;

};

