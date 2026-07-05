#pragma once
#include <string>
#include <vector>
#include <pbc/pbc.h>

namespace ibemail {
namespace hash {

// H1: Maps ID to G1 element
void H1(element_t out_q_id, const std::string& id);

// H2: Maps GT element to n bytes
std::vector<unsigned char> H2(element_t gt_element, size_t n_bytes);

// H3: Maps sigma || M to a Zq* scalar
void H3(element_t out_r, const std::vector<unsigned char>& sigma, const std::vector<unsigned char>& M);

// H4: Maps sigma to n bytes
std::vector<unsigned char> H4(const std::vector<unsigned char>& sigma, size_t n_bytes);

// Utility for hashing strings (e.g. passwords)
std::string sha256_string(const std::string& input);

}
}
