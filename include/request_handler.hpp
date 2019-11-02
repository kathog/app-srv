#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>
#include <map>
#include <functional>
#include <regex>
#include <vector>
#include <pcre.h>

using namespace std;

namespace craftsoft {
namespace server {

struct Response;
struct Request;
class Handler {

public:

    ~Handler () {
       // delete reg;
    }

    string pattern;
    function<void(const Request&, Response&)> invoker;
    pcre * reg;
    pcre_extra * aid;
    function<bool(const Request&, Response&)> before;
    function<void(const Request&, Response&)> after;

};

/// The common handler for all incoming requests.
class RequestHandler
{
public:
    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    /// Construct with a directory containing files to be served.
    explicit RequestHandler(const std::string& doc_root);

    /// Handle a request and produce a reply.
    void handleRequest(const Request& req, Response& rep);

    inline static map<string, std::vector<Handler>> handlers;

private:
    /// The directory containing the files to be served.
    std::string doc_root_;

    /// Perform URL-decoding on a string. Returns false if the encoding was
    /// invalid.
    static bool urlDecode(const std::string& in, std::string& out);

    void buildRequestPaths(const Request& req, std::string pattern, std::string_view request_path);
    void buildQueryParams(const Request& req, int f, std::string_view request_path);
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HANDLER_HPP
