#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/signal.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>

#include <cstring>

#include "StateListener.hpp"
#include "cmd.hpp"
#include "DbProxy.hpp"
#include <iconv.h>

using namespace std;
using namespace boost::asio;

string G2U(const string& src)
{
  string re;
  const char *sfrom = src.c_str();

  size_t slen = strlen(sfrom);
  if(slen > (size_t) 1024)
    slen = 1024;

  iconv_t cd;
  if((cd = iconv_open("UTF-8", "GB18030")) < 0)
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



StateListener::StateListener()
	:_db(DbProxy::getInstance()),
	 _socket(_io, ip::udp::endpoint(ip::udp::v4(), client_listen_port)),
	 _recv_buffer(new char[128]),
	 _timer(_io, boost::posix_time::seconds(1))
{
  findClient(0, "0.0.0.0"); //更新_clients

  _timer.async_wait(boost::bind(&StateListener::delStateCount, this)); 

  memset(_recv_buffer, 0, 128);
  _socket.async_receive_from(buffer(_recv_buffer, 128), _peer,
			     boost::bind(&StateListener::onRecive, this, 
					 boost::asio::placeholders::error,
					 boost::asio::placeholders::bytes_transferred)
   );

  boost::thread t(boost::bind(&StateListener::run, this));
  t.detach();
}

void StateListener::run()
{
  _io.run();
}
StateListener::~StateListener()
{
  delete[] _recv_buffer;
}

void StateListener::onRecive(const boost::system::error_code& error, size_t s)
{
  vector<string> tokens;
  boost::split(tokens, _recv_buffer, boost::is_any_of(" "));
  if(tokens.size() >= 2) {
      try{
	  int id = -1;
	  id = boost::lexical_cast<int>(tokens[0]);
	  handleCMD(_peer.address().to_string(), tokens[1], id);
      } catch(std::exception& e){
	  cerr << "err id in StateListner.cpp:onRecive()\n";
	  cerr << e.what() << "\n";
      }
  }

  memset(_recv_buffer, 0, 128);
  _socket.async_receive_from(
			     buffer(_recv_buffer, 128), _peer,
			     boost::bind(&StateListener::onRecive, this, 
					 boost::asio::placeholders::error,
					 boost::asio::placeholders::bytes_transferred)
			    );
}

void StateListener::tick(int id)
{
  boost::mutex::scoped_lock lock(_mutex);

  _state_changed(StateEvent(StateEvent::OnLine, id));

  _states[id] = count_max;
}
void StateListener::delStateCount()
{
  boost::mutex::scoped_lock lock(_mutex);
  std::map<int, int>::iterator i= _states.begin();
  for (; i!=_states.end(); ++i) {
      if(i->second == 0)
	continue;
      else if (i->second > 0)
	i->second--;

      //--------------------------------//
      if (i->second == 0)
	_state_changed(StateEvent(StateEvent::OffLine, i->first));
  }

  _timer.expires_at(_timer.expires_at() + boost::posix_time::seconds(1));
  _timer.async_wait(boost::bind(&StateListener::delStateCount, this)); 
}

/*查找对应ID的Client
 *如果没有找到更新一次数据之后再进行查找
 *并且更新 _states数据
 */
bool StateListener::findClient(int id, const string& ip)
{
  if (_clients.find(id) != _clients.end() && _clients[id].ip == ip) {
      return true;
  } else {
      _clients = _db.getClients();

      boost::mutex::scoped_lock lock(_mutex);
	  std::map<int, Client>::iterator i = _clients.begin();
      for (; i!=_clients.end(); ++i) {
	  if(_states.find(i->first) == _states.end())
	    _states.insert(make_pair(i->first, 0));
      }

      if(_clients.find(id) != _clients.end() && _clients[id].ip == ip)
	return true;
      else
	return false;
  }
}

void StateListener::handleCMD(const string& ip, const string& name, int id)
{
  if(id == -1) {
      cout << "收到发现包 " << ip << "(" << name << ")" 
	<< " ID：" << id << "\n";

      bool find = false;
      typedef pair<int, Client> T;
      BOOST_FOREACH (const T& p, _clients) {
	  if(p.second.ip == ip){
	      find = true;
	      //小心修改了参数, 下面的send_cmd hello 需要使用id
	      id = p.second.id;
	      break;
	  }
      }
      if (!find) {
	  _db.addClient(Client(ip, G2U(name)));
	  _state_changed(StateEvent(StateEvent::NewClient, id));

	  findClient(0, "0.0.0.0");
	  //更新_clients
      }
      tick(id);
      cmd_async_send(id, ip, "HELLO");
  } else if(findClient(id, ip)) {
      cout << "收到心跳包 " << ip << " ID：" << id << "\n";
      tick(id);
  } else {
      /*如果收到的包ID 不等于-1 但也不在当前数据库里
       * 则可能是 之前的连接了的Client的发送的数据包 
       * 此时发送 BYE 命令 使其获得正确的ID
       */
      cmd_async_send(id, ip, "BYE");
  }
}
