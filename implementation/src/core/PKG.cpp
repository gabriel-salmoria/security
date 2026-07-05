#include "PKG.hpp"
#include "../common/Hash.hpp"
#include "../common/Utils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

namespace ibemail {

// Default Type A parameters for PBC
extern const std::string TYPE_A_PARAMS = 
"type a\n"
"q 8780710799663312522437781984754049815806883199414208211028653399266475630880222957078625179422662221423155858769582317459277713367317481324925129998224791\n"
"h 12016012264891146079388821366740534204802954401251311822919615131047207289359704531102844802183906537786776\n"
"r 730750818665451621361119245571504901405976559617\n"
"exp2 159\n"
"exp1 107\n"
"sign1 1\n"
"sign0 1\n";

PKG::PKG(const std::string& data_dir) : data_dir(data_dir), bf(TYPE_A_PARAMS) {
}

void PKG::Initialize() {
    std::string master_sec_path = data_dir + "/master.sec";
    std::string params_pub_path = data_dir + "/params.pub";

    std::ifstream f_sec(master_sec_path);
    std::ifstream f_pub(params_pub_path);

    if (f_sec.good() && f_pub.good()) {
        std::stringstream buf_sec, buf_pub;
        buf_sec << f_sec.rdbuf();
        buf_pub << f_pub.rdbuf();
        master_secret = utils::base64_decode(buf_sec.str());
        pub_params = utils::base64_decode(buf_pub.str());
        bf.LoadPKG(master_secret, pub_params);
    } else {
        auto [sec, pub] = bf.Setup();
        master_secret = sec;
        pub_params = pub;

        std::ofstream out_sec(master_sec_path);
        out_sec << utils::base64_encode(master_secret);
        std::ofstream out_pub(params_pub_path);
        out_pub << utils::base64_encode(pub_params);
    }
    LoadRegistry();
}

void PKG::LoadRegistry() {
    std::ifstream file(data_dir + "/registry.dat");
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string id, hash, status_str;
        ss >> id >> hash >> status_str;
        UserStatus status = UserStatus::PENDING;
        if (status_str == "approved") status = UserStatus::APPROVED;
        if (status_str == "revoked") status = UserStatus::REVOKED;
        registry[id] = {hash, status};
    }
}

void PKG::SaveRegistry() {
    std::ofstream file(data_dir + "/registry.dat");
    for (const auto& [id, record] : registry) {
        std::string status_str = "pending";
        if (record.status == UserStatus::APPROVED) status_str = "approved";
        if (record.status == UserStatus::REVOKED) status_str = "revoked";
        file << id << "\t" << record.password_hash << "\t" << status_str << "\n";
    }
}

bool PKG::RegisterUser(const std::string& id, const std::string& password) {
    if (registry.count(id)) return false;
    registry[id] = {hash::sha256_string(password), UserStatus::PENDING};
    SaveRegistry();
    return true;
}

std::map<std::string, UserRecord> PKG::GetRegistry() const {
    return registry;
}

void PKG::SetUserStatus(const std::string& id, UserStatus status) {
    if (registry.count(id)) {
        registry[id].status = status;
        SaveRegistry();
        
        if (status == UserStatus::REVOKED) {
            std::string key_file = data_dir + "/keys/" + id + ".prv";
            remove(key_file.c_str());
        }
    }
}

std::vector<unsigned char> PKG::ExtractKey(const std::string& id, const std::string& password) {
    if (!registry.count(id)) return {};
    auto& record = registry[id];
    if (record.status != UserStatus::APPROVED) return {};
    
    if (record.password_hash != hash::sha256_string(password)) return {};

    std::vector<unsigned char> d_ID = bf.Extract(id);
    
    // Save to disk (simulating delivering the key to the user)
    std::string key_file = data_dir + "/keys/" + id + ".prv";
    std::ofstream out(key_file);
    out << utils::base64_encode(d_ID);
    
    return d_ID;
}

std::vector<unsigned char> PKG::GetPublicParams() const {
    return pub_params;
}

}
