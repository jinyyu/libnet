#include <EventLoop.h>
#include <unistd.h>
#include "Session.h"
#include "Timestamp.h"
#include "Connection.h"
#include "Selector.h"
#include "Channel.h"
#include "Logger.h"
#include "SelectionKey.h"

namespace net
{

Session::Session(EventLoop* loop, const InetSocketAddress& local)
    : loop_(loop),
      local_(local),
      channel_(nullptr)
{
  fd_ = ::socket(local.family(), SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (fd_ < 0) {
    LOG_ERROR("socket error %d", errno);
  }
}


Session::~Session()
{
  if (channel_) {
    delete channel_;
  }
}

void Session::connect(const InetSocketAddress& peer)
{
  peer_ = peer;
  channel_ = new Channel(loop_->selector_, fd_);
  SelectionCallback write_cb = [this](const Timestamp & timestamp, SelectionKey* key) {
    this->handle_connected(timestamp, key);
  };
  channel_->set_writing_selection_callback(write_cb);


  if (!do_connect(peer)) {
    //handle connect error
    handle_error(Timestamp::currentTime());
    return;
  }
  else {
    channel_->enable_writing();
  }
}

void Session::handle_connected(const Timestamp& timestamp, SelectionKey* key)
{
  channel_->disable_all();

  if (key->is_error()) {
    LOG_INFO("error");
    handle_error(timestamp);
    return;
  }

  int err;
  socklen_t len = sizeof(err);
  getsockopt(fd_, SOL_SOCKET, SO_ERROR, &err, &len);

  if (err != 0) {
    //error
    LOG_ERROR("connect error %d", err);
    handle_error(timestamp);
    return;

  }

  //LOG_INFO("connect success")
  //success
  ConnectionPtr conn(new Connection(fd_, loop_, local_, peer_));

  conn->connection_established_callback(connection_established_callback_);
  conn->read_message_callback(read_message_callback_);
  conn->connection_closed_callback(connection_closed_callback_);

  loop_->on_new_connection(conn, timestamp);
}

bool Session::do_connect(const InetSocketAddress& addr)
{
  int len = addr.family() == AF_INET ? sizeof(addr.sockaddr_) : sizeof(addr.sockaddr6_);
  int ret = ::connect(fd_, addr.sockaddr_cast(), len);
  int err = errno;

  if (ret < 0 && err != EINPROGRESS) {
    LOG_ERROR("connect error %d", errno);
    return false;
  }
  return true;

}

void Session::handle_error(const Timestamp& timestamp)
{
  LOG_ERROR("error happened %s", timestamp.to_string().c_str());
  if (connect_error_callback_) {
    connect_error_callback_(timestamp);
  }
  ::close(fd_);
}



}