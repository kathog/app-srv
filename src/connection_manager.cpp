#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#include "../include/connection_manager.hpp"

namespace craftsoft {
namespace server {

ConnectionManager::ConnectionManager()
{
}

void ConnectionManager::start(connection_ptr c)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        connections_.insert(c);
    }
    c->start();
}

void ConnectionManager::stop(connection_ptr c)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        connections_.erase(c);
    }
    c->stop();
}

void ConnectionManager::stop_all()
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto c : connections_)
        c->stop();
    connections_.clear();
}

} // namespace server
} // namespace http
