#include "EventLoop.h"
#include <cstdio>
#include <memory>

void hello()
{
	printf("hello work\n");
}

int main()
{
	simple_muduo::EventLoop loop;
	loop.addTimer(std::bind(hello), 1,true);
	loop.loop();

	return 0;
}
