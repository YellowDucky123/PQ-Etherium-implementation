#include "blake3.hpp"

int main() {
  Blake3 hash(10, 10, 10, 8);

  unsigned char* digest = (unsigned char*)"hello";
  std::vector<uint8_t> d(digest, digest + 5);
  std::vector<uint8_t> output = hash.apply({1,2,3,4,5,6,7,8,9,10}, 90, {1,2,3,4,5,6,7,8,9,10}, d);

  for (size_t i = 0; i < output.size(); i++) {
    printf("%02x", output[i]);
  }
}