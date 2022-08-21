#ifndef SIMPLE_MUDUO_EPOOLPOLLER_H
#define SIMPLE_MUDUO_EPOOLPOLLER_H

#include <vector>
#include <sys/epoll.h> // epoll_event

#include "Poller.h"
#include "Timestamp.h"

namespace simple_muduo{

class Channel;
class EPollPoller: public Poller
{
public:
	EPollPoller(EventLoop* loop);
	~EPollPoller();

	// Poller接口的重写
	Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
	void updateChannel(Channel* channel) override;
	void removeChannel(Channel* channel) override;

private:
	static const int kInitEventListSize = 16;

	void fillActiveChannels(int numEvents, ChannelList* activeChannels);
	// epoll_ctl 更新channel通道
	void update(int operation, Channel* channel);

private:
	using EventList = std::vector<epoll_event>;
	// epoll_create创建返回的fd保存在epollfd_中
	int epollfd_;
	// 用于存放epoll_wait返回的所有发生的事件的文件描述符事件集
	EventList events_;
};
}
#endif //SIMPLE_MUDUO_EPOOLPOLLER_H
