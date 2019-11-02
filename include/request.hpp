#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <vector>
#include <map>
#include "header.hpp"

namespace craftsoft {
namespace server {

/// A request received from a client.
struct Request {
    std::string method;
    std::string uri;
    std::string body;
    int http_version_major;
    int http_version_minor;
    std::vector<header> headers;
    std::vector<header> cookies;
    mutable std::map<std::string, std::string> paths;
    mutable std::map<std::string, std::string> query;

    Request() {
        method.reserve(20);
        uri.reserve(500);
        body.reserve(2000);
        headers.reserve(10);
        cookies.reserve(10);
    }


    inline bool isPath (std::string&& p) {
        return paths.find(p) != paths.end();
    }

    inline bool isQuery (std::string&& q) {
        return query.find(q) != query.end();
    }
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HPP
