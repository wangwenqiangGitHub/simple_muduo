# 目标
original author github: https://github.com/chenshuo/muduo
- 基于muduo cpp11版本，简化代码。
- 适用于arm x86 linux平台的网络库。
- 编译工具链>= gcc4.9 或者 >=arm-linux-gnueabi-gcc4.9

# 时间戳
- 采用系统时间
# 日志
- TODO:采用spdlog

# utl
- TODO:fileUtil

# IO多路复用(example/IO)
- select
	- 1. bitmap fd默认大小最大为1024
	- 2.fd\_set每次都会被修改，不可重用，每次都需要清零，设置fd\_set;
	- 3. fd\_set每次都会有用户态到内核态的开销。
	- 4. 查询所有的fd，复杂度为o(n)。
- poll
	- 解决了select中的1，2这两个个问题
- epool
	- 解决了1.2.4问题，对3做了优化吧

