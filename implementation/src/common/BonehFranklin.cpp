#include "BonehFranklin.hpp"
#include "Hash.hpp"
#include "Utils.hpp"
#include <stdexcept>
#include <random>

namespace ibemail {

BonehFranklin::BonehFranklin(const std::string& param_str) {
    pairing_init_set_str(pairing, param_str.c_str());
    element_init_G1(P, pairing);
    element_init_G1(P_pub, pairing);
    element_init_Zr(s, pairing);
    has_P = false;
    has_s = false;
    n_bytes = 32;
}

BonehFranklin::~BonehFranklin() {
    element_clear(P);
    element_clear(P_pub);
    element_clear(s);
    pairing_clear(pairing);
}

std::pair<std::vector<unsigned char>, std::vector<unsigned char>> BonehFranklin::Setup() {
    element_random(P);
    element_random(s);
    element_mul_zn(P_pub, P, s);

    has_P = true;
    has_s = true;

    int g1_len = element_length_in_bytes(P);
    int zr_len = element_length_in_bytes(s);

    std::vector<unsigned char> s_bytes(zr_len);
    element_to_bytes(s_bytes.data(), s);

    std::vector<unsigned char> pub_params(2 * g1_len);
    element_to_bytes(pub_params.data(), P);
    element_to_bytes(pub_params.data() + g1_len, P_pub);

    return {s_bytes, pub_params};
}

void BonehFranklin::LoadPKG(const std::vector<unsigned char>& s_bytes, const std::vector<unsigned char>& pub_params_bytes) {
    int g1_len = element_length_in_bytes(P);
    if (pub_params_bytes.size() != (size_t)(2 * g1_len)) throw std::runtime_error("Invalid public params size");

    element_from_bytes(s, (unsigned char*)s_bytes.data());
    element_from_bytes(P, (unsigned char*)pub_params_bytes.data());
    element_from_bytes(P_pub, (unsigned char*)pub_params_bytes.data() + g1_len);

    has_s = true;
    has_P = true;
}

void BonehFranklin::LoadUser(const std::vector<unsigned char>& pub_params_bytes) {
    int g1_len = element_length_in_bytes(P);
    if (pub_params_bytes.size() != (size_t)(2 * g1_len)) throw std::runtime_error("Invalid public params size");

    element_from_bytes(P, (unsigned char*)pub_params_bytes.data());
    element_from_bytes(P_pub, (unsigned char*)pub_params_bytes.data() + g1_len);

    has_P = true;
    has_s = false;
}

std::vector<unsigned char> BonehFranklin::Extract(const std::string& ID) {
    if (!has_s) throw std::runtime_error("PKG master secret not loaded");

    element_t Q_ID, d_ID;
    element_init_G1(Q_ID, pairing);
    element_init_G1(d_ID, pairing);

    hash::H1(Q_ID, ID);
    element_mul_zn(d_ID, Q_ID, s);

    std::vector<unsigned char> d_ID_bytes(element_length_in_bytes(d_ID));
    element_to_bytes(d_ID_bytes.data(), d_ID);

    element_clear(Q_ID);
    element_clear(d_ID);
    return d_ID_bytes;
}

Ciphertext BonehFranklin::Encrypt(const std::string& ID, const std::vector<unsigned char>& M) {
    if (!has_P) throw std::runtime_error("Public params not loaded");

    element_t Q_ID, r;
    element_init_G1(Q_ID, pairing);
    element_init_Zr(r, pairing);

    // 1. Q_ID = H1(ID)
    hash::H1(Q_ID, ID);

    // 2. sigma in {0,1}^n
    std::vector<unsigned char> sigma(n_bytes);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (size_t i = 0; i < n_bytes; ++i) sigma[i] = dis(gen);

    // 3. r = H3(sigma, M)
    hash::H3(r, sigma, M);

    Ciphertext C;

    // 4. U = r * P
    element_t U;
    element_init_G1(U, pairing);
    element_mul_zn(U, P, r);
    C.U.resize(element_length_in_bytes(U));
    element_to_bytes(C.U.data(), U);

    // e(Q_ID, P_pub)^r
    element_t pairing_res;
    element_init_GT(pairing_res, pairing);
    pairing_apply(pairing_res, Q_ID, P_pub, pairing);
    element_pow_zn(pairing_res, pairing_res, r);

    // V = sigma XOR H2(e(Q_ID, P_pub)^r)
    std::vector<unsigned char> H2_res = hash::H2(pairing_res, n_bytes);
    C.V = utils::xor_bytes(sigma, H2_res);

    // W = M XOR H4(sigma)
    std::vector<unsigned char> H4_res = hash::H4(sigma, M.size());
    C.W = utils::xor_bytes(M, H4_res);

    element_clear(Q_ID);
    element_clear(r);
    element_clear(U);
    element_clear(pairing_res);

    return C;
}

std::optional<std::vector<unsigned char>> BonehFranklin::Decrypt(const Ciphertext& C, const std::vector<unsigned char>& d_ID_bytes) {
    if (!has_P) throw std::runtime_error("Public params not loaded");

    element_t U, d_ID;
    element_init_G1(U, pairing);
    element_init_G1(d_ID, pairing);

    element_from_bytes(U, (unsigned char*)C.U.data());
    element_from_bytes(d_ID, (unsigned char*)d_ID_bytes.data());

    // e(d_ID, U)
    element_t pairing_res;
    element_init_GT(pairing_res, pairing);
    pairing_apply(pairing_res, d_ID, U, pairing);

    // sigma_prime = V XOR H2(e(d_ID, U))
    std::vector<unsigned char> H2_res = hash::H2(pairing_res, n_bytes);
    std::vector<unsigned char> sigma_prime = utils::xor_bytes(C.V, H2_res);

    // M_prime = W XOR H4(sigma_prime)
    std::vector<unsigned char> H4_res = hash::H4(sigma_prime, C.W.size());
    std::vector<unsigned char> M_prime = utils::xor_bytes(C.W, H4_res);

    // r_prime = H3(sigma_prime, M_prime)
    element_t r_prime, r_prime_P;
    element_init_Zr(r_prime, pairing);
    element_init_G1(r_prime_P, pairing);

    hash::H3(r_prime, sigma_prime, M_prime);
    element_mul_zn(r_prime_P, P, r_prime);

    // Consistency check: U == r_prime * P
    bool success = (element_cmp(U, r_prime_P) == 0);

    element_clear(U);
    element_clear(d_ID);
    element_clear(pairing_res);
    element_clear(r_prime);
    element_clear(r_prime_P);

    if (success) {
        return M_prime;
    } else {
        return std::nullopt;
    }
}

}
