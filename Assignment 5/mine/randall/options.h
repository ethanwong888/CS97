#include <stdbool.h>

enum Input { RDRAND, MRAND48_R, SLASH_F };
enum Output { STDOUT, N };

struct opts {
  bool valid;
  long long nbytes;
  enum Input input;
  char* r_src;
  enum Output output;
  unsigned int block_size;

  // int input_flag;
  // int output_flag;
};

void read_options(
  int argc,
  char* argv[],
  struct opts* opts
);
