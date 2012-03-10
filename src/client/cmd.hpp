#ifndef __CMD_HPP__
#define __CMD_HPP__
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <string>
#include <map>

struct Response{
    int id;
    int code;
    std::string descript;
    std::map<std::string, std::string> value;
};
typedef boost::function<void(Response&)> CallBack;

class Cmd {
public:
  Cmd(int id, std::string ip, std::string data, CallBack f);
  void send();
private:
  int id;
  std::string data;

  boost::asio::ip::tcp::socket _s;
  boost::asio::ip::tcp::endpoint peer;

  boost::asio::streambuf _buf;

  CallBack callback;
  void handle_connect(const boost::system::error_code& err);
  void handle_write(const boost::system::error_code& err);
  void handle_read(const boost::system::error_code& err);
};
#endif
