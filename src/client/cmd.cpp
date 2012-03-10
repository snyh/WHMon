#include "cmd.hpp"

#include <string>
#include <vector>
#include <iterator>
#include <iostream>
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>

using namespace std;
using namespace boost::asio;
namespace pl = boost::asio::placeholders;
static io_service _io;

Cmd::Cmd(int id, string ip, string data, CallBack f)
	:id(id), data(data), _s(_io), peer(ip::address::from_string(ip), 43200), callback(f)
{
}
void Cmd::send()
{
  _io.reset();
  _s.async_connect(peer,
		   boost::bind(&Cmd::handle_connect, this, pl::error));
  _io.run();
}
void Cmd::handle_connect(const boost::system::error_code& err)
{
  if(!err){
      boost::asio::async_write(_s, buffer(boost::str(boost::format("%1% %2%\r\n") % id % data)),
			       boost::bind(&Cmd::handle_write, this, pl::error));
  } else {
      Response r;
      r.id = id;
      r.code = -1;
      r.descript = err.message();
      if(!callback.empty())
	callback(r);
  }
}

void Cmd::handle_write(const boost::system::error_code& err)
{
  if(!err){
      boost::asio::async_read_until(_s, _buf, "\r\n",
				    boost::bind(&Cmd::handle_read, this, pl::error));
  } else {
      cout << "handle_write error\n";
      Response r;
      r.id = id;
      r.code = -1;
      r.descript = err.message();
      if(!callback.empty())
	callback(r);
  }
}
void Cmd::handle_read(const boost::system::error_code& err)
{
  if(err){
      cout << "handle_read error\n";
      Response r;
      r.id = id;
      r.code = -1;
      r.descript = err.message();
      if(!callback.empty())
	callback(r);
      return;
  }


  //正常情况
  Response r;
  string d((istreambuf_iterator<char>(&_buf)),
	   istreambuf_iterator<char>());
  vector<string> tokens;
  boost::split(tokens, d, boost::is_any_of(" "));

  cout << "Response:" << d << "\n";//TODO:Debug info

  //先决条件  至少有3个字段
  //ID [OK|ERR] CMD [ARG] 
  if(tokens.size() < 3){
      r.code = -1;
      r.descript = str(boost::format("错误包: %1% != %2%") % d % id);
      if(!callback.empty())
	callback(r);
      return;
  }

  //构造Response r
  r.id = id;

  if(tokens[1] == "OK"){
      r.code = 0;
  } else if(tokens[1] == "ERR"){
      r.code = -1;
  } else {
      r.code = -1;
      r.descript = str(boost::format("错误包: %1% != %2%") % d % id);
      if(!callback.empty())
	callback(r);
      return;
  }

  r.value["CMD"] = tokens[2];


  //错误检查 返回的ID应该和发送的ID相同
  try {
      if(boost::lexical_cast<int>(tokens[0]) != id){
	  r.code = -1;
	  r.descript = str(boost::format("错误标示号: %1% != %2%") % tokens[0] % id);
	  if(!callback.empty())
	    callback(r);
	  return;
      } 
  } catch (...) {
      r.code = -1;
      r.descript = str(boost::format("错误包: %1% != %2%") % d % id);
      if(!callback.empty())
	callback(r);
      return;
  }


  //处理VNC 控制包
  if(tokens.size() != 5){
      r.code = -1;
      r.descript = str(boost::format("错误VNC包: %1% != %2%") % d % id);
      if(!callback.empty())
	callback(r);
      return;
  } else {
      if(r.value["CMD"] == "OPENVNC"){
	  r.descript = "VNC CMD";
	  r.value["MODE"] = tokens[3];
	  r.value["PASSWD"] = tokens[4];
	  r.id = r.id;
      }
  }

  //其他包
  if(!callback.empty())
    callback(r);
}

void update(boost::function<void(string,string, int)> fun)
{
  for(;;){
      io_service io;
      ip::udp::socket s(io, ip::udp::endpoint(ip::udp::v4(), 43201));
      ip::udp::endpoint peer;
      char buf[128] = {0};
      s.receive_from(buffer(buf, 128), peer);

      vector<string> tokens;
      boost::split(tokens, buf, boost::is_any_of(" "));

      // copy(tok.begin(), tok.end(), back_inserter(tokens));
      assert(tokens.size() >= 2);
      if(tokens.size() < 2)
	continue;

      if (tokens[0] == "-1") {
	  if(!fun.empty())
	    fun(peer.address().to_string(), tokens[1], -1);
      } else {
	  int id = -1;
	  try{
	      id = boost::lexical_cast<int>(tokens[0]);
	  } catch(...) {
	      cout << "err id in cmd.cpp:update()\n";
	      return;
	  }
	  if (tokens[1] == "OK") {
	      //cout << "xintiaobao:" << id << "\n";
	      if(!fun.empty())
		fun(peer.address().to_string(), "NONAME", id);
	      //心跳包
	  } else if (tokens[1] == "ERR") {
	      //其他错误信息
	  }
      }
  }
}
