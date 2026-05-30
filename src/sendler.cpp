#include <Sendler.h>
#include <iostream>
#include "httplib.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

std::string FooSendler::send(std::string command) {
    const std::string url = "http://" + m_ip + ":" + m_port;
    httplib::Client client(url);

    const std::string body = json{{"command", command}}.dump();
    auto resp = client.Post("/commands", body, "application/json");

    if (!resp) {
        std::cerr << "[sender] нет соединения с " << url << '\n';
        return "ConnectionFailed";
    }
    if (resp->status != 200) {
        std::cerr << "[sender] HTTP " << resp->status << ": " << resp->body << '\n';
        return "Error";
    }
    std::cout << "[sender] " << resp->body << '\n';
    return "Success";
}
