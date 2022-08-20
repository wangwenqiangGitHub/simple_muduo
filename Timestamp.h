#ifndef SIMPLE_MUDUO_TIMESTAMP_H
#define SIMPLE_MUDUO_TIMESTAMP_H
#include <cstdint>
#include <iostream>
#include <string>
namespace simple_muduo{

class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecond);
    static Timestamp now();
    std::string toString() const;
	int64_t microSeconds() const {return m_microSecond;}
	int64_t millSeconds() const {return m_microSecond/1000;}
	int64_t seconds() const {return m_microSecond/(1000*1000);}
private:
    int64_t m_microSecond;
};

} //namespace simple_muduo
#endif // SIMPLE_MUDUO_TIMESTAMP_H
