#include <cerrno>
#include <sys/epoll.h>
#include <unistd.h> //close
#include <string.h> //memset

#include "EPollPoller.h"
#include "Poller.h"
#include "Logger.h"
#include "Timestamp.h"
#include "Channel.h"

using namespace simple_muduo;
const int kNew = -1;    // 某个channel还没添加至Poller          // channel的成员index_初始化为-1
const int kAdded = 1;   // 某个channel已经添加至Poller
const int kDeleted = 2; // 某个channel已经从Poller删除

EPollPoller::EPollPoller(EventLoop* loop)
	: Poller(loop)
	, epollfd_(::epoll_create(EPOLL_CLOEXEC))
	, events_(kInitEventListSize)
{
	if(epollfd_ < 0)
	{
		LOG_ERROR("EPollPoller construct error:%d \n", errno);
	}
}

EPollPoller::~EPollPoller()
{
	::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
	int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()),timeoutMs);
	int savedErrno = errno;
	Timestamp now(Timestamp::now());
	if(numEvents > 0)
	{
		fillActiveChannels(numEvents, activeChannels);
		if(numEvents == events_.size())
		{
			events_.resize(events_.size() * 2);
		}
	}
	else if(numEvents == 0)
	{
		LOG_INFO("nothing happend\n");
	}
	else
	{
		// error happens, log uncommon ones
		if (savedErrno != EINTR)
		{
			errno = savedErrno;
			LOG_ERROR("EPollPoller::poll()");
		}
	}
	return now;
}

void EPollPoller::updateChannel(Channel *channel)
{
	const int index = channel->index();
    LOG_INFO("func=%s => fd=%d events=%d index=%d\n", __FUNCTION__, channel->fd(), channel->events(), index);

    if (index == kNew || index == kDeleted)
    {
		int fd = channel->fd();
        if (index == kNew)
        {
            channels_[fd] = channel;
        }
        else // index == kAdd
        {
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else // channel已经在Poller中注册过了
    {
        int fd = channel->fd();
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

// 从Poller中删除channel
void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);

    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);

    int index = channel->index();
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

// 填写活跃的连接
void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels)
{
    for (int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel); // EventLoop就拿到了它的Poller给它返回的所有发生事件的channel列表了
    }
}

// 更新channel通道 其实就是调用epoll_ctl add/mod/del
void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    ::memset(&event, 0, sizeof(event));

    int fd = channel->fd();

    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_ERROR("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}
