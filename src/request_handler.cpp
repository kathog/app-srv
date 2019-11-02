#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#include "../include/request_handler.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include "../include/mime_types.hpp"
#include "../include/reply.hpp"
#include "../include/request.hpp"
#include "../include/server.hpp"
#include <regex>
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <chrono>
#include <pcre.h>
#include <string_view>

namespace craftsoft {
namespace server {


std::string srvVersion = "nerull_app_srv/0.13";
std::string srvVersionName = "Server";
const char delimiter = '&';
const char iDelimiter = '=';
const char de = '/';

auto stack = pcre_jit_stack_alloc(16384, 16384);

thread_local std::vector<std::string_view> patternValues;
thread_local std::vector<std::string_view> pathValues;
thread_local std::vector<std::string_view> qrySplit;
thread_local std::vector<std::string_view> innerSplit;


RequestHandler::RequestHandler(const std::string& doc_root): doc_root_(doc_root) {
}

void RequestHandler::buildRequestPaths(const Request& req, std::string pattern, std::string_view requestPath) {
    std::string_view text(pattern);
    const char * data = text.data();
    int idx[20];
    int loop = 0;
    for (int i = 0; i < text.size(); i++) {
        const char c = data[i];
        if (c == de) {
            idx[loop ++] = i;
        }
    }
    patternValues.clear();
    for (int i = 0; i < loop; i++) {
        if (i+1 == loop) {
            patternValues.emplace_back(text.substr(idx[i]+1));
        } else {
            patternValues.emplace_back(text.substr(idx[i]+1, (idx[i+1]-1 -idx[i])));
        }

    }

    data = requestPath.data();
    int idx0[20];
    int loop0 = 0;
    for (int i = 0; i < requestPath.size(); i++) {
        const char c = data[i];
        if (c == de) {
            idx0[loop0 ++] = i;
        }
    }
    pathValues.clear();
    for (int i =0; i < loop; i++) {
        if (i+1 == loop0) {
            pathValues.emplace_back(requestPath.substr(idx0[i]+1));
        } else {
            pathValues.emplace_back(requestPath.substr(idx0[i]+1, (idx0[i+1]-1 - idx0[i])));
        }
    }

    for (auto i = 0; i < loop; i++) {
        std::string_view v0 = patternValues[i];
        std::string_view v1 = pathValues[i];
        if (v0 != v1 && v0.length() > 0) {
            req.paths.insert(make_pair(v0.substr(1, v0.size() - 2), v1));
        }
    }
}

void RequestHandler::buildQueryParams(const Request& req, int f, std::string_view request_path) {
    const std::string_view query_ = request_path.substr(f);
    char * data = (char*) (query_.data());
    data[0] = '&';
    int idx[20];
    int loop = 0;
    for (int i = 0; i < query_.size(); i++) {
        const char c = data[i];
        if (c == delimiter) {
            idx[loop ++] = i;
        }
    }
    qrySplit.clear();
    for (int i =0; i < loop; i++) {
        if (i+1 == loop) {
            qrySplit.emplace_back(query_.substr(idx[i] + 1));
        } else {
            qrySplit.emplace_back(query_.substr(idx[i] + 1, (idx[i+1] - 1 - idx[i])));
        }
    }
    for (int i = 0; i < loop; i++) {
        auto qElem = qrySplit[i];
        int len = qElem.find(iDelimiter);
        if (len > 0) {
            req.query.insert(make_pair(qElem.substr(0, len), qElem.substr(len+1)));
        }
    }
}

void RequestHandler::handleRequest(const Request& req, Response& rep) {

//    auto time = std::chrono::high_resolution_clock::now();
    std::string requestPath0;
    if (!urlDecode(req.uri, requestPath0)) {
        rep = Response::stock_reply(Response::bad_request);
        return;
    }

    if (requestPath0.empty() || requestPath0[0] != '/' || requestPath0.find("..") != std::string::npos) {
        rep = Response::stock_reply(Response::bad_request);
        return;
    }

    std::string_view request_path(requestPath0);
    int f = request_path.find("?");
    string_view uri = f > 0 ? request_path.substr(0, f) : request_path;

    if (Server::methods.find(req.method) != Server::methods.end()) {
        std::vector<Handler> inner = Server::methods.at(req.method)->handlers;

        for (auto iHandler : inner) {
            int pos = 0;
            for(int match[3]; pcre_jit_exec(iHandler.reg, iHandler.aid, uri.data(), uri.size(), pos, 0, match, 3, stack) >= 0; pos = match[1]) {
                std::function<void(const Request&, Response&)>& invoker = iHandler.invoker;

                buildRequestPaths(req, iHandler.pattern, uri);
                if (f > 0) {
                    buildQueryParams(req, f, request_path);
                }

//                auto end = std::chrono::high_resolution_clock::now();
//                auto tResult = std::chrono::duration_cast<std::chrono::duration<double>>(end - time);
//                std::cout << tResult.count() * 1000 * 1000 << " us" << std::endl;

                try {
                    bool inv = true;
                    if (iHandler.before != nullptr) {
                        inv = iHandler.before(req, rep);
                    }
                    if (inv) {
                        invoker(req, rep);
                    }
                    if (iHandler.after != nullptr) {
                        iHandler.after(req, rep);
                    }
                    rep.addHeader(srvVersionName, srvVersion);
                } catch (std::exception & e) {
                    std::cerr << "exception: " << e.what() << "\n";
                    std::cerr << "body: " << req.body << "\n";
                    rep = Response::stock_reply(Response::internal_server_error);
                    rep.addHeader(srvVersionName, srvVersion);
                    return;
                }
                rep.addHeader("Content-Length", std::to_string(rep.content.size()));

                return;
            }
        }
    }
    // jesli zaden regexp nie znajdzie
    rep = Response::stock_reply(Response::not_found);
    rep.addHeader(srvVersionName, srvVersion);
}

bool RequestHandler::urlDecode(const std::string & in, std::string & out) {
    out.clear();
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%') {
            if (i + 3 <= in.size()) {
                int value = 0;
                std::istringstream is(in.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    out += static_cast<char>(value);
                    i += 2;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return true;
}

} // namespace server
} // namespace http
