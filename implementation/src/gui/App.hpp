#pragma once
#include "../core/PKG.hpp"
#include "../core/MailSystem.hpp"
#include <string>
#include <memory>
#include <vector>

namespace ibemail {
namespace gui {

class App {
public:
    App(const std::string& data_dir);
    void Render();

private:
    std::string data_dir;
    PKG pkg;
    std::unique_ptr<MailSystem> mail_sys;

    // State
    std::string current_user_id;
    bool is_authenticated = false;
    std::vector<unsigned char> private_key;

    // UI state
    int current_tab = 0; 

    // Buffers for input
    char input_id[256] = "";
    char input_pass[256] = "";
    char comp_to[256] = "";
    char comp_subj[256] = "";
    char comp_body[4096] = "";

    // Error / Msg state
    std::string status_msg;
    
    // Inbox state
    std::string selected_msg_id;
    std::optional<Email> reading_email;

    // Sent state
    std::string selected_sent_id;
    std::optional<Email> reading_sent;

    void RenderAuthTab();
    void RenderInboxTab();
    void RenderSentTab();
    void RenderComposeTab();
    void RenderAdminTab();
};

}
}
