constexpr int TICK_ERR = -1;
constexpr int TICK_HLT = -2;
constexpr int TICK_BUS = -3;

class MixIO;
class MixCPU;

class MixClock {
public:
  MixClock(MixCPU *cpu, MixIO *io) : cpu(cpu), io(io) {};
  int ts() { return _ts; }
  int tick() {
    return tick_at(_ts + 1);
  }
  int tick_at(int new_ts) {
    _ts = new_ts;
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
