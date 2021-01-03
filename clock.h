constexpr TICK_ERR = -1;
constexpr TICK_HLT = -2;

class MixIO;
class MixCPU;

class MixClock {
public:
  MixClock(MixCPU *cpu, MixIO *io) : cpu(cpu), io(io) {};
  int ts() { return _ts; }
  int tick() {
    _ts++;
    int ret;
    if ((ret = cpu->tick()) < 0)
      return ret;
    if ((ret = io->tick()) < 0)
      return ret;
    return _ts;
  }
  int next_ts() {
    int cn = cpu->next_ts();
    int in = io->next_ts();
    return (cn < in) ? cn : in;
  }
private:
  MixCPU *cpu;
  MixIO *io;
  int _ts = 0;
};