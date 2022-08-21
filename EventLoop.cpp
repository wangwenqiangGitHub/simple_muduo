#include <cerrno>
#include <mutex>
#include <sys/eventfd.h>
#include <unistd.h>

#include "CurrentThread.h"
#include "Logger.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"

using namespace simple_muduo;

// 防止一个线程创建多个EventLoop
__thread EventLoop* t_loopInThisThread = nullptr;

// 定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

// 创建线程之后主线程和子线程谁先运行是不确定的。
// 通过一个eventfd在线程之间传递数据的好处是多个线程无需上锁就可以实现同步

// 函数原型：
//     #include <sys/eventfd.h>
//     int eventfd(unsigned int initval, int flags);
// 参数说明：
//      initval,初始化计数器的值。
//      flags, EFD_NONBLOCK,设置socket为非阻塞。
//             EFD_CLOEXEC，执行fork的时候，在父进程中的描述符会自动关闭，子进程中的描述符保留。
// 场景：
//     eventfd可以用于同一个进程之中的线程之间的通信。
//     eventfd还可以用于同亲缘关系的进程之间的通信。
//     eventfd用于不同亲缘关系的进程之间通信的话需要把eventfd放在几个进程共享的共享内存中（没有测试过）。


// 创建wakeupfd 用来notify唤醒subReactor处理新来的channel
int createEventfd()
{
	int efd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if(efd < 0)
	{
		LOG_ERROR("efd error:%d\n", errno);
	}
	return efd;
}

EventLoop::EventLoop()
	:looping_(false)
	 , quit_(false)
	 , poller_(Poller::newDefaultPoller(this))
	 , threadId_(CurrentThread::tid())
	 , wakeupFd_(createEventfd())
	 , wakeupChannel_(new Channel(this, wakeupFd_))
	 , callingPendingFunctors_(false)
{
	if(t_loopInThisThread)
    {
        LOG_INFO("Another EventLoop %p exists in this thread %d\n", t_loopInThisThread, threadId_);
    }
	else
    {
        t_loopInThisThread = this;
    }
	wakeupChannel_->setReadCallback(
			std::bind(&EventLoop::handleRead, this));
}

EventLoop::~EventLoop()
{
	// 给Channel移除所有感兴趣的事件
	wakeupChannel_->disableAll(); 
	//把Channel从EventLoop上删除掉
	wakeupChannel_->remove();
	::close(wakeupFd_);
	t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
	looping_ = true;
	quit_ = false;
	while(!quit_)
	{
		activeChannels_.clear();
		pollRetureTime_ = poller_->poll(kPollTimeMs, & activeChannels_);
		for(Channel* channel : activeChannels_)
		{
			// Poller监听哪些channel发生了事件 然后上报给EventLoop
			// 通知channel处理相应的事件
			channel->handleEvent(pollRetureTime_);
         // 执行当前EventLoop事件循环需要处理的回调操作 对于线程数 >=2 的情况 IO线程 mainloop(mainReactor) 主要工作：
         // ccept接收连接 => 将accept返回的connfd打包为Channel => TcpServer::newConnection通过轮询将TcpConnection对象分配给subloop处理
         //  mainloop调用queueInLoop将回调加入subloop（该回调需要subloop执行 但subloop还在poller_->poll处阻塞） 
		 //  queueInLoop通过wakeup将subloop唤醒
        doPendingFunctors();
		}
	}
	LOG_INFO("EventLoop stop loop\n");
	looping_ = false;
}

// 退出事件loop : 1. 退出自己线程; 2.不是自己EventLoop线程，那么调用wakeup()唤醒其他EventLoop所属线程的epoll_wait
// 比如在一个subloop(worker)中调用mainloop(IO)的quit时 需要唤醒mainloop(IO)的poller_->poll 让其执行完loop()函数
// ！！！ 注意： 正常情况下 mainloop负责请求连接 将回调写入subloop中 通过生产者消费者模型即可实现线程安全的队列
// ！！！       但是muduo通过wakeup()机制 使用eventfd创建的wakeupFd_ notify 使得mainloop和subloop之间能够进行通信
void EventLoop::quit()
{
    quit_ = true;

    if (!isInLoopThread())
    {
        wakeup();
    }
}
// 将回调函数在本loop中执行
void EventLoop::runInLoop(Functor cb)
{
	if(isInLoopThread())
	{
		cb();
	}
	else{
		queueInLoop(cb);
	}
}

// 将回调函数放入队列 在所在loop 线程中执行回调
void EventLoop::queueInLoop(Functor cb)
{
	// 减小锁的临界区
	{
		std::unique_lock<std::mutex> lock(mutex_);
		pendingFunctors_.emplace_back(cb);
	}
	// callingPendingFunctors_的意思是 当前loop正在执行回调中 但是loop的pendingFunctors_中又加入了新的回调 需要通过wakeup写事件
    // * 唤醒相应的需要执行上面回调操作的loop的线程 让loop()下一次poller_->poll()不再阻塞（阻塞的话会延迟前一次新加入的回调的执行），然后
    // * 继续执行pendingFunctors_中的回调函数
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup(); // 唤醒loop所在线程
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8\n", n);
    }
}

// 用来唤醒loop所在线程 向wakeupFd_写一个数据 wakeupChannel就发生读事件 当前loop线程就会被唤醒
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8\n", n);
    }
}

// EventLoop的方法 => Poller的方法
void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
		// 交换的方式减少了锁的临界区范围 提升效率 同时避免了死锁 
		// 如果执行functor()在临界区内 且functor()中调用queueInLoop()就会产生死锁
        functors.swap(pendingFunctors_);
    }

    for (const Functor &functor : functors)
    {
        functor(); // 执行当前loop需要执行的回调操作
    }

    callingPendingFunctors_ = false;
}
