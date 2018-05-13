#pragma once

#include <exa/dependencies.hpp>

namespace exa
{
    enum class socket_type
    {
        unspecified = 0,
        seq_packet = SOCK_SEQPACKET,
        stream = SOCK_STREAM,
        datagram = SOCK_DGRAM,
        rdm = SOCK_RDM,
        raw = SOCK_RAW
    };

    enum class protocol_type
    {
        unspecified = 0,
        ip = IPPROTO_IP,
        ip_v4 = IPPROTO_IPV4,
        ip_v6 = IPPROTO_IPV6,
        tcp = IPPROTO_TCP,
        udp = IPPROTO_UDP,
        icmp = IPPROTO_ICMP,
        igmp = IPPROTO_IGMP,
        egp = IPPROTO_EGP,
        pup = IPPROTO_PUP,
        idp = IPPROTO_IDP,
        routing = IPPROTO_ROUTING,
        fragment = IPPROTO_FRAGMENT,
        esp = IPPROTO_ESP,
        ah = IPPROTO_AH,
        icmp_v6 = IPPROTO_ICMPV6,
        none = IPPROTO_NONE,
        pim = IPPROTO_PIM,
        sctp = IPPROTO_SCTP,
        raw = IPPROTO_RAW,
        ip_v6_hop_by_hop_options = IPPROTO_HOPOPTS,
        ip_v6_destination_options = IPPROTO_DSTOPTS
    };

    enum class address_family
    {
        unspecified = AF_UNSPEC,
        inter_network = AF_INET,
        inter_network_v6 = AF_INET6,
        apple_talk = AF_APPLETALK,
        unix = AF_UNIX,
        ipx = AF_IPX
    };

    enum class select_mode
    {
        error,
        read,
        write
    };

    enum class socket_shutdown
    {
        read = 1,
        write = 2,
        both = read | write
    };

    enum class socket_flags
    {
        none = 0,
        peek = MSG_PEEK,
        out_of_band = MSG_OOB,
        wait_all = MSG_WAITALL,
        dont_route = MSG_DONTROUTE
    };
}
