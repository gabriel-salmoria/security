#pragma once
#include <string>
#include <vector>
#include <map>
#include "../common/BonehFranklin.hpp"

namespace ibemail {

enum class UserStatus {
    PENDING,
    APPROVED,
    REVOKED
};

struct UserRecord {
    std::string password_hash;
    UserStatus status;
};

class PKG {
public:
    PKG(const std::string& data_dir);

    // Initialize or load master secret and params
    void Initialize();

    // User Registration
    bool RegisterUser(const std::string& id, const std::string& password);

    // Admin functions
    std::map<std::string, UserRecord> GetRegistry() const;
    void SetUserStatus(const std::string& id, UserStatus status);

    // User authentication & key extraction
    // Returns empty vector if authentication fails, or status is not APPROVED
    std::vector<unsigned char> ExtractKey(const std::string& id, const std::string& password);

    // Get public parameters (for users to encrypt)
    std::vector<unsigned char> GetPublicParams() const;

private:
    void LoadRegistry();
    void SaveRegistry();
    void SeedDefaultUsers();
    
    std::string data_dir;
    std::map<std::string, UserRecord> registry;
    BonehFranklin bf;
    std::vector<unsigned char> pub_params;
    std::vector<unsigned char> master_secret;
};

}
