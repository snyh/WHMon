#include "DbProxy.hpp"
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/any.hpp>

#include <Wt/WSignal>
#include <Wt/SyncLock>

#include <cstdlib>
#include <cstring>
#include <cassert>
using namespace std;



namespace {
    void createTables(sqlite3 *db_handle) {
	char *zErrMsg = 0;
	int res = sqlite3_exec(db_handle, 
			       "select count(*) from Clients",
			       0, 0, &zErrMsg);
	const char *sql_clients =
	  "CREATE TABLE Clients (\n"
	  "ID INTEGER PRIMARY KEY autoincrement,\n"
	  "IP TEXT NOT NULL UNIQUE,\n"
	  "NAME TEXT NOT NULL DEFAULT (\'NONAME\')\n"
	  ")";
	const char *sql_zones = 
	  "CREATE TABLE User2Zones (\n"
	  "USER TEXT NOT NULL,\n"
	  "ZONE INTEGER NOT NULL,\n"
	  "PRIMARY KEY (USER, ZONE),\n"
	  "FOREIGN KEY(ZONE) REFERENCES Zone2Name(ZONE)\n"
	  ")";
	const char *sql_users = 
	  "CREATE TABLE Users (\n"
	  "NAME TEXT NOT NULL UNIQUE,\n"
	  "PASSWD TEXT NOT NULL,\n"
	  "ROLE INTEGER NOT NULL\n"
	  ")";
	const char *sql_classroom =
	  "CREATE TABLE IP2Zone (\n"
	  "IP TEXT NOT NULL,\n"
	  "ZONE INTEGER NOT NULL,\n"
	  "PRIMARY KEY (IP, ZONE)\n"
	  "FOREIGN KEY(ZONE) REFERENCES Zone2Name(ZONE),\n"
	  "FOREIGN KEY(IP) REFERENCES Clients(IP)\n"
	  ")";
	//此触发器用来在删除一整个区域时，节点自动归属到0区域
	//和  更改某个节点到非0区域时， 自动删除它在0区域的数据
	const char *sql_zone_name =
	  "CREATE TABLE Zone2Name (\n"
	  "NAME TEXT NOT NULL UNIQUE, \n"
	  "ZONE INTEGER PRIMARY KEY autoincrement\n"
	  ")";
	const char *trigger_delDefaultZone = 
	  "CREATE TRIGGER delDefaultZone AFTER INSERT ON IP2Zone\n"
	  "WHEN new.ZONE != 0\n"
	  "BEGIN\n"
	  "DELETE FROM IP2Zone WHERE IP=new.IP and ZONE=0;\n"
	  "END\n";
	const char *trigger_move2defaultZone =
	  "CREATE TRIGGER move2defaultZone AFTER DELETE ON IP2Zone\n"
	  "WHEN old.IP not in (SELECT IP FROM IP2Zone)\n"
	  "BEGIN\n"
	  "INSERT INTO IP2Zone (IP, ZONE)\n"
	  "SELECT IP, 0 FROM Clients\n"
	  "WHERE old.IP = IP;\n"
	  "END;\n";
	const char *trigger_addToDefaultZone = 
	  "CREATE TRIGGER addToDefaultZone AFTER INSERT ON Clients\n"
	  "WHEN new.IP not in(SELECT IP FROM IP2Zone)\n"
	  "BEGIN\n"
	  "INSERT INTO IP2Zone VALUES(new.IP, 0);\n"
	  "END;\n";
	const char *trigger_delZoneInIP2Zone =
	  "CREATE TRIGGER del_zone_in_ip2zone AFTER DELETE On Zone2Name\n"
	  "FOR EACH ROW\n"
	  "begin\n"
	  "DELETE FROM IP2Zone\n"
	  "WHERE ZONE  = old.zone;\n"
	  "end;\n";
	const char *trigger_onDelClients =
	  "CREATE TRIGGER onDelClients AFTER DELETE ON Clients\n"
	  "BEGIN\n"
	  "DELETE FROM IP2Zone\n"
	  "WHERE IP = old.IP\n;"
	  "END\n";

	if (res != SQLITE_OK) {
	    if(sqlite3_exec(db_handle, sql_zone_name, 0, 0, &zErrMsg)
	       != SQLITE_OK) assert(!"");
	    if(sqlite3_exec(db_handle, sql_clients, 0, 0, &zErrMsg)
	       != SQLITE_OK) assert(!"");
	    if(sqlite3_exec(db_handle, sql_zones, 0, 0, &zErrMsg)
	       != SQLITE_OK) assert(!"");
	    if(sqlite3_exec(db_handle, sql_users, 0, 0, &zErrMsg)
	       != SQLITE_OK) assert(!"");
	    if(sqlite3_exec(db_handle, sql_classroom, 0, 0, &zErrMsg)
	       != SQLITE_OK) assert(!"");
	    if(sqlite3_exec(db_handle, trigger_delDefaultZone, 0, 0, &zErrMsg)
	       != SQLITE_OK) assert(!"");
	    if(sqlite3_exec(db_handle, trigger_delZoneInIP2Zone, 0, 0, &zErrMsg)
	       != SQLITE_OK) assert(!"");
	    if(sqlite3_exec(db_handle, trigger_addToDefaultZone, 0, 0, &zErrMsg)
	       != SQLITE_OK) assert(!"");
	    if(sqlite3_exec(db_handle, trigger_move2defaultZone, 0, 0, &zErrMsg)
	       != SQLITE_OK) {
		cout << zErrMsg;
		assert(!"");
	    }
	    if(sqlite3_exec(db_handle, trigger_onDelClients, 0, 0, &zErrMsg)
	       != SQLITE_OK) assert(!"");


	    if(sqlite3_exec(db_handle,
			    "insert into Users values(\"admin\", \"admin\", 0)",
			    0, 0, &zErrMsg)
	       != SQLITE_OK) assert(!"");
	    if(sqlite3_exec(db_handle,
			    "insert into Zone2Name values(\"未分类\", 0)",
			    0, 0, &zErrMsg)
	       != SQLITE_OK) assert(!"");
	}
    }

    static int getClientsCB(void *data, int argc, char **argv, char **azColName) {
	vector<Client> *clients = static_cast<vector<Client>*>(data);
	Client c;
	for (int i=0; i < argc; i++) {
	    if(strncmp(azColName[i], "ID", 2) == 0)
	      c.id = atoi(argv[i]);
	    else if(strncmp(azColName[i], "IP", 2) == 0)
	      c.ip = argv[i];
	    else if(strncmp(azColName[i], "NAME", 4) == 0)
	      c.name = argv[i];
	    else 
	      assert(!"Not Should arrive here!");
	}
	clients->push_back(c);
	return 0;
    }

    static int getClients2CB(void *data, int argc, char **argv, char **azColName) {
	vector<string> *ips = static_cast<vector<string>*>(data);
	assert(argc == 1);

	if(strncmp(azColName[0], "IP", 2) == 0)
	  ips->push_back(argv[0]);
	else
	  assert(!"Not Should arrive here!");

	return 0;
    }

    static int getUserCB(void *data, int argc, char **argv, char **azColName) {
	User* u = static_cast<User*>(data);
	for (int i=0; i < argc; i++) {
	    if(strncmp(azColName[i], "NAME", 4) == 0)
	      u->name = argv[i];
	    else if(strncmp(azColName[i], "PASSWD", 6) == 0)
	      u->passwd  = argv[i];
	    else if(strncmp(azColName[i], "ROLE", 4) == 0)
	      u->role = User::ROLE(atoi(argv[i]));
	    else 
	      assert(!"Not Should arrive here!");
	}
	return 0;
    }
    static int runSQLCB(void *data, int argc, char **argv, char **azColName) {
		DbData::Type& tmp  = static_cast<DbData*>(data)->_data;
		static_cast<DbData*>(data)->_count++;
		for (int i=0; i < argc; i++) {
			map<string, vector<string> >item;
			if(tmp.find(azColName[i]) == tmp.end()) {
				vector<string> _column;
				tmp.insert(make_pair(azColName[i], _column));
			}
			tmp[azColName[i]].push_back(argv[i]);
	}

	return 0;
    }
}


DbProxy::DbProxy(const string& db_path)
{
  int rc = sqlite3_open(db_path.c_str(), &_db_handle);
  if (rc) {
      cerr << "Open the database: " << db_path << " failed!\n";
      exit(1);
  } 
  createTables(_db_handle); 
}
DbProxy::~DbProxy()
{
  sqlite3_close(_db_handle);
}


vector<Client> DbProxy::getClients(int zone)
{
  string sql = "select IP from IP2Zone where ZONE = "
    + boost::lexical_cast<string>(zone);
  vector<string> ips;
  vector<Client> clients;
  sqlite3_exec(_db_handle, sql.c_str(), getClients2CB, &ips, &err_msg);
  for (size_t i=0; i<ips.size(); ++i) {
      string sql2 = "select * from Clients where IP = " + ips[i];
      sqlite3_exec(_db_handle, sql2.c_str(), getClientsCB, &clients, &err_msg);
  }
  return clients;
}

bool DbProxy::modifyClient(const Client& c)
{
  const char* sql = 
    "UPDATE Clients\n"
    "SET IP = ?,"
    "NAME = ?"
    "WHERE ID = ?\n";
  sqlite3_stmt *stmt = 0;
  sqlite3_prepare(_db_handle, sql, -1, &stmt, 0); 
  sqlite3_bind_text(stmt, 1, c.ip.c_str(), -1, 0);
  sqlite3_bind_text(stmt, 2, c.name.c_str(), -1, 0);
  sqlite3_bind_int(stmt, 3, c.id);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return true;
}



User DbProxy::getUser(const string& name)
{
  string sql = "select * from Users where name = \"" + name + "\"";
  User u;
  sqlite3_exec(_db_handle, sql.c_str(), getUserCB, &u, &err_msg);
  return u;
}
bool DbProxy::modifyUser(User& u)
{
  return delUser(u.name) &&  addUser(u);
}
bool DbProxy::addUser(User& u)
{
  delUser(u.name);

  const char* sql = "insert into Users values(?, ?, ?)";
  sqlite3_stmt *stmt = 0;
  if(SQLITE_OK != sqlite3_prepare(_db_handle, sql, -1, &stmt, 0)){
      assert(!__LINE__);
  }
  if(SQLITE_OK != sqlite3_bind_text(stmt, 1, u.name.c_str(), -1, 0)){
      assert(!__LINE__);
  }
  if(SQLITE_OK != sqlite3_bind_text(stmt, 2, u.passwd.c_str(), -1, 0)){
      assert(!__LINE__);
  }
  if(SQLITE_OK != sqlite3_bind_int(stmt, 3, int(u.role))){
      assert(!__LINE__);
  }
  if(sqlite3_step(stmt) != SQLITE_DONE) {
      //assert(!__LINE__);
      return false;
  }
  sqlite3_finalize(stmt);
  return true;
}
bool DbProxy::delUser(const string& name)
{
  const char* sql = "delete from Users where name = ?";
  sqlite3_stmt *stmt = 0;
  if(SQLITE_OK != sqlite3_prepare(_db_handle, sql, -1, &stmt, 0)){
      assert(!__LINE__);
  }
  if(SQLITE_OK != sqlite3_bind_text(stmt, 1, name.c_str(), -1, 0)){
      assert(!__LINE__);
  }
  if(sqlite3_step(stmt) != SQLITE_DONE) {
      //assert(!__LINE__);
      return false;
  }
  sqlite3_finalize(stmt);
  return true;
}



bool DbProxy::addZoneNames(const string& n)
{
  string sql = "insert into Zone2Name values(\"" + n + "\", NULL)";
  return runSQL(sql);
}
bool DbProxy::delZoneNames(int id)
{
  string sql = "DELETE from Zone2Name where ZONE = " + boost::lexical_cast<string>(id);
  return runSQL(sql);
}


bool DbProxy::addZoneByIP(const string& ip, int zone)
{
  string sql = str(boost::format("INSERT INTO IP2Zone VALUES(\"%1%\", %2%)") % ip % zone);
  return runSQL(sql);
}
bool DbProxy::delZoneByIP(const string& ip, int zone)
{
  string sql = str(boost::format("DELETE FROM IP2Zone WHERE IP = \"%1%\" AND ZONE =  %2%") % ip % zone);
  return runSQL(sql);
}

bool DbProxy::runSQL(const string& sql)
{
  Wt::SyncLock<boost::recursive_mutex::scoped_lock> lock(_mutex);
  if (SQLITE_OK != sqlite3_exec(_db_handle, sql.c_str(), 0, 0, &err_msg)) {
      cerr << "\tDbProxy::runSQL() Failed.\n"
	<< "\tWith SQL STATEMENT: " << sql << "\n"
	<< "\tError Message: " << err_msg << "\n";
      return false;
  }
  return true;
}

bool DbProxy::runSQL(const string& sql, DbData& data)
{
  Wt::SyncLock<boost::recursive_mutex::scoped_lock> lock(_mutex);
  if (SQLITE_OK != sqlite3_exec(_db_handle, sql.c_str(),
				runSQLCB, &data, &err_msg)) {
      cerr << "\tDbProxy::runSQL() Failed.\n"
	<< "\tWith SQL STATEMENT: " << sql << "\n"
	<< "\tError Message: " << err_msg << "\n";
      return false;
  }
  return true;
}

bool DbProxy::addZone(const string& name)
{
  string  sql = "INSERT INTO Zone2Name  VALUES(\"" + name + "\", NULL)";
  return runSQL(sql);
}
bool DbProxy::modifyZone(int zone, const string& name)
{
  string sql = 
    str(boost::format("UPDATE Zone2Name SET NAME = \"%1%\" where ZONE = %2%")
	% name % zone);
  return runSQL(sql);
}
bool DbProxy::delZone(int zone)
{
  string sql = str(boost::format("DELETE FROM Zone2Name  where ZONE = %1%") % zone);
  return runSQL(sql);
}

bool DbProxy::addClient(const Client& c)
{
  string sql = str(boost::format("INSERT INTO Clients values(NULL, \"%1%\", \"%2%\");") % c.ip % c.name);
  return runSQL(sql);
}

bool DbProxy::delClient(int id)
{
  string sql = "DELETE FROM Clients WHERE ID = " + boost::lexical_cast<string>(id);
  return runSQL(sql);
}
int DbProxy::getClientID(string ip)
{
  string sql = "SELECT ID FROM Clients WHERE IP = \"" + ip + "\";";
  DbData d;
  runSQL(sql, d);
  return d.fetch<int>(0,"ID");
}

map<int, Client> DbProxy::getClients()
{
  string sql  = "SELECT * FROM Clients";
  DbData d;
  runSQL(sql, d);

  map<int, Client> clients;
  for (int i=d.count()-1; i>=0; --i) {
      int id = d.fetch<int>(i, "ID");
      string ip = d.fetch<string>(i, "IP");
      string name = d.fetch<string>(i, "NAME");
      clients.insert(make_pair(id, Client(ip, name, id)));
  }
  return clients;
}

vector<int> DbProxy::getZonesByUser(const string& user)
{
  string sql = "select ZONE from User2Zones where USER = \"" + user + "\"";
  DbData d;
  runSQL(sql, d);
  vector<int> zones;
  for (int i=d.count()-1; i>=0; --i) {
      zones.push_back(d.fetch<int>(i, "ZONE"));
  }
  return zones;
}
vector<int> DbProxy::getZonesByIP(const string& ip)
{
  string sql = "select ZONE from IP2Zone where IP = \"" + ip + "\"";
  DbData d;
  runSQL(sql, d);
  vector<int> zones;
  for (int i=d.count()-1; i>=0; --i) {
      zones.push_back(d.fetch<int>(i, "ZONE"));
  }
  return zones;
}
map<int, string> DbProxy::getZoneNames(const string& user)
{
  string sql = "SELECT z.NAME, z.ZONE FROM Zone2Name z, User2Zones u WHERE u.ZONE=z.ZONE and u.USER=\""
    + user + "\";";
  DbData d;
  runSQL(sql, d);

  map<int, string> names;
  for (int i=d.count()-1; i>=0; --i) {
      names.insert(make_pair(d.fetch<int>(i, "ZONE"),
			     d.fetch<string>(i, "NAME")));
  }
  return names;
}
map<int, string> DbProxy::getZoneAllNames()
{
  string sql = "SELECT NAME, ZONE FROM Zone2Name;";
  DbData d;
  runSQL(sql, d);
  map<int, string> names;
  for (int i=d.count()-1; i>=0; --i) {
      names.insert(make_pair(d.fetch<int>(i, "ZONE"),
			     d.fetch<string>(i, "NAME")));
  }
  return names;
}
