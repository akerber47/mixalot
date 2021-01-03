#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include "dbg.h"
#include "core.h"

std::map<std::string, std::pair<int, int>> OP_TABLE = {
  {"NOP", {0, 0}},
  {"ADD", {1, 5}},
  {"SUB", {2, 5}},
  {"MUL", {3, 5}},
  {"DIV", {4, 5}},
  {"NUM", {5, 0}},
  {"CHR", {5, 1}},
  {"HLT", {5, 2}},
  {"SLA", {6, 0}},
  {"SRA", {6, 1}},
  {"SLAX", {6, 2}},
  {"SRAX", {6, 3}},
  {"SLC", {6, 4}},
  {"SRC", {6, 5}},
  {"MOVE", {7, 1}},
  {"LDA", {8, 5}},
  {"LD1", {9, 5}},
  {"LD2", {10, 5}},
  {"LD3", {11, 5}},
  {"LD4", {12, 5}},
  {"LD5", {13, 5}},
  {"LD6", {14, 5}},
  {"LDX", {15, 5}},
  {"LDAN", {16, 5}},
  {"LD1N", {17, 5}},
  {"LD2N", {18, 5}},
  {"LD3N", {19, 5}},
  {"LD2", {10, 5}},
  {"LD3", {11, 5}},
  {"LD4", {12, 5}},
  {"LD5", {13, 5}},
  {"LD6", {14, 5}},
  {"LDX", {15, 5}},
  {"LDAN", {16, 5}},
  {"LD1N", {17, 5}},
  {"LD2N", {18, 5}},
  {"LD3N", {19, 5}},
  {"LD2", {10, 5}},
  {"LD3", {11, 5}},
  {"LD4", {12, 5}},
  {"LD5", {13, 5}},
  {"LD6", {14, 5}},
  {"LDX", {15, 5}},
  {"LDAN", {16, 5}},
  {"LD1N", {17, 5}},
  {"LD2N", {18, 5}},
  {"LD3N", {19, 5}},
  {"LD2", {10, 5}},
  {"LD3", {11, 5}},
  {"LD4", {12, 5}},
  {"LD5", {13, 5}},
  {"LD6", {14, 5}},
  {"LDX", {15, 5}},
  {"LDAN", {16, 5}},
  {"LD1N", {17, 5}},
  {"LD2N", {18, 5}},
  {"LD3N", {19, 5}},
  {"LD2", {10, 5}},
  {"LD3", {11, 5}},
  {"LD4", {12, 5}},
  {"LD5", {13, 5}},
  {"LD6", {14, 5}},
  {"LDX", {15, 5}},
  {"LDAN", {16, 5}},
  {"LD1N", {17, 5}},
  {"LD2N", {18, 5}},
  {"LD3N", {19, 5}},

int star = 0;
bool ended = false;
std::map<int, Word> words = {};
std::map<std::string, int> globals = {};
std::map<std::string, int> fglobals = {};
std::vector<int> locals = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
std::vector<int> flocals = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Assemble next row of input
int assemble_next(std::string s) {
  std::getline(in, s);
  for (char c : s) {
    if (!((c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') ||
          c == ' ' || c == '*' || c == '/' ||
          c == '+' || c == '-' || c == ':' ||
          c == '=' || 
          c == '(' || c == ')' || c == ',')) {
      D2("Found invalid character: ", c);
      return -1;
    }
  }
  // Omit blank lines and comments
  if (s.size() == 0 || s[0] == '*')
    return 0;
  int i = 0;
  while (i < s.size() && s[i] != ' ') i++;
  if (i == s.size()) {
    D2("Instruction must contain an opcode, given: " s);
    return -1;
  }
  std::string loc {s, 0, i};
  while (i < s.size() && s[i] == ' ') i++;
  int op_start = i;
  while (i < s.size() && s[i] != ' ') i++;
  if (i == s.size()) {
    D2("Instruction must contain an opcode, given: " s);
    return -1;
  }
  std::string op {s, op_start, i};
  std::string addr;
  // Special case -- ALF operator can ingest spaces
  if (op == "ALF") {
    i++;
    if (i < s.size() && s[i] == ' ') i++;
    if (i+5 < s.size()) {
      addr = {s, i, i+5};
    } else {
      D2("ALF instruction: address too short: ", s);
      return -1;
    }
  } else {
    while (i < s.size() && s[i] == ' ') i++;
    if (i == s.size()) {
      addr = "";
    } else {
      int addr_start = i;
      while (i < s.size() && s[i] != ' ') i++;
      addr = {s, addr_start, i};
    }
  }

  // At this point, we've tokenized the line
  // into loc (maybe empty), op, and addr (maybe empty).
  // Handle different cases for different operators.

  // Special operators
  if (op == "EQU" || op == "ORIG" ||
      op == "CON" || op == "END") {
    int w;
    if (parse_w(addr, w) < 0) {
      D("Error parsing W value");
      return -1;
    }
    // TODO handle special opcode with w
  } else if (op == "ALF") {
    // TODO handle ALF
  } else {
    int a;
    std::string future_a;
    int i;
    int f;
    if (parse_a(addr, a, future_a, i, f) < 0) {
      D("Error parsing A,I,F values");
      return -1;
    }
    // TODO handle general opcode
  }
}

int assemble_all(std::istream in) {
  for (std::string s; getline(in, s); ) {
    if ((int ret = assemble_next(s)) < 0)
      return ret;
  }
  if (!ended) {
    D("Never encountered END instruction");
    return -1;
  }
  return 0;
}

/*
 * Dump all assembled rows into an output file.
 */
void dump(std::string out_file);

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cout << "Usage: mixal <input.mixal> <output.mix>" << std::endl;
    return 2;
  }
  DBG_INIT();
  std::string in_file = argv[1];
  std::string out_file = argv[2];
  std::ifstream in {in_file};
  if (assemble_all(in) < 0) {
    D("Assembly failed!");
    return 1;
  }
  dump(out_file);
  DBG_CLOSE();
  return 0;
}
