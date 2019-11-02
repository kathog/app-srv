#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include "connection.hpp"
#include "connection_manager.hpp"
#include "request_handler.hpp"
#include <map>
#include <functional>
#include <regex>
#include <pcre.h>

using namespace std;

namespace craftsoft {
namespace server {

struct Handler;

class Method{

public:

   explicit Method (std::string method0) : method(method0) {

   }

   Method& handler (const string& pattern, function<void(const Request&, Response&)> invoke) {
       std::string pattern0 = preparePattern(pattern);
       char const* error;
       int offset;
       auto regex = pcre_compile(pattern0.data(), 0,  &error, &offset, nullptr);
       auto aid = pcre_study(regex, PCRE_STUDY_JIT_COMPILE, &error);
       Handler h{ pattern, invoke, regex, aid, nullptr, nullptr };
       handlers.push_back(h);

       return *this;
   }

   Method& before (function<bool(const Request&, Response&)> invoke) {
       handlers.at(handlers.size() -1).before = invoke;
       return *this;
   }

   Method& after (function<void(const Request&, Response&)> invoke) {
       handlers.at(handlers.size() -1).after = invoke;
       return *this;
   }

   std::string method;
   std::vector<Handler> handlers;

private:


   std::string preparePattern(const string& pattern) {
       std::regex rex("\\{\\w+\\}");
       std::smatch matcher;
       std::string fmt = "\\w+";
       std::string result = std::regex_replace(pattern, rex, fmt);

       return result;
   }

};

class Server
{
public:



    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    explicit Server(const std::string& address, const std::string& port, const std::string& doc_root);

    void run(int n_threads);

    std::shared_ptr<Method> get();
    std::shared_ptr<Method> post();
    std::shared_ptr<Method> put();
    std::shared_ptr<Method> del();
    std::shared_ptr<Method> options();

    inline static std::map<std::string, std::shared_ptr<Method>> methods;
private:
    void doAccept();

    void doAwaitStop();

    boost::asio::io_context ioContext_;

    boost::asio::signal_set signals_;

    boost::asio::ip::tcp::acceptor acceptor_;

    ConnectionManager connectionManager_;

    RequestHandler requestHandler_;

    void addHandler(Handler h, const string& method);
    std::string preparePattern(const string& pattern);
};

}
}

#endif // HTTP_SERVER_HPP
