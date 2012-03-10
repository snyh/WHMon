#ifndef __CMD_HPP__
#define __CMD_HPP__
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/signal.hpp>
#include <string>
#include <map>

struct Response{
    int id;
    int code;
    std::string descript;
    std::map<std::string, std::string> value;
};


class Cmd {
public:
  typedef boost::signal<void(Response)> Signal;
  typedef boost::function<void(Response)> CallBack;
  static CallBack nullCB;
public:
  ~Cmd();
  Cmd(int id, const std::string& ip, const std::string& data);
  boost::signal<void(Response)>& onCompleted() { return _on_completed; };
  void send();
private:
  int id;
  std::string data;

  boost::asio::ip::tcp::socket _s;
  boost::asio::ip::tcp::endpoint peer;
  boost::asio::streambuf _buf;

  void handle_connect(const boost::system::error_code& err);
  void handle_write(const boost::system::error_code& err);
  void handle_read(const boost::system::error_code& err);

  boost::signal<void(Response)> _on_completed;
};
const int server_listen_port = 43200;

boost::signals::connection
cmd_async_send(int id, const std::string& ip,
	       const std::string& data, Cmd::CallBack f=Cmd::nullCB);

#endif
