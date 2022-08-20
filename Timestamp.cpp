#include <cstdint>
#include <ctime>
#include <sys/time.h> //gettimeofday
#include "Timestamp.h"
using namespace simple_muduo;
Timestamp::Timestamp() : m_microSecond(0)
{
}

Timestamp::Timestamp(int64_t microsecond)
    : m_microSecond(microsecond)
{
}

Timestamp Timestamp::now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int64_t ret;
	ret = ((int64_t)tv.tv_sec) * 1000 * 1000 + tv.tv_usec;
	return Timestamp(ret);
}
std::string Timestamp::toString() const
{
	struct tm tm_time;
	time_t seconds = static_cast<time_t>(m_microSecond/ (1000 * 1000));
	int microseconds = m_microSecond% (1000 * 1000);
    char buf[128] = {0};
	localtime_r(&seconds, &tm_time);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour,
             tm_time.tm_min,
             tm_time.tm_sec);
    return buf;
}

// #include <iostream>
// #include <inttypes.h> // PRId64 宏
// int main() {
//     std::cout << Timestamp::now().toString() << std::endl;
// 	std::cout << Timestamp::now().millSeconds() << std::endl;
// 	printf("%" PRId64 "\n",Timestamp::now().millSeconds() );
//     return 0;
// }
