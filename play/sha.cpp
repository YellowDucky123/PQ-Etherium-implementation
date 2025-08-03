#include <openssl/sha.h>
#include <openssl/evp.h>
#include <vector>
#include <array>
#include <iostream>
int main() {
    std::vector<uint8_t> output(10);  
    unsigned char* hash;
    std::vector<uint8_t> message = {0x61, 0x62, 0x63}; // Example message "abc"
    std::vector<uint8_t> message2 = {0x75, 0x74, 0x73};
    EVP_MD_CTX *mdctx;
    if((mdctx = EVP_MD_CTX_new()) == NULL) {
        std::cerr << "Failed to create EVP_MD_CTX" << std::endl;
        return 1;
    }

    if(1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)) {
        std::cerr << "Failed to initialize digest" << std::endl;
        return 1;
    }

    if(1 != EVP_DigestUpdate(mdctx, message.data(), message.size())) {
        std::cerr << "Failed to update digest" << std::endl;
        return 1;
    }
    
    if(1 != EVP_DigestUpdate(mdctx, message2.data(), message2.size())) {
        std::cerr << "Failed to update digest with second message" << std::endl;
        return 1;
    }

    if((hash = (unsigned char *)OPENSSL_malloc(EVP_MD_size(EVP_sha256()))) == NULL) {
        std::cerr << "Failed to allocate memory for hash" << std::endl;
        return 1;
    }

    unsigned int digest_len;
    if(1 != EVP_DigestFinal_ex(mdctx, hash, &digest_len)) {
        std::cerr << "Failed to finalize digest" << std::endl;
        return 1;
    }

    for(int i = 0; i < output.size(); i++) {
        output[i] = hash[i];
    }

    EVP_MD_CTX_free(mdctx);
    for(uint8_t b: output) {
        std::cout << std::hex << static_cast<int>(b) << " ";
    }
}