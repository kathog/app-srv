#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#include "../include/server.hpp"
#include <signal.h>
#include <utility>
#include "../include/request_handler.hpp"
#include <regex>
#include <vector>
#include <future>

namespace craftsoft {
namespace server {

Server::Server(const std::string& address, const std::string& port, const std::string& doc_root) : ioContext_(), signals_(ioContext_), acceptor_(ioContext_), connectionManager_(), requestHandler_(doc_root) {
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif

    doAwaitStop();

    boost::asio::ip::tcp::resolver resolver(ioContext_);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen(400);

    doAccept();
}

void Server::run(int nThreads) {
    if ( nThreads > 1 ) {
        std::vector<std::future<void>> futures;
        for (int i =0; i < nThreads; i++) {
            auto f = std::async (std::launch::async, [&]() {
                ioContext_.run();
            });
            futures.push_back(std::move(f));
        }

        for (auto & f : futures) {
            f.wait();
        }


    } else {
        ioContext_.run();
    }
}

void Server::doAccept() {
    acceptor_.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
        if (!acceptor_.is_open()) {
            return;
        }
        if (!ec) {
            connectionManager_.start(std::make_shared<Connection>(ioContext_, std::move(socket), connectionManager_, requestHandler_));
        }
        doAccept();
    });
}

void Server::doAwaitStop() {
    signals_.async_wait([this](boost::system::error_code ec, int signo) {
        acceptor_.close();
        connectionManager_.stop_all();
    });
}


std::shared_ptr<Method> Server::get() {
    if (methods.find("GET") == methods.end()) {
        std::shared_ptr<Method> m = std::make_shared<Method>("GET");
        methods.insert(make_pair("GET", m));
        return m;
    } else {
        return methods.at("GET");
    }
}

std::shared_ptr<Method> Server::put() {
    if (methods.find("PUT") == methods.end()) {
        std::shared_ptr<Method> m = std::make_shared<Method>("PUT");
        methods.insert(make_pair("PUT", m));
        return m;
    } else {
        return methods.at("PUT");
    }
}

std::shared_ptr<Method> Server::del() {
    if (methods.find("DELETE") == methods.end()) {
        std::shared_ptr<Method> m = std::make_shared<Method>("DELETE");
        methods.insert(make_pair("DELETE", m));
        return m;
    } else {
        return methods.at("DELETE");
    }
}

std::shared_ptr<Method> Server::post() {
    if (methods.find("POST") == methods.end()) {
        std::shared_ptr<Method> m = std::make_shared<Method>("POST");
        methods.insert(make_pair("POST", m));
        return m;
    } else {
        return methods.at("POST");
    }
}

std::shared_ptr<Method> Server::options() {
    if (methods.find("OPTIONS") == methods.end()) {
        std::shared_ptr<Method> m = std::make_shared<Method>("OPTIONS");
        methods.insert(make_pair("OPTIONS", m));
        return m;
    } else {
        return methods.at("OPTIONS");
    }
}

} // namespace server
} // namespace craftsoft
