#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#include "../include/connection.hpp"
#include <utility>
#include <vector>
#include "../include/connection_manager.hpp"
#include "../include/request_handler.hpp"
#include <future>
#include <iostream>
#include <string_view>

namespace craftsoft {
namespace server {

const char spacja = ' ';
const char dwokropek = ':';
const char srednik = ';';
const char rownasie = '=';
const std::string cookieName = "Cookie";

Connection::Connection( boost::asio::io_service & io_service, boost::asio::ip::tcp::socket socket,	ConnectionManager& manager, RequestHandler& handler)
    : ioService_(io_service),
      socket_(std::move(socket)),
      connection_manager_(manager),
      request_handler_(handler) {}

void Connection::start() {
    do_read();
}

void Connection::stop()
{
    socket_.close();
}

void Connection::do_read()
{
    request_ = Request();
    auto self(shared_from_this());

    socket_.async_read_some(boost::asio::buffer(buffer_), [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
            //            auto time = std::chrono::high_resolution_clock::now();
            RequestParser::result_type result;
            int bodySize = 0;
            try {
                std::istringstream f(buffer_.data());
                std::string line;
                line.reserve(500);
                std::string headerName;
                headerName.reserve(100);
                std::string cookies;
                cookies.reserve(500);
                std::string cookieValue;
                cookieValue.reserve(400);

                bool body = false;
                int count = 0;
                int lSize = 0;
                while (std::getline(f, line)) {
                    if (body) {
                        request_.body += line;
                        request_.body += "\n";
                    } else {
                        const char * data = line.data();
                        lSize = line.size();
                        // wyszukiwanie metody i uri
                        if (count == 0) {
                            int oldValue = 0;
                            request_.http_version_major = atoi(&data[lSize - 4]);
                            request_.http_version_minor = atoi(&data[lSize - 2]);
                            for (int i = 0; i < lSize; i++) {
                                const char c = data[i];
                                if (c == spacja) {
                                    if (oldValue == 0) {
                                        request_.method = line.substr(0, i);
                                        oldValue = i;
                                    } else {
                                        request_.uri = line.substr(oldValue+1, i - (oldValue+1));
                                        break;
                                    }
                                }
                            }
                            count ++;
                        } else {
                            for (int i = 0; i < line.size(); i++) {
                                const char c = data[i];
                                if (c == dwokropek && data[i+1] == spacja) {
                                    headerName = line.substr(0, i);
                                    if (headerName == cookieName) {
                                        int oldValue = 0;
                                        cookies = line.substr(i + 2);
                                        const char * data = cookies.data();
                                        lSize = cookies.size();
                                        for (int i = 0; i < lSize; i++) {
                                            const char c = data[i];
                                            if (c == srednik) {
                                                cookieValue = cookies.substr(oldValue, i);
                                                int idx = cookieValue.find(rownasie);
                                                if (idx > 0) {
                                                    request_.cookies.emplace_back(header{cookieValue.substr(0, idx), cookieValue.substr(idx + 1)});
                                                }
                                                oldValue = i +2;
                                            }
                                        }
                                        cookieValue = cookies.substr(oldValue, lSize-1);
                                        const int idx = cookieValue.find(rownasie);
                                        if (idx > 0) {
                                            const int idxK = cookieValue.find("\r");
                                            if (idxK > 0) {
                                                request_.cookies.emplace_back(header{cookieValue.substr(0, idx), cookieValue.substr(idx + 1, idx - idxK)});
                                            } else {
                                                request_.cookies.emplace_back(header{cookieValue.substr(0, idx), cookieValue.substr(idx + 1)});
                                            }
                                        }
                                    } else {
                                        if (headerName == "Content-Length" || headerName == "content-length") {
                                            bodySize = atoi(line.substr(i + 2).data());
                                        }
                                        request_.headers.emplace_back(header{headerName, line.substr(i + 2)});
                                    }
                                }
                            }
                        }
                        if (lSize == 1) {
                            body = true;
                        }
                    }

                }
                result = RequestParser::good;
            } catch (exception &e) {
                result = RequestParser::bad;
            }
            //                        std::tie(result, std::ignore) = request_parser_.parse(request_, buffer_.data(), buffer_.data() + bytes_transferred);
            if (result == RequestParser::good) {
                //                std::string_view data_{buffer_.data()};
                //                auto idx = data_.find("\r\n\r\n");
                //                if (idx > 0) {
                //                    request_.body = data_.substr(idx + 4, data_.size());
                //                } else {
                //                    idx = data_.find("\n\n");
                //                    if (idx > 0) {
                //                        request_.body = data_.substr(idx + 4, data_.size());
                //                    }
                //                }
                //                auto end = std::chrono::high_resolution_clock::now();
                //                auto tResult = std::chrono::duration_cast<std::chrono::duration<double>>(end - time);
                //                std::cout << tResult.count() * 1000 * 1000 << " us" << std::endl;
                if (bodySize > 0) {
                    request_.body = request_.body.substr(0, bodySize);
                }
                request_handler_.handleRequest(request_, reply_);
                do_write();
            } else if (result == RequestParser::bad) {
                reply_ = Response::stock_reply(Response::bad_request);
                do_write();
            } else {
                do_read();
            }
        }
        else if (ec != boost::asio::error::operation_aborted) {
            connection_manager_.stop(shared_from_this());
        }
    });
}

void Connection::do_write() {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, reply_.to_buffers(), [this, self](boost::system::error_code ec, std::size_t) {
        if (!ec) {
            boost::system::error_code ignored_ec;
            socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
        }

        if (ec != boost::asio::error::operation_aborted) {
            connection_manager_.stop(shared_from_this());
        }
    });
}

} // namespace server
} // namespace http
