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
#include <iconv.h>


using namespace std;
using namespace boost::asio;
Cmd::CallBack Cmd::nullCB;
static io_service _io;

string U2G(const string& src)
{
  string re;
  const char *sfrom = src.c_str();

  size_t slen = strlen(sfrom);
  if(slen > (size_t) 1024)
    slen = 1024;

  iconv_t cd;
  if((cd = iconv_open("GB18030", "UTF-8")) < 0)
    return re;
  char *sin = (char*)sfrom;
  char sto[1024] = {0};
  char *dout = sto;
  size_t dlen = 800;
  if (iconv(cd, &sin, &slen, &dout, &dlen) < 0)
    return re;
  iconv_close(cd);
  return sto;
}

boost::signals::connection
cmd_async_send(int id, const string& ip, const string& data, Cmd::CallBack f)
{
  std::cout << "发送[" << data << "] 到" << ip << " [" << id << "]\n";
  boost::shared_ptr<Cmd> cmd(new Cmd(id, ip, U2G(data)));
  boost::signals::connection con;
  if(!f.empty())
    con = cmd.get()->onCompleted().connect(f);
  boost::thread t(boost::bind(&Cmd::send, cmd));
  t.detach();
  return con;
}

Cmd::Cmd(int id, const string& ip, const string& data)
	:id(id),
	data(data),
	_s(_io), 
	peer(ip::address::from_string(ip), server_listen_port)
{
}
Cmd::~Cmd()
{
}
void Cmd::send()
{
  _io.reset();
  _s.async_connect(peer,
		   boost::bind(&Cmd::handle_connect, this, boost::asio::placeholders::error));
  _io.run();
}
void Cmd::handle_connect(const boost::system::error_code& err)
{
  if(!err){
      boost::asio::async_write(_s, buffer(boost::str(boost::format("%1% %2%\r\n") % id % data)),
			       boost::bind(&Cmd::handle_write, this, boost::asio::placeholders::error));
  } else {
      Response r;
      r.id = id;
      r.code = -1;
      r.descript = err.message();
      _on_completed(r);
  }
}

void Cmd::handle_write(const boost::system::error_code& err)
{
  if(!err){
      boost::asio::async_read_until(_s, _buf, "\r\n",
				    boost::bind(&Cmd::handle_read, this, boost::asio::placeholders::error));
  } else {
      cout << "handle_write error\n";
      Response r;
      r.id = id;
      r.code = -1;
      r.descript = err.message();
      _on_completed(r);
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
      _on_completed(r);
      return;
  }


  //正常情况
  Response r;
  string d((istreambuf_iterator<char>(&_buf)),
	   istreambuf_iterator<char>());
  vector<string> tokens;
  boost::split(tokens, d, boost::is_any_of(" "));

  //cout << "Response:" << d << "\n";//TODO:Debug info

  //先决条件  至少有3个字段
  //ID [OK|ERR] CMD [ARG] 
  if(tokens.size() < 3){
      r.code = -1;
      r.descript = str(boost::format("错误包: %1% != %2%") % d % id);
      _on_completed(r);
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
      _on_completed(r);
      return;
  }

  r.value["CMD"] = tokens[2];


  //错误检查 返回的ID应该和发送的ID相同
  try {
      if(boost::lexical_cast<int>(tokens[0]) != id){
	  r.code = -1;
	  r.descript = str(boost::format("错误标示号: %1% != %2%") % tokens[0] % id);
	  _on_completed(r);
	  return;
      } 
  } catch (...) {
      r.code = -1;
      r.descript = str(boost::format("错误包: %1% != %2%") % d % id);
      _on_completed(r);
      return;
  }


  //处理VNC 控制包
  if(tokens.size() != 5){
      r.code = -1;
      r.descript = str(boost::format("错误VNC包: %1% != %2%") % d % id);
      _on_completed(r);
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
  _on_completed(r);
}

