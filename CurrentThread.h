#ifndef SIMPLE_MUDUO_CURRENTTHREAD_H
#define SIMPLE_MUDUO_CURRENTTHREAD_H
#include <unistd.h>
#include <sys/syscall.h>
namespace simple_muduo{ namespace CurrentThread{
	// 保存tid缓存，因为系统调用非常耗时，因此拿到tid后将其保存
	extern __thread int t_cachedTid;
	void cacheTid();

	inline int tid()
	{
		if(__builtin_expect(t_cachedTid == 0, 0))
		{
			cacheTid();
		}
		return t_cachedTid;
	}
}
}
#endif //  SIMPLE_MUDUO_CURRENTTHREAD_H
