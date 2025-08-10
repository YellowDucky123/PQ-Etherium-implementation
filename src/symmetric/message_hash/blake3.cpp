#include <iostream>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <cstdint>

extern "C" {
#include "BLAKE3/c/blake3.h"
}

int main(void) {
  // Initialize the hasher.
  blake3_hasher hasher;
  blake3_hasher_init(&hasher);

  // Read input bytes from stdin.
  unsigned char buf[65536];
  while (1) {
    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
    if (n > 0) {
      blake3_hasher_update(&hasher, buf, n);
    } else if (n == 0) {
      break; // end of file
    } else {
      fprintf(stderr, "read failed: %s\n", strerror(errno));
      return 1;
    }
  }

  // Finalize the hash. BLAKE3_OUT_LEN is the default output length, 32 bytes.
  // std::vector<uint8_t> output (BLAKE3_OUT_LEN);
  uint8_t output[BLAKE3_OUT_LEN];
  // std::vector<uint8_t> output(BLAKE3_OUT_LEN);
  blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);

  // Print the hash as hexadecimal.
  for (size_t i = 0; i < BLAKE3_OUT_LEN; i++) {
    printf("%02x", output[i]);
  }
  printf("\n");
  return 0;
}
