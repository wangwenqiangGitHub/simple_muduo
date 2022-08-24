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
	static Timestamp invalid();

    std::string toString() const;
	int64_t microSeconds() const {return m_microSecond;}
	int64_t millSeconds() const {return m_microSecond/1000;}
	int64_t seconds() const {return m_microSecond/(1000*1000);}
	
    bool valid() const
    {
        return m_microSecond > 0;
    }

    /// return seconds
    static double  timeDiff(const Timestamp& end, const Timestamp& start)
    {
         int64_t delta = end.microSeconds() - start.microSeconds();
         return (delta * 1.0) / (1000 * 1000);
    }

    /// return mill seconds
    static int64_t  timeDiffMs(const Timestamp& end, const Timestamp& start)
    {
        int64_t delta = end.microSeconds() - start.microSeconds();
        return (delta / 1000);
    }

    /// return micro seconds
    static int64_t  timeDiffUs(const Timestamp& end, const Timestamp& start)
    {
        return end.microSeconds() - start.microSeconds();
    }
private:
    int64_t m_microSecond;
};

inline Timestamp operator+(const Timestamp& lhs, double seconds)
{
    int64_t delta = seconds * 1000 * 1000;
    return Timestamp(lhs.microSeconds() + delta);
}

inline Timestamp operator+(double seconds, const Timestamp& rhs)
{
    return (rhs + seconds);
}

inline bool operator<(const Timestamp& lhs, const Timestamp& rhs)
{
    return lhs.microSeconds() < rhs.microSeconds();
}

inline Timestamp operator+=(Timestamp& lhs, double seconds)
{
    return lhs = lhs + seconds;
}

inline Timestamp operator-(const Timestamp& lhs, double seconds)
{ 
    return (lhs + (-seconds));
}

inline Timestamp operator-=(Timestamp& lhs, double seconds)
{ 
    return (lhs = (lhs + (-seconds)));
}
} //namespace simple_muduo
#endif // SIMPLE_MUDUO_TIMESTAMP_H
