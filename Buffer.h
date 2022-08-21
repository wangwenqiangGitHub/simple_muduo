#ifndef SIMPLE_MUDUO_BUFFER_H
#define SIMPLE_MUDUO_BUFFER_H
#include <cstddef> // size_t
#include <string>
#include <sys/types.h>
#include <vector>

namespace simple_muduo{
//muduo网络库底层的缓冲区类型定义
// 非线程安全:
//     1 对于input buffer, onMessage()回调始终发生在TcpConnection所属的那个线程，应用程序应该在onMessage()完成对input buffer的操作，并且不需要把input buffer暴漏给其他线程，这样所有对input buffer都是在一个线程中的
//     2 对于output buffer,应用程序不会直接操作它，而是调用TcpConnection::send()来发送数据的，后者线程安全
class Buffer
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

	explicit Buffer(size_t initalSize = kInitialSize)
		: buffer_(kCheapPrepend + initalSize)
		, readerIndex_(kCheapPrepend)
		, writerIndex_(kCheapPrepend)
	{

	}
	size_t readableBytes() const {return writerIndex_ - readerIndex_; }
	size_t writableBytes() const {return buffer_.size() - writerIndex_;}
	size_t prependableBytes() const {return readerIndex_;}

	const char* peek() const { return begin() + readerIndex_;}
	
	void retrieve(size_t len)
	{
		if(len < readableBytes())
		{
			// 说明应用只读取了可读缓冲区数据的一部分，
			// 就是len长度 还剩下readerIndex+=len到writerIndex_的数据未读
			readerIndex_ += len;
		}
		else{
			retrieveAll();
		}
	}

	void retrieveAll()
	{
		readerIndex_ = kCheapPrepend;
		writerIndex_ = kCheapPrepend;
	}

	// 把onMessage函数上报的Buffer数据 转换成string类型的数据上报。
	std::string retrieveAllAsString() {return retrieveAsString(readableBytes());}
	std::string retrieveAsString(size_t len)
	{
		std::string result(peek(), len);
		// 将读出来的buffer去除
		retrieve(len);
		return result;
	}

	// 确保有足够的可写空间
	void ensureWritableBytes(size_t len)
	{
		if(writableBytes() < len)
		{
			makeSPace(len);
		}
	}

	// 在当前buffer 写的索引位置插入数据[data, data+len]
	void append(const char* data, size_t len)
	{
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        writerIndex_ += len;
	}
	char* beginWrite() { return begin() + writerIndex_;}
	const char* beginWrite() const {return begin() + writerIndex_;}

	// 通过fd写数据
	ssize_t readFd(int fd, int* saveErrno);
	// 通过fd发送数据
	ssize_t writeFd(int fd, int* saveErrno);
private:
	// std::vector 底层数组元素地址的起始地址；缓存区buffer的起始地址
	char* begin() {return &*buffer_.begin();}
	const char* begin() const {return &*buffer_.begin(); }

	void makeSPace(size_t len)
	{
		// 将要写入的数据len的长度大于 还能写入的buffer长度(writableBytes()) + 已经读完的buffer的长度 - 预留的长度(kCheapPrepend)
		if(writableBytes() + prependableBytes() < len + kCheapPrepend)
		{
			buffer_.resize(writerIndex_ + len);
		}
		else
		{
			// 将数据拷贝到begin()+kCheapPrepend 起始位置；使得可写的位置一个连续的空间
			size_t readable = readableBytes();
			std::copy(begin() + readerIndex_,
					  begin() + writerIndex_,
					  begin() + kCheapPrepend);
			readerIndex_ = kCheapPrepend;
			writerIndex_ = readerIndex_ + readable;
		}
	}
	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;
};
}
#endif //SIMPLE_MUDUO_BUFFER_H
