#include <boost/static_assert.hpp>
#include <boost/static_string.hpp>
#include <cstdint>
#include <cstdio>

class Timestamp
{
private:
	int64_t microSecondsSinceEpoch_;
};
BOOST_STATIC_ASSERT(sizeof(Timestamp) == sizeof(int64_t));
int main(void)
{
	return 0;
}
