#include "Poller.h"
#include "Channel.h"

using namespace simple_muduo;

Poller::Poller(EventLoop* loop)
	: ownerLoop_(loop)
{

}

bool Poller::hasChannel(Channel *channel) const
{
	auto it = channels_.find(channel->fd());
	return it != channels_.end() && it->second == channel;
}
