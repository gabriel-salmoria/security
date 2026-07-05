#include "Utils.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <stdexcept>

namespace ibemail {
namespace utils {

std::string generate_uuid_v4() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4";
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);
    return ss.str();
}

std::string base64_encode(const std::vector<unsigned char>& data) {
    if (data.empty()) return "";
    int out_len = 4 * ((data.size() + 2) / 3);
    std::string out(out_len, '\0');
    int len = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]), data.data(), data.size());
    if (len < 0) throw std::runtime_error("Base64 encode failed");
    return out;
}

std::vector<unsigned char> base64_decode(const std::string& input) {
    if (input.empty()) return {};
    std::vector<unsigned char> out(3 * input.size() / 4 + 1);
    int len = EVP_DecodeBlock(out.data(), reinterpret_cast<const unsigned char*>(input.data()), input.size());
    if (len < 0) throw std::runtime_error("Base64 decode failed");
    // Handle padding
    int padding = 0;
    if (input.size() > 0 && input[input.size() - 1] == '=') padding++;
    if (input.size() > 1 && input[input.size() - 2] == '=') padding++;
    out.resize(len - padding);
    return out;
}

std::vector<unsigned char> xor_bytes(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
    size_t len = std::min(a.size(), b.size());
    std::vector<unsigned char> out(len);
    for (size_t i = 0; i < len; ++i) {
        out[i] = a[i] ^ b[i];
    }
    return out;
}

}
}
