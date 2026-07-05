#pragma once
#include <string>
#include <vector>

namespace ibemail {
namespace utils {

std::string generate_uuid_v4();
std::string base64_encode(const std::vector<unsigned char>& data);
std::vector<unsigned char> base64_decode(const std::string& input);
std::vector<unsigned char> xor_bytes(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b);

}
}
