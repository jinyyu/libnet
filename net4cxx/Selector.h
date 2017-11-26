#ifndef NET4CXX_DISTRIBUTION_SELECTOR_H
#define NET4CXX_DISTRIBUTION_SELECTOR_H
#include <net4cxx/NonCopyable.h>
#include <net4cxx/Timestamp.h>
#include <vector>
#include <sys/epoll.h>
#include <pthread.h>

namespace net4cxx
{

class SelectionKey;

class Selector: NonCopyable
{
public:
    explicit Selector(pthread_t pthread_id);

    ~Selector();

    void add(SelectionKey* selection_key);

    void modify(SelectionKey* selection_key);

    void remove(SelectionKey* selection_key);

    Timestamp select(int timeout_milliseconds, std::vector<SelectionKey*>& active_key);


private:
    void control(int op, SelectionKey* selection_key);

    pthread_t pthread_id_;
    int epoll_fd_;
    std::vector<struct epoll_event> events_;
    int selecting_events_;

};

}

#endif //LIBNET_DISTRIBUTION_SELECTOR_H
