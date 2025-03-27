# co 协程服务器框架

> 基于C++的高性能协程服务器框架，支持异步抢占调度和网络通信

## 功能特性

- **异步抢占调度** - 支持协程间的异步抢占式调度，通过`AsyncSafeGuard`标记安全点
- **轻量级协程** - 提供高效的协程创建和切换机制，支持`co`关键字创建协程
- **网络通信** - 内置协程友好的`CoSocket`封装，简化网络编程
- **协程控制** - 提供`Yield`、`Sleep`等协程控制原语
- **定时器支持** - 通过`ColangServer::AddTimer`添加协程定时器

## 示例说明

### 1. 异步抢占示例 (async_schedule.cpp)

演示框架的异步抢占能力，两个协程会交替执行：

```cpp
co([]() {
  // 协程1执行密集计算
  for(int i=3000000; i<6000000; i++) {
    // 计算密集型任务
    if(i % 1000 == 0) {
      AsyncSafeGuard guard;  // 声明安全点，允许调度器抢占
      std::cout << i << " co1" << std::endl;
    }
  }
})();

// 主协程同时执行类似任务
for(int i=0; i<3000000; i++) {
  // ...
  if(i % 1000 == 0) {
    AsyncSafeGuard guard;  // 安全点
    std::cout << i << " co2" << std::endl;
  }
}
```

### 2. 基础功能示例 (base_use.cpp)

展示协程基本功能：

```cpp
// 创建协程
co([]() {
  for(int i=3; i<6; i++) {
    std::cout << i << " world" << std::endl;
    ThisCoro::Yield();  // 主动让出执行权
  }
})();

// 定时器示例
ColangServer::GetInst().AddTimer(
  []() { std::cout << "定时器触发" << std::endl; }, 
  20  // 20毫秒后触发
);

// 协程睡眠
ThisCoro::Sleep(5000);  // 睡眠5秒
```

### 3. 网络功能示例 (echo_server.cpp)

实现协程化Echo服务器：

```cpp
class EchoServer {
public:
  int run() {
    // 初始化并监听端口
    acceptor_.Bind("127.0.0.1", 40321);
    acceptor_.Listen(128);
    
    while(true) {
      CoSocket client;
      if(acceptor_.Accept(client) == 0) {
        // 为每个客户端创建独立协程
        co([sock = std::move(client)]() mutable {
          std::string data;
          size_t len = 0;
          sock.Recv(data, len);  // 协程化接收
          sock.Send(data, len);  // 协程化发送
        })();
      }
    }
  }
};

// 客户端测试代码
co([]() { EchoServer().run(); })();  // 启动服务端

for(int i=0; i<10; i++) {
  CoSocket client;
  client.Connect("127.0.0.1", 40321);
  client.Send("hello", len);
  client.Recv(echo, len);  // 接收回显
}
```

## 构建与运行

### 环境要求
- C++17 或更高版本
- Linux/Unix 环境
- CMake 3.10+

### 编译命令
```bash
mkdir build && cd build
cmake ..
make
```

### 运行示例
```bash
# 异步抢占示例
./example/async_schedule

# 基础功能示例  
./example/base_use

# 网络功能示例
./example/echo_server
```

## 贡献指南

欢迎通过Issue或PR参与贡献，请确保：
1. 代码风格与现有代码保持一致
2. 新功能需附带测试用例
3. 提交前通过所有测试

## 许可证

GNU General Public License v3.0 (GPLv3)
