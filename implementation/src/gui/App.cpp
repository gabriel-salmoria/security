#include "App.hpp"
#include "../common/Utils.hpp"
#include <imgui.h>
#include <iostream>
#include <fstream>
#include <sstream>

namespace ibemail {
namespace gui {

App::App(const std::string& data_dir) : data_dir(data_dir), pkg(data_dir) {
    pkg.Initialize();
    mail_sys = std::make_unique<MailSystem>(data_dir, pkg.GetPublicParams());
}

void App::Render() {
    // Make the window fill the entire viewport
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("IBE Mail System", nullptr, 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Header
    ImGui::TextColored(ImVec4(0.40f, 0.72f, 0.90f, 1.0f), "IBE Mail");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.50f, 0.52f, 0.58f, 1.0f), "  |  Boneh-Franklin FullIdent Scheme");
    ImGui::Separator();

    if (ImGui::BeginTabBar("Tabs")) {
        if (!is_authenticated) {
            if (ImGui::BeginTabItem("Authentication / Registration")) {
                RenderAuthTab();
                ImGui::EndTabItem();
            }
        } else {
            if (ImGui::BeginTabItem("Inbox")) {
                RenderInboxTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Sent")) {
                RenderSentTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Compose")) {
                RenderComposeTab();
                ImGui::EndTabItem();
            }
        }
        
        if (ImGui::BeginTabItem("PKG Admin (Debug)")) {
            RenderAdminTab();
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }

    if (!status_msg.empty()) {
        ImGui::Separator();
        ImGui::Spacing();
        bool is_success = (status_msg.find("Authenticated") != std::string::npos || 
                           status_msg.find("sent") != std::string::npos ||
                           status_msg.find("Registered") != std::string::npos ||
                           status_msg.find("Decrypted") != std::string::npos);
        ImVec4 color = is_success ? ImVec4(0.30f, 0.80f, 0.50f, 1.0f) : ImVec4(1.0f, 0.45f, 0.40f, 1.0f);
        ImGui::TextColored(color, "%s", status_msg.c_str());
    }

    ImGui::End();
}

void App::RenderAuthTab() {
    ImGui::InputText("ID (Email)", input_id, IM_ARRAYSIZE(input_id));
    ImGui::InputText("Password", input_pass, IM_ARRAYSIZE(input_pass), ImGuiInputTextFlags_Password);

    if (ImGui::Button("Login / Extract Key")) {
        std::vector<unsigned char> key = pkg.ExtractKey(input_id, input_pass);
        if (key.empty()) {
            status_msg = "Auth failed or user not APPROVED.";
        } else {
            is_authenticated = true;
            current_user_id = input_id;
            private_key = key;
            status_msg = "Authenticated as " + current_user_id;
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Register")) {
        if (pkg.RegisterUser(input_id, input_pass)) {
            status_msg = "Registered successfully. Waiting for PKG approval.";
        } else {
            status_msg = "User already exists.";
        }
    }
}

void App::RenderInboxTab() {
    ImGui::Text("Logged in as: %s", current_user_id.c_str());
    if (ImGui::Button("Logout")) {
        is_authenticated = false;
        private_key.clear();
        current_user_id.clear();
        reading_email = std::nullopt;
        return;
    }

    ImGui::Separator();
    
    ImGui::BeginChild("MailList", ImVec2(220, 0), true);
    ImGui::TextColored(ImVec4(0.50f, 0.52f, 0.58f, 1.0f), "Messages");
    ImGui::Separator();
    auto msg_ids = mail_sys->GetInboxMessageIDs(current_user_id);
    for (const auto& id : msg_ids) {
        if (ImGui::Selectable(id.c_str(), selected_msg_id == id)) {
            selected_msg_id = id;
            reading_email = mail_sys->ReadEmail(current_user_id, selected_msg_id, private_key);
            if (!reading_email) {
                status_msg = "Failed to decrypt message (Invalid ciphertext or wrong key)";
            } else {
                status_msg = "Decrypted with " + current_user_id;
            }
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("MailContent", ImVec2(0, 0), true);
    if (reading_email) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "--- Decrypted Message ---");
        ImGui::Text("From: %s", reading_email->from.c_str());
        ImGui::Text("To: %s", reading_email->to.c_str());
        ImGui::Text("Date: %s", reading_email->date.c_str());
        ImGui::Text("Subject: %s", reading_email->subject.c_str());
        ImGui::Separator();
        ImGui::TextWrapped("%s", reading_email->body.c_str());
        ImGui::Separator();
    } else if (!selected_msg_id.empty()) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Decryption error (Invalid Ciphertext or Wrong Key).");
        ImGui::Separator();
    }

    if (!selected_msg_id.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "--- Raw Encrypted File (.ibemail) ---");
        std::string filepath = data_dir + "/mail/inbox/" + current_user_id + "/" + selected_msg_id + ".ibemail";
        std::ifstream file(filepath);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            ImGui::BeginChild("RawText", ImVec2(0, 200), true);
            ImGui::TextWrapped("%s", buffer.str().c_str());
            ImGui::EndChild();
        }
    }
    ImGui::EndChild();
}

void App::RenderSentTab() {
    ImGui::Text("Sent messages from: %s", current_user_id.c_str());
    ImGui::Separator();

    ImGui::BeginChild("SentList", ImVec2(220, 0), true);
    ImGui::TextColored(ImVec4(0.50f, 0.52f, 0.58f, 1.0f), "Sent Messages");
    ImGui::Separator();
    auto sent_ids = mail_sys->GetSentMessageIDs(current_user_id);
    for (const auto& id : sent_ids) {
        if (ImGui::Selectable(id.c_str(), selected_sent_id == id)) {
            selected_sent_id = id;
            reading_sent = mail_sys->ReadSentHeader(current_user_id, selected_sent_id);
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("SentContent", ImVec2(0, 0), true);
    if (reading_sent) {
        ImGui::Text("To: %s", reading_sent->to.c_str());
        ImGui::Text("Date: %s", reading_sent->date.c_str());
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Subject and body are encrypted in the .ibemail file.");
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Only the recipient (%s) can decrypt.", reading_sent->to.c_str());

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "--- Raw Encrypted File (.ibemail) ---");
        std::string filepath = data_dir + "/mail/sent/" + current_user_id + "/" + selected_sent_id + ".ibemail";
        std::ifstream file(filepath);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            ImGui::BeginChild("SentRawText", ImVec2(0, 200), true);
            ImGui::TextWrapped("%s", buffer.str().c_str());
            ImGui::EndChild();
        }
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Select a sent message to view its envelope.");
    }
    ImGui::EndChild();
}

void App::RenderComposeTab() {
    ImGui::InputText("To", comp_to, IM_ARRAYSIZE(comp_to));
    ImGui::InputText("Subject", comp_subj, IM_ARRAYSIZE(comp_subj));
    ImGui::InputTextMultiline("Body", comp_body, IM_ARRAYSIZE(comp_body), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));

    if (ImGui::Button("Send Encrypted Email")) {
        Email email;
        email.message_id = utils::generate_uuid_v4();
        email.from = current_user_id;
        email.to = comp_to;
        email.subject = comp_subj;
        email.body = comp_body;
        
        try {
            mail_sys->SendEmail(email);
            status_msg = "Email sent to " + std::string(comp_to);
            comp_to[0] = '\0';
            comp_subj[0] = '\0';
            comp_body[0] = '\0';
        } catch (std::exception& e) {
            status_msg = "Error sending: " + std::string(e.what());
        }
    }
}

void App::RenderAdminTab() {
    auto reg = pkg.GetRegistry();
    if (ImGui::BeginTable("PKG Registry", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Status");
        ImGui::TableSetupColumn("Action");
        ImGui::TableHeadersRow();

        for (const auto& [id, record] : reg) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", id.c_str());
            
            ImGui::TableNextColumn();
            std::string st = "PENDING";
            if (record.status == UserStatus::APPROVED) st = "APPROVED";
            else if (record.status == UserStatus::REVOKED) st = "REVOKED";
            ImGui::Text("%s", st.c_str());
            
            ImGui::TableNextColumn();
            ImGui::PushID(id.c_str());
            if (record.status != UserStatus::APPROVED && ImGui::Button("Approve")) {
                pkg.SetUserStatus(id, UserStatus::APPROVED);
            }
            ImGui::SameLine();
            if (record.status != UserStatus::REVOKED && ImGui::Button("Revoke")) {
                pkg.SetUserStatus(id, UserStatus::REVOKED);
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

}
}
