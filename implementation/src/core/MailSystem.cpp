#include "MailSystem.hpp"
#include "../common/Utils.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <chrono>

namespace ibemail {

// Extracted from PKG.cpp for simplicity, ideally we should declare it in a common header
extern const std::string TYPE_A_PARAMS;

MailSystem::MailSystem(const std::string& data_dir, const std::vector<unsigned char>& pub_params) 
    : data_dir(data_dir), bf(TYPE_A_PARAMS) {
    bf.LoadUser(pub_params);
}

void MailSystem::SendEmail(const Email& email) {
    std::string plain_content = "Subject: " + email.subject + "\n\n" + email.body;
    std::vector<unsigned char> m_bytes(plain_content.begin(), plain_content.end());
    
    Ciphertext C = bf.Encrypt(email.to, m_bytes);

    std::stringstream out;
    out << "IBE-MAIL/1.0\n";
    out << "Message-ID: " << email.message_id << "\n";
    out << "From: " << email.from << "\n";
    out << "To: " << email.to << "\n";
    
    // Format date if not provided
    std::string date = email.date;
    if (date.empty()) {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        char buf[100];
        std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", std::localtime(&time));
        date = buf;
    }
    out << "Date: " << date << "\n";
    out << "---\n";
    out << "U: " << utils::base64_encode(C.U) << "\n";
    out << "V: " << utils::base64_encode(C.V) << "\n";
    out << "W: " << utils::base64_encode(C.W) << "\n";

    // Write to sender's sent box
    std::string sent_dir = data_dir + "/mail/sent/" + email.from;
    std::filesystem::create_directories(sent_dir);
    std::ofstream out_sent(sent_dir + "/" + email.message_id + ".ibemail");
    out_sent << out.str();

    // Write to receiver's inbox
    std::string inbox_dir = data_dir + "/mail/inbox/" + email.to;
    std::filesystem::create_directories(inbox_dir);
    std::ofstream out_inbox(inbox_dir + "/" + email.message_id + ".ibemail");
    out_inbox << out.str();
}

std::vector<std::string> MailSystem::GetInboxMessageIDs(const std::string& id) {
    std::vector<std::string> ids;
    std::string inbox_dir = data_dir + "/mail/inbox/" + id;
    if (std::filesystem::exists(inbox_dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(inbox_dir)) {
            if (entry.path().extension() == ".ibemail") {
                ids.push_back(entry.path().stem().string());
            }
        }
    }
    return ids;
}

std::vector<std::string> MailSystem::GetSentMessageIDs(const std::string& id) {
    std::vector<std::string> ids;
    std::string sent_dir = data_dir + "/mail/sent/" + id;
    if (std::filesystem::exists(sent_dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(sent_dir)) {
            if (entry.path().extension() == ".ibemail") {
                ids.push_back(entry.path().stem().string());
            }
        }
    }
    return ids;
}

std::optional<Email> MailSystem::ReadSentHeader(const std::string& id, const std::string& message_id) {
    std::string filepath = data_dir + "/mail/sent/" + id + "/" + message_id + ".ibemail";
    std::ifstream file(filepath);
    if (!file.is_open()) return std::nullopt;

    Email email;
    email.message_id = message_id;
    std::string line;
    while (std::getline(file, line)) {
        if (line == "---") break; // Stop at the ciphertext section
        if (line.find("From: ") == 0) email.from = line.substr(6);
        else if (line.find("To: ") == 0) email.to = line.substr(4);
        else if (line.find("Date: ") == 0) email.date = line.substr(6);
    }
    email.subject = "(encrypted)";
    email.body = "(encrypted content — only the recipient can decrypt)";
    return email;
}

std::optional<Email> MailSystem::ReadEmail(const std::string& id, const std::string& message_id, const std::vector<unsigned char>& private_key) {
    std::string filepath = data_dir + "/mail/inbox/" + id + "/" + message_id + ".ibemail";
    std::ifstream file(filepath);
    if (!file.is_open()) return std::nullopt;

    Email email;
    Ciphertext C;
    
    std::string line;
    bool in_header = true;
    while (std::getline(file, line)) {
        if (line == "---") {
            in_header = false;
            continue;
        }
        if (in_header) {
            if (line.find("Message-ID: ") == 0) email.message_id = line.substr(12);
            else if (line.find("From: ") == 0) email.from = line.substr(6);
            else if (line.find("To: ") == 0) email.to = line.substr(4);
            else if (line.find("Date: ") == 0) email.date = line.substr(6);
        } else {
            try {
                if (line.find("U: ") == 0) C.U = utils::base64_decode(line.substr(3));
                else if (line.find("V: ") == 0) C.V = utils::base64_decode(line.substr(3));
                else if (line.find("W: ") == 0) C.W = utils::base64_decode(line.substr(3));
            } catch (const std::exception& e) {
                return std::nullopt;
            }
        }
    }

    try {
        auto decrypted = bf.Decrypt(C, private_key);
        if (!decrypted) return std::nullopt;

        std::string plain(decrypted->begin(), decrypted->end());
        size_t subj_pos = plain.find("Subject: ");
        size_t body_pos = plain.find("\n\n");
        if (subj_pos != std::string::npos && body_pos != std::string::npos) {
            email.subject = plain.substr(subj_pos + 9, body_pos - (subj_pos + 9));
            email.body = plain.substr(body_pos + 2);
        } else {
            email.body = plain; // fallback
        }
    } catch (const std::exception& e) {
        return std::nullopt; // Failed to parse Base64 or decrypt properly
    }

    return email;
}

}
