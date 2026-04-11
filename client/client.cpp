#include "httplib.h"
#include "nlohmann/json.hpp"
#include <iostream>

using json = nlohmann::json;

int main()
{
    httplib::Client cli("localhost", 8080);

    // Формируем JSON, как в Python‑примере
    json request_json;
    request_json["command"] = "forward";
    request_json["id"] = 33;
    request_json["time"] = 1.00;

    // Преобразуем JSON в строку
    std::string body = request_json.dump();

    // Отправляем POST‑запрос на /commands
    auto post_res = cli.Post("/commands", body, "application/json");

    if (post_res && post_res->status == 200) {
        std::cout << "POST запрос успешен!" << std::endl;
        std::cout << "Ответ сервера: " << post_res->body << std::endl;
    } else {
        std::cout << "POST запрос не удался";
        if (post_res) {
            std::cout << " (статус " << post_res->status << ")";
        } else {
            std::cout << " (ошибка соединения)";
        }
        std::cout << std::endl;
    }

    auto get_res = cli.Get("/commands");

    if (get_res && get_res->status == 200) {
        std::cout << "GET запрос успешен!" << std::endl;
        std::cout << "Ответ: " << get_res->body << std::endl;
    } else {
        std::cout << "GET запрос не удался";
        if (get_res) {
            std::cout << " (статус " << get_res->status << ")";
        } else {
            std::cout << " (ошибка соединения)";
        }
        std::cout << std::endl;
    }

    return 0;
}