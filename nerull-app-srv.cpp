// boost-asio.cpp: definiuje punkt wejścia dla aplikacji.
//
#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "include/server.hpp"

#include <boost/regex.hpp>
#include <regex>
#include <pcre.h>
#include <chrono>
#include <thread>
#include <future>


using namespace craftsoft::server;

int main(int argc, char* argv[]) {
	try {
		// Check command line arguments.
		if (argc != 4) {
			std::cerr << "Usage: http_server <address> <port> <doc_root>\n";
			std::cerr << "  For IPv4, try:\n";
			std::cerr << "    receiver 0.0.0.0 80 .\n";
			std::cerr << "  For IPv6, try:\n";
			std::cerr << "    receiver 0::0 80 .\n";
			//return 1;
		}

		// Initialise the server.
        craftsoft::server::Server s("0.0.0.0", "9080", ".");
        s.get()->handler("/post/{id}", [&](const Request & req, Response & response) {
//                cout << "id wątka: " << std::this_thread::get_id() << endl;

            response.content.append("{\"body\": \"").append(req.paths["id"]).append("\"}");
            response.addHeader("Content-Type", "application/json");
        });


        s.get()->handler("/search", [&](const Request & req, Response & response) {
            //std::cout << &req << std::endl;
            response.content.append("{\"search\": \"").append(req.query["dupa"]).append("\"}");
            response.addHeader("Content-Type", "application/json");
        }).before([&](const Request & req, Response & response) -> bool {
            return true;
        });
		

        s.get()->handler("/tag/{name}/{tag}", [&](const Request & req, Response & response) {
            response.content.append("{\"tag\": \"").append(req.paths["name"]).append(req.paths["tag"]).append("\"}");
            response.addHeader("Content-Type", "application/json");
        }).before([&](const Request & req, Response & response) -> bool {
            return true;
        });

        s.get()->handler("/home", [&](const Request & req, Response & response) {
            response.content.append("{\"title\": \"HOME\"}");
            response.addHeader("Content-Type", "application/json");
        }).before([&](const Request & req, Response & response) -> bool {
            return true;
        });

        s.get()->handler("/", [&](const Request & req, Response & response) {
            response.content.append("{\"title\": \"ROOT\"}");
            response.addHeader("Content-Type", "application/json");
        }).before([&](const Request & req, Response & response) -> bool {
            return true;
        });

        s.run(40);

	} catch (std::exception & e) {
		std::cerr << "exception: " << e.what() << "\n";
	}

	return 0;
}
