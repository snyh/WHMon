#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <list>
#include <string>



class Observer {
    int id;
    boost::asio::ip::udp::endpoint p;
    bool is_reg;
public:
    Observer(boost::asio::ip::udp::endpoint peer, int id) : id(id), p(peer), is_reg(false){}
    std::string str();
    boost::asio::ip::udp::endpoint peer() const { return p;}
    void reg(bool r){ is_reg = r;}
    void set_id(int id) { this->id = id;}
    bool operator== (const Observer &o) const{
	return p == o.p;
    }
};

struct Config {
public:
  std::vector<boost::asio::ip::address> ips;
  int l_port;
  std::string c_name;
  boost::asio::ip::udp::endpoint peer;
public:
  void fresh();
  bool add_ip(std::string);
  bool del_ip(std::string);
  bool set_listen_port(int port);
  bool set_name(std::string);
};

class CMD {
public:
  virtual void run(boost::asio::ip::tcp::socket&) = 0;
  virtual bool create(int id, std::string c,
		      std::list<std::string>& tokens){return false;};

  void reply(boost::asio::ip::tcp::socket&, std::string message);
  virtual CMD* set(std::string){
      //不好的设计只有CMD_error使用此方法。不应该增加到基类里来
      //临时发现parse_cmd返回CMD_error时new 操作 内存泄漏所以增加此方法
     return this;
  }
protected:
  int id;
};

CMD* parse_cmd(std::string data);


class CMD_error : public CMD {
    std::string err;
public:
    CMD* set(std::string m){ id=-1;err = m; return this;}
    void run(boost::asio::ip::tcp::socket&);
};

class CMD_hello : public CMD {
public:
    bool create(int id, std::string c, std::list<std::string>& tokens) {
	this->id = id;
	if(tokens.size() != 1 || c != "HELLO")
	  return false;
	return true;
    }
    void run(boost::asio::ip::tcp::socket&);
};

class CMD_bye : public CMD {
public:
    bool create(int id, std::string c, std::list<std::string>& tokens){
	this->id = id;
	if(tokens.size() != 1 || c != "BYE")
	  return false;
	return true;
    }
    void run(boost::asio::ip::tcp::socket&);
};

class CMD_openvnc : public CMD{
    std::string mode;
public:
  bool create(int id, std::string c, std::list<std::string>& tokens) {
      this->id = id;
      if(tokens.size() != 2 || c != "OPENVNC")
	return false;
      mode = tokens.front();
      return true;
  }
  void run(boost::asio::ip::tcp::socket&);
};

class CMD_closevnc : public CMD{
public:
  bool create(int id, std::string c, std::list<std::string>& tokens) {
      this->id = id;
      if(tokens.size() != 1)
	return false;
      return c == "CLOSEVNC" ? true : false;
  }
  void run(boost::asio::ip::tcp::socket&);
};

class CMD_shutdown : public CMD{
public:
  bool create(int id, std::string c, std::list<std::string>& tokens) {
      this->id = id;
      if(tokens.size() != 1)
	return false;
      return c == "SHUTDOWN" ? true : false;
  }
  void run(boost::asio::ip::tcp::socket&);
};
class CMD_show : public CMD{
    std::string message;
public:
  bool create(int id, std::string c, std::list<std::string>& tokens){
      /*
      if(tokens.size() <= 1)
	return false;
	*/
      this->id = id;

      std::string dummy;
      std::istringstream is(tokens.back());
      is >> dummy; //id 
      is >> dummy; //cmd(show)
      getline(is, message);

      return c == "SHOW" ? true : false;
  }
  void run(boost::asio::ip::tcp::socket&);
};

class CMD_update : public CMD {
    std::string type; 
    std::string data;
public:
    bool create(int id, std::string c, std::list<std::string>& tokens){
	this->id = id;
	if(tokens.size() != 3 || c != "UPDATE")
	  return false;

	type = tokens.front();
	tokens.pop_front();
	if(type != "ADDIP" &&
	   type != "DELIP" &&
	   type != "SETPORT")
	  return false;

	data = tokens.front();
	return true;
    }
    void run(boost::asio::ip::tcp::socket&);
};
