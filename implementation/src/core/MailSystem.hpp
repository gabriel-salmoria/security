#pragma once
#include <string>
#include <vector>
#include <optional>
#include "../common/BonehFranklin.hpp"

namespace ibemail {

struct Email {
    std::string message_id;
    std::string from;
    std::string to;
    std::string date;
    std::string subject;
    std::string body;
};

class MailSystem {
public:
    MailSystem(const std::string& data_dir, const std::vector<unsigned char>& pub_params);

    // Send an email (encrypts subject and body)
    void SendEmail(const Email& email);

    // Receive emails for a specific ID
    // Reads all .ibemail files in data_dir/mail/inbox/ID/
    std::vector<std::string> GetInboxMessageIDs(const std::string& id);
    
    // List sent emails for a specific ID
    std::vector<std::string> GetSentMessageIDs(const std::string& id);

    // Read only the cleartext headers of a sent email (no decryption needed)
    std::optional<Email> ReadSentHeader(const std::string& id, const std::string& message_id);

    // Decrypt a specific email using private key
    std::optional<Email> ReadEmail(const std::string& id, const std::string& message_id, const std::vector<unsigned char>& private_key);

private:
    std::string data_dir;
    BonehFranklin bf;
};

}
