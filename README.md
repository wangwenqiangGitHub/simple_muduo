# timestamp
- 值语义:可以拷贝，拷贝之后，与原对象脱离关系
- 对象语音: 要么是不能拷贝，要么是可以拷贝，拷贝之后与原对象仍然存在一定的关系，比如共享底层资源(要实现自己的拷贝构造函数)
- muduo::copyable空基类，标识类，值类型
- 时间的起点是1970年-01-01 00:00:00
