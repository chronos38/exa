#include <exa/tcp_listener.hpp>
#include <exa/task.hpp>
#include <exa/detail/io_task.hpp>

#include <chrono>

using namespace std::chrono_literals;

namespace exa
{
    tcp_listener::tcp_listener(uint16_t port) : tcp_listener(endpoint(address::any, port))
    {
    }

    tcp_listener::tcp_listener(const address& addr, uint16_t port) : tcp_listener(endpoint(addr, port))
    {
    }

    tcp_listener::tcp_listener(const endpoint& ep) : endpoint_(ep)
    {
        socket_ = std::make_shared<exa::socket>(ep.family(), socket_type::stream, protocol_type::tcp);
    }

    tcp_listener::~tcp_listener()
    {
        stop();
    }

    bool tcp_listener::active() const
    {
        return active_;
    }

    bool tcp_listener::exclusive_address_use() const
    {
        return socket_->exclusive_address_use();
    }

    void tcp_listener::exclusive_address_use(bool value)
    {
        if (active_)
        {
            throw std::runtime_error("Can't change address exclusivity while listening.");
        }

        socket_->exclusive_address_use(value);
    }

    bool tcp_listener::reuse_address() const
    {
        return socket_->reuse_address();
    }

    void tcp_listener::reuse_address(bool value)
    {
        if (active_)
        {
            throw std::runtime_error("Can't change address reusing while listening.");
        }

        socket_->reuse_address(value);
    }

    endpoint tcp_listener::local_endpoint() const
    {
        if (socket_->bound())
        {
            return socket_->local_endpoint();
        }
        else
        {
            return endpoint_;
        }
    }

    const std::shared_ptr<socket>& tcp_listener::socket() const
    {
        return socket_;
    }

    std::shared_ptr<exa::socket> tcp_listener::accept_socket() const
    {
        if (!active_)
        {
            throw std::runtime_error("TCP listener isn't actively listening.");
        }

        return socket_->accept();
    }

    std::future<std::shared_ptr<exa::socket>> tcp_listener::accept_socket_async() const
    {
        if (!active_)
        {
            throw std::runtime_error("TCP listener isn't actively listening.");
        }

        return socket_->accept_async();
    }

    std::shared_ptr<tcp_client> tcp_listener::accept_client() const
    {
        if (!active_)
        {
            throw std::runtime_error("TCP listener isn't actively listening.");
        }

        return std::make_shared<tcp_client>(socket_->accept());
    }

    std::future<std::shared_ptr<tcp_client>> tcp_listener::accept_client_async() const
    {
        if (!active_)
        {
            throw std::runtime_error("TCP listener isn't actively listening.");
        }

        return detail::io_task::run<std::shared_ptr<tcp_client>>([=] {
            return socket_->poll(1s, select_mode::read)
                       ? std::make_tuple(true, std::make_shared<tcp_client>(socket_->accept()))
                       : std::make_tuple(false, std::shared_ptr<tcp_client>());
        });
    }

    bool tcp_listener::pending() const
    {
        if (!active_)
        {
            throw std::runtime_error("TCP listener isn't actively listening.");
        }

        return socket_->poll(0us, select_mode::read);
    }

    void tcp_listener::start(size_t backlog)
    {
        if (backlog == 0 || backlog > max_connections)
        {
            throw std::out_of_range("Backlog value needs to within specified range of (0, 0x7fffffff].");
        }
        if (active_)
        {
            return;
        }

        socket_->bind(endpoint_);

        try
        {
            socket_->listen(backlog);
        }
        catch (...)
        {
            stop();
            std::rethrow_exception(std::current_exception());
        }

        active_ = true;
    }

    void tcp_listener::stop()
    {
        socket_->close();
        socket_ = std::make_shared<exa::socket>(endpoint_.family(), socket_type::stream, protocol_type::tcp);
        active_ = false;
    }
    std::shared_ptr<tcp_listener> tcp_listener::create(uint16_t port)
    {
        if (socket::ipv6_supported())
        {
            auto l = std::make_shared<tcp_listener>(address::ipv6_any, port);
            l->socket()->dual_mode(true);
            return l;
        }
        else
        {
            return std::make_shared<tcp_listener>(address::any, port);
        }
    }
}
