#pragma once
#include <vector>
#include <string>
#include <pbc/pbc.h>
#include <optional>

namespace ibemail {

struct Ciphertext {
    std::vector<unsigned char> U; // Serialized G1 element
    std::vector<unsigned char> V; // n bytes mask
    std::vector<unsigned char> W; // n bytes mask
};

class BonehFranklin {
public:
    // Initializes the pairing context from a param string
    BonehFranklin(const std::string& param_str);
    ~BonehFranklin();

    // 1. Setup: generates a random generator P, master secret s, and public key P_pub.
    // Returns a pair of: (master_secret_bytes, public_params_bytes (P || P_pub))
    std::pair<std::vector<unsigned char>, std::vector<unsigned char>> Setup();

    // Load state for PKG (needs master secret and public params)
    void LoadPKG(const std::vector<unsigned char>& s_bytes, const std::vector<unsigned char>& pub_params_bytes);
    
    // Load state for normal User (needs only public params)
    void LoadUser(const std::vector<unsigned char>& pub_params_bytes);

    // 2. Extract: PKG computes private key for ID
    // Returns serialized d_ID
    std::vector<unsigned char> Extract(const std::string& ID);

    // 3. Encrypt: Encrypts message M for ID
    Ciphertext Encrypt(const std::string& ID, const std::vector<unsigned char>& M);

    // 4. Decrypt: Decrypts ciphertext C using private key d_ID_bytes
    // Returns the message M, or std::nullopt if the consistency check fails.
    std::optional<std::vector<unsigned char>> Decrypt(const Ciphertext& C, const std::vector<unsigned char>& d_ID_bytes);

private:
    pairing_t pairing;
    element_t P;
    element_t P_pub;
    element_t s; 
    bool has_P;
    bool has_s;
    
    size_t n_bytes; // Chosen as 32 (SHA-256 size)
};

}
