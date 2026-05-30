#include <iostream>
#include "httplib.h"
#include "json.hpp"
#include "gpio_control.h"

using json = nlohmann::json;

enum class Command { START, STOP };

class EngineStateMachine {
private:
    enum class State { STOPPED, RUNNING } state = State::STOPPED;

public:
    void process(Command cmd) {
        if (cmd == Command::START && state == State::STOPPED) {
            std::cout << "Переход: STOPPED -> RUNNING. Включаем мотор." << std::endl;
            start();
            state = State::RUNNING;
        } else if (cmd == Command::STOP && state == State::RUNNING) {
            std::cout << "Переход: RUNNING -> STOPPED. Выключаем мотор." << std::endl;
            stop();
            state = State::STOPPED;
        } else {
            std::cout << "Команда проигнорирована: мотор уже в состоянии " << get_status_string() << std::endl;
        }
    }

    std::string get_status_string() const {
        return (state == State::RUNNING) ? "running" : "stopped";
    }
};

int main(int argc, char* argv[]) {
    std::string ip = "0.0.0.0";
    int port = 8080;

    if (argc >= 2) ip   = argv[1];
    if (argc >= 3) port = std::stoi(argv[2]);

    EngineStateMachine engine_fsm;
    httplib::Server svr;

    // GET: текущее состояние мотора
    svr.Get("/commands", [&engine_fsm](const httplib::Request&, httplib::Response& res) {
        json body;
        body["engine_status"] = engine_fsm.get_status_string();
        res.set_content(body.dump(), "application/json");
        res.status = 200;
    });

    // POST: команда мотору
    svr.Post("/commands", [&engine_fsm](const httplib::Request& req, httplib::Response& res) {
        if (req.get_header_value("Content-Type") != "application/json") {
            res.status = 415;
            res.set_content("Unsupported Content-Type. Expected application/json", "text/plain");
            return;
        }

        try {
            json req_json  = json::parse(req.body);
            std::string cmd_name = req_json.value("command", "none");

            json res_json;
            if (cmd_name == "start") {
                engine_fsm.process(Command::START);
            } else if (cmd_name == "stop") {
                engine_fsm.process(Command::STOP);
            } else {
                res_json["status"]  = "error";
                res_json["message"] = "Unknown command: " + cmd_name;
                res.status = 400;
                res.set_content(res_json.dump(), "application/json");
                return;
            }

            res_json["status"]               = "success";
            res_json["message"]              = "Command processed by Motor";
            res_json["current_engine_state"] = engine_fsm.get_status_string();
            res.status = 200;
            res.set_content(res_json.dump(), "application/json");

        } catch (const json::parse_error& e) {
            res.status = 400;
            res.set_content("Invalid JSON: " + std::string(e.what()), "text/plain");
        }
    });

    std::cout << "Server listening on http://" << ip << ":" << port << std::endl;
    svr.listen(ip, port);
    return 0;
}
