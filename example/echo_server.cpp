#include "co_macro.h"
#include "co_sock.h"
#include "colang_server.h"
#include <iostream>
#include <sys/socket.h>
using namespace ab;

class EchoServer {
public:
  int run() {
    acceptor_.InitProtocol(true, true);
    int reuse = 1;
    setsockopt(acceptor_.Getfd(), SOL_SOCKET, SO_REUSEADDR, &reuse,
               sizeof(reuse));
    acceptor_.Bind("127.0.0.1", 40321);
    acceptor_.Listen(128);
    while (true) {
      CoSocket client_socket;
      int ret = acceptor_.Accept(client_socket);
      if (ret) {
        std::cout << "accept error" << std::endl;
        continue;
      }
      // 新启一个协程处理客户端连接
      co([sock = std::move(client_socket)]() mutable {
        std::string data;
        size_t len = 0;
        int ret = sock.Recv(data, len);
        if (ret != 0) {
          std::cout << "client_socket recv error " << ret << std::endl;
          return;
        }
        ret = sock.Send(data, len);
        if (ret != 0) {
          std::cout << "client_socket send error " << ret << std::endl;
          return;
        }
        return;
      })();
    }
    return 0;
  }

  CoSocket acceptor_;
};

CO_MAIN() {
  // 新启一个协程作为服务端
  co([]() {
    EchoServer server;
    server.run();
  })();
  ThisCoro::Yield();
  // 主携程不断发送数据给服务端
  for (int i = 0; i < 10; i++) {
    CoSocket client_socket;
    client_socket.InitProtocol(true, true);
    client_socket.Connect("127.0.0.1", 40321);
    std::string data = "hello world";
    size_t len = 0;
    client_socket.Send(data, len);
    std::string recv_data;
    client_socket.Recv(recv_data, len);
    std::cout << recv_data << std::endl;
  }
  std::cout << "finish" << std::endl;
  CO_RETUN(0);
}