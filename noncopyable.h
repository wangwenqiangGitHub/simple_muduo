#ifndef SIMPLE_MUDUO_NONCOPYABLE_H
#define SIMPLE_MUDUO_NONCOPYABLE_H

namespace simple_muduo{
class noncopyable
{
public:
	noncopyable(const noncopyable&) = delete;
	void operator=(const noncopyable&) = delete;
protected:
	noncopyable() = default;
	~noncopyable() = default;
};
}

#endif // SIMPLE_MUDUO_NONCOPYABLE_H

