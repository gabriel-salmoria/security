#include "Hash.hpp"
#include <openssl/evp.h>
#include <stdexcept>
#include <iomanip>
#include <sstream>

namespace ibemail {
namespace hash {

void H1(element_t out_q_id, const std::string& id) {
    // A PBC internal function directly hashes a buffer into a group element.
    element_from_hash(out_q_id, (void*)id.data(), id.size());
}

std::vector<unsigned char> H2(element_t gt_element, size_t n_bytes) {
    int len = element_length_in_bytes(gt_element);
    std::vector<unsigned char> buf(len);
    element_to_bytes(buf.data(), gt_element);

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    unsigned char domain = 0x02; // Domain separation
    EVP_DigestUpdate(ctx, &domain, 1);
    EVP_DigestUpdate(ctx, buf.data(), buf.size());
    
    std::vector<unsigned char> hash(EVP_MD_size(EVP_sha256()));
    unsigned int out_len;
    EVP_DigestFinal_ex(ctx, hash.data(), &out_len);
    EVP_MD_CTX_free(ctx);

    // Truncate or expand to n_bytes. For our purposes, n_bytes will be 32 (SHA-256 size).
    // If it's less, we truncate.
    hash.resize(n_bytes); 
    return hash;
}

void H3(element_t out_r, const std::vector<unsigned char>& sigma, const std::vector<unsigned char>& M) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    unsigned char domain = 0x03; // Domain separation
    EVP_DigestUpdate(ctx, &domain, 1);
    EVP_DigestUpdate(ctx, sigma.data(), sigma.size());
    EVP_DigestUpdate(ctx, M.data(), M.size());
    
    std::vector<unsigned char> hash(EVP_MD_size(EVP_sha256()));
    unsigned int out_len;
    EVP_DigestFinal_ex(ctx, hash.data(), &out_len);
    EVP_MD_CTX_free(ctx);

    // out_r is in Zq*. Map the hash output to an element of Zq*.
    element_from_hash(out_r, hash.data(), hash.size());
}

std::vector<unsigned char> H4(const std::vector<unsigned char>& sigma, size_t n_bytes) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    unsigned char domain = 0x04; // Domain separation
    EVP_DigestUpdate(ctx, &domain, 1);
    EVP_DigestUpdate(ctx, sigma.data(), sigma.size());
    
    std::vector<unsigned char> hash(EVP_MD_size(EVP_sha256()));
    unsigned int out_len;
    EVP_DigestFinal_ex(ctx, hash.data(), &out_len);
    EVP_MD_CTX_free(ctx);

    hash.resize(n_bytes); 
    return hash;
}

std::string sha256_string(const std::string& input) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, input.data(), input.size());
    
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int out_len;
    EVP_DigestFinal_ex(ctx, hash, &out_len);
    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < out_len; i++) {
        ss << std::setw(2) << (int)hash[i];
    }
    return ss.str();
}

}
}
