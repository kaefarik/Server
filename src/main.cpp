#include <App.h>
#include <Recognizer.h>
#include <Sendler.h>
#include <iostream>

static void printUsage(const char* prog) {
    std::cout << "Usage: " << prog
              << " -ip <addr> -p <port> [-c | -v <path>]\n";
}

int main(int argc, char* argv[]) {
    std::string ip   = "0.0.0.0";
    std::string port = "8080";
    std::string videoPath = "../video/SkySun.mp4";
    bool useCamera = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if      (arg == "-ip" && i+1 < argc) ip        = argv[++i];
        else if (arg == "-p"  && i+1 < argc) port      = argv[++i];
        else if (arg == "-v"  && i+1 < argc) videoPath = argv[++i];
        else if (arg == "-c")                useCamera = true;
        else { printUsage(argv[0]); return 1; }
    }

    auto recognizer = std::make_unique<FooRecognizer>();
    auto sender     = std::make_unique<FooSendler>(ip, port);
    App app(std::move(recognizer), std::move(sender), useCamera, videoPath);
    return app.run();
}
