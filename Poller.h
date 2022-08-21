#ifndef SIMPLE_MUDUO_POLLER_H
#define SIMPLE_MUDUO_POLLER_H
#include <vector>
#include <unordered_map>

#include "Timestamp.h"


namespace simple_muduo{
class Channel;
class EventLoop;

class Poller{
public:
	using ChannelList = std::vector<Channel*>;
	Poller(EventLoop* loop);
	virtual ~Poller() = default;

	// 子类需要实现这几个纯虚函数  IO复用保存统一的借口
	virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
	virtual void updateChannel(Channel* channel)=0;
	virtual void removeChannel(Channel* channel)=0;

	//判断参数channel是否在当前线程的Poller中
	bool hasChannel(Channel* channel) const;

	// EventLoop可以通过该接口获取默认的IO复用的具体实现
	static Poller* newDefaultPoller(EventLoop* loop);

protected:
	// fd与fd所属的channel通道构成了这个map
	using ChannelMap = std::unordered_map<int, Channel*>;
	ChannelMap channels_;
private:
    EventLoop *ownerLoop_; // 定义Poller所属的事件循环EventLoop
};
}
#endif //SIMPLE_MUDUO_POLLER_H
