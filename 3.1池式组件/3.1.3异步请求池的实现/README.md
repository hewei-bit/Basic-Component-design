# Basic-Component-design
后端开发基础组件设计——异步请求池
通过开启一个线程使用epoll_wait接收返回的dns消息


# 异步请求池实现
## 实现异步请求的四个步骤与伪代码

1. 初始化，创建epoll和线程池，通过一个独立的线程调用epoll_wait()
```bash
1. Init 实现异步
	a. epoll_create
	b. pthread_create
```
2. 主函数通过socket连接服务器，并提交请求，将fd加入epoll中
```bash
2. commit 
	a. socket 
	b. connnect_server
	c. encode-->redis/mysql/dns
	d. send
	e. fd加入到epoll中-->epoll_ctl
```
3. callback编写回调函数
```bash
3. callback
	while(1)
		epoll_wait()
		recv()
		parse()
		fd-->epoll_ delete
```
4. 销毁资源
```bash
4. destroy	
	close(epfd);
	pthread_cancel(thid)


