#ifndef __STATELISTENER__
#define __STATELISTENER__

#include <assert.h>
#include <map>
#include <vector>
#include <boost/utility.hpp>
#include <boost/signal.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "DbProxy.hpp"

const int client_listen_port = 43201;
const int count_max = 15;

/*StateListener 另开一thread 监听 UDP 432001 
 * 接受  端点发现包  (ID为-1  的包)
 * 	 以及心跳包  (ID为接受端点发现包之后 指定的一个ID)
 * 通过 StateListener::changed 处理 这两种包 
 * 维护 Client::state_count 计数
 */

struct StateEvent {
    enum Type {NewClient, OnLine, OffLine};
    StateEvent(Type t, int id):type(t), id(id){}

    Type type;
    int id;
};

class StateListener : private boost::noncopyable {
public:
  typedef boost::signal<void(const StateEvent)> Signal;
public:
  Signal& stateChanged() {return _state_changed;}
  static StateListener& getInstance() { return _instance; }
  void run();
private:
  StateListener();
  ~StateListener();
  bool findClient(int id, const std::string& ip);
  void handleCMD(const std::string& ip, const std::string& name, int id);
  void tick(int id);
  void delStateCount();
  void onRecive(const boost::system::error_code& error, size_t s);

  DbProxy& _db;
  Signal _state_changed;

  std::map<int, Client> _clients; //pair<client_id, Client>
  std::map<int, int> _states; //pair<client_id, client_count>

  static StateListener _instance;
  boost::asio::io_service _io;
  boost::asio::ip::udp::socket _socket;

  char* _recv_buffer;
  boost::asio::ip::udp::endpoint _peer;
  boost::mutex _mutex;
  boost::asio::deadline_timer _timer;
};

#endif
