#pragma once

#ifdef _WIN32
#pragma warning(push, 0)

#ifndef _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")

namespace exa
{
    namespace detail
    {
        template <int Major = 2, int Minor = 2>
        class winsock_init
        {
        public:
            winsock_init()
            {
                if (::InterlockedIncrement(&init_count_) == 1)
                {
                    WSADATA wsa_data = {0};
                    auto result = ::WSAStartup(MAKEWORD(Major, Minor), &wsa_data);
                    ::InterlockedExchange(&result_, result);
                }
            }

            ~winsock_init()
            {
                if (::InterlockedDecrement(&init_count_) == 0)
                {
                    ::WSACleanup();
                }
            }

        private:
            static LONG init_count_;
            static LONG result_;
        };

        template <int Major, int Minor>
        LONG winsock_init<Major, Minor>::init_count_ = 0;
        template <int Major, int Minor>
        LONG winsock_init<Major, Minor>::result_ = 0;

        static const winsock_init<> winsock_init_instance;
    }
}

#else

#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#endif

#include <gsl/gsl_algorithm>
#include <gsl/gsl_assert>
#include <gsl/gsl_byte>
#include <gsl/gsl_util>
#include <gsl/multi_span>
#include <gsl/pointers>
#include <gsl/span>
#include <gsl/string_span>

#ifdef _WIN32
#pragma warning(pop)
#endif
