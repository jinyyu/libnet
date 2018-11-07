#pragma once

#include <evcpp/callbacks.h>
#include <evcpp/InetSocketAddress.h>

namespace ev
{

class EventLoop;
class InetSocketAddress;
class EventSource;

class Session
{
public:
    explicit Session(EventLoop* loop, const InetSocketAddress& local);

    ~Session();

    void connect_error_callback(const ConnectErrorCallback& cb)
    {
        connect_error_callback_ = cb;
    }

    void connection_established_callback(const EstablishedCallback& cb)
    {
        connection_established_callback_ = cb;
    }

    void read_message_callback(const ReadCallback& cb)
    {
        read_message_callback_ = cb;
    }

    void connection_closed_callback(const ClosedCallback& cb)
    {
        connection_closed_callback_ = cb;
    }

    void connect(const InetSocketAddress& peer);

private:
    void handle_connected(uint64_t timestamp);

    bool do_connect(const InetSocketAddress& addr);

    void handle_error(uint64_t timestamp);

private:
    int fd_;
    EventLoop* loop_;
    InetSocketAddress local_;
    InetSocketAddress peer_;
    EventSource* event_source_;

    ConnectErrorCallback connect_error_callback_;
    EstablishedCallback connection_established_callback_;
    ReadCallback read_message_callback_;
    ClosedCallback connection_closed_callback_;
};

}

