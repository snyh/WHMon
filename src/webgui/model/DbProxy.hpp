#ifndef __DBPROXY__
#define __DBPROXY__
#include <string>
#include <vector>
#include <map>
#include <Wt/WObject>
#include <boost/utility.hpp>
#include <Wt/WStandardItemModel>
#include <Wt/WSignal>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include <sqlite3.h>

struct Client {
    Client(const std::string& ip="", const std::string& name="", int id = -1)
      : id(id), ip(ip), name(name) {}

    int id; 
    std::string ip;
    std::string name;
};
struct Zone {
    std::string user;
    int number;
};
struct User {
    enum ROLE { SuperAdmin=0, Admin, Visitor };
    std::string name;
    std::string passwd;
    ROLE role;
};

class DbData {
public:
  DbData():_count(0){}
  class ROW_COLUMN_OUT_RANGE {};
  template <typename T> 
    T fetch(unsigned int row, const std::string& column) {
	if(row<0 || column.empty() || row >= _count)
	  throw ROW_COLUMN_OUT_RANGE() ;
	std::string item = _data.at(column).at(row);
	return boost::lexical_cast<T>(item);
    }
  size_t count() const { return _count; }

  typedef std::map<const std::string,  //COLUMN NAME
	  std::vector<std::string> // DATA
	    > Type;
  Type _data;
  size_t _count;
};

//TODO :剔除部分函数，或者内部使用runSQL执行
//这是一个失败的东西。。。
//所以最后添加了 runSQL这个函数 直接运行sql语句
//因为 单独的函数 每调用一次都会去实际操作数据库
//如果这个函数需要调用很多次会导致 
//连接数据库，写数据库，断开连接 重复的行为
//
//之前虽然也意识到但觉得没有多大问题，但效率实在
//不行 
class DbProxy : public Wt::WObject, private boost::noncopyable {
public:
  static DbProxy& getInstance() { return _instance; }

  bool runSQL(const std::string& sql);
  bool runSQL(const std::string& sql, DbData& data);


  bool addZone(const std::string& zone_name);
  bool modifyZone(int, const std::string&);
  bool delZone(int zone_n);

  int  getClientID(std::string ip);
  bool delClient(int id);
  std::map<int, Client> getClients();

  std::vector<int> getZonesByIP(const std::string& ip);
  std::vector<int> getZonesByUser(const std::string& user);

  std::map<int, std::string> getZoneNames(const std::string& user);
  std::map<int, std::string> getZoneAllNames();
public:

  std::vector<Client> getClients(int zone);
  bool modifyClient(const Client& c);
  bool addClient(const Client& c);

  User getUser(const std::string& name);
  bool addUser(User& u);
  bool modifyUser(User& u);
  bool delUser(const std::string& name);

  bool addZoneByIP(const std::string&, int);
  bool delZoneByIP(const std::string&, int);


  bool addZoneNames(const std::string&);
  bool delZoneNames(int);
private:
  static DbProxy _instance;
  DbProxy(const std::string& db_path);
  ~DbProxy();
  sqlite3 *_db_handle;
  char *err_msg;

  boost::recursive_mutex _mutex;
};
#endif
