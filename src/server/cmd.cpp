#include <wx/config.h>
#include <wx/utils.h>
#include "cmd.h"
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <list>
#include <set>

#include <iostream>

#include <cstdlib>
#include <ctime>

using namespace std;
using boost::asio::ip::tcp;

/*
char* U2G(const char* utf8)
{
  int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
  wchar_t* wstr = new wchar_t[len+1];
  memset(wstr, 0, len+1);
  MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
  len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
  char* str = new char[len+1];
  memset(str, 0, len+1);
  WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
  if(wstr) delete[] wstr;
  return str;
}
*/

void hstart(std::string cmd, int flag=wxEXEC_ASYNC)
{
  if(wxFileExists(_T("c:\\windows\\whmon\\hstart.exe")))
      wxExecute(_T("c:\\windows\\whmon\\hstart.exe /NOCONSOLE \"") +
		wxString(cmd.c_str(), wxConvLocal) +
		_T("\""),
		flag);
  else
    wxExecute(wxString(cmd.c_str(), wxConvLocal), flag);
}

void config_vnc(bool);
string get_passwd(bool raw=false);

extern Config theConfig;
extern string app_name;
extern vector<Observer> observers;

static CMD* CMDs[8] = {
    new CMD_hello(),
    new CMD_bye(),
    new CMD_openvnc(),
    new CMD_closevnc(),
    new CMD_shutdown(),
    new CMD_update(),
    new CMD_show(),
    new CMD_error(),
};
static CMD* CMD_err = CMDs[7];

void Config::fresh()
{
//TODO: 去除对wxconfig 的使用
  wxConfig config(_T("WHMon"));
  wxString ip;
  long dummy;
  if (!config.Read(_T("LocalPort"), &dummy)) {
      std::cout << "Please Configure LocalPort before run WHMon\n"
		<< "refrence " << app_name << "--help\n";
      exit(-3);
  }
  this->l_port = dummy;

  wxString name;
  if (config.Read(_T("name"), &name)) {
      this->c_name = name.mb_str();
  }

  config.SetPath(_T("IPS"));
  bool t = config.GetFirstEntry(ip, dummy);
  while(t){
      try {
	  if(dummy == 0)
	    config.DeleteEntry(ip, true);
	  else 
	    ips.push_back(boost::asio::ip::address::from_string(
			  std::string(ip.mb_str()))
			 );
      } catch (boost::system::system_error &e) {
	  std::cout << "Exception: Error IP Format(" << ip << ")\n"
	  	    << e.what() << std::endl;
	  config.DeleteEntry(ip, true);
	  //自动清除前一次无效IP，因此输入时无须检测IP合法性
      }
      t = config.GetNextEntry(ip, dummy);
  }
  if(ips.empty()) {
      std::cout << "Please at least configure one server address\n"
		<< "refrence " << app_name << "--help\n";
      exit(-4);
  }
}
bool Config::add_ip(std::string ip)
{
  wxConfig config(_T("WHMon"));
  config.SetPath(_T("IPS"));
  return config.Write(wxString(ip.c_str(), wxConvLocal), 1);
}
bool Config::del_ip(std::string ip)
{
  wxConfig config(_T("WHMon"));
  config.SetPath(_T("IPS"));
  return config.Write(wxString(ip.c_str(), wxConvLocal), 0);
}
bool Config::set_listen_port(int port)
{
  wxConfig config(_T("WHMon"));
  return config.Write(_T("LocalPort"), port);
}
bool Config::set_name(std::string name)
{
  wxConfig config(_T("WHMon"));
  return config.Write(_T("name"), wxString(name.c_str(), wxConvLocal));
}



//各个命令的具体实现
void CMD_error::run(tcp::socket& s)
{
  //TODO:
  reply(s, str(boost::format("ERR %1%")  % err));
}
void CMD_hello::run(tcp::socket& s)
{
  boost::asio::ip::udp::endpoint p;
  p.address(s.remote_endpoint().address());
  p.port(43201);
  vector<Observer>::iterator it = find(observers.begin(), observers.end(),
				       Observer(p, id));
  it->reg(true);
  it->set_id(id);
  reply(s, "OK HELLO");
}
void CMD_bye::run(tcp::socket& s)
{
  boost::asio::ip::udp::endpoint p;
  p.address(s.remote_endpoint().address());
  p.port(43201);
  vector<Observer>::iterator it = find(observers.begin(), observers.end(),
				       Observer(p, id));
  it->reg(false);
  it->set_id(-1);
  reply(s, "OK BYE");
}

void CMD_openvnc::run(tcp::socket& s)
{
  try {
      hstart("sc stop tvnserver", wxEXEC_SYNC);
      config_vnc(true);
      hstart("sc start tvnserver", wxEXEC_SYNC);
      reply(s, "OK OPENVNC " + mode + " " + get_passwd(true));
  } catch(...) {
      CMD_error e;
      e.set("vnc program start error");
      e.run(s);
  }
}
void CMD_closevnc::run(tcp::socket& s)
{
  hstart("sc stop tvnserver", wxEXEC_SYNC);
  config_vnc(false);
  hstart("sc start tvnserver", wxEXEC_SYNC);
  reply(s, "OK CLOSEVNC");
  return;
}
void CMD_shutdown::run(tcp::socket& s)
{
  reply(s, "OK SHUTDOWN");
  wxExecute(_T("shutdown -s -c \"管理系统定时关机 若此计算机正在使用请使用 shutdown -a 取消此次关机\"")); //computer should be down
}
void CMD_show::run(tcp::socket& s)
{
  reply(s, "OK SHOW");
  string pro("c:\\windows\\whmon\\show.exe WHMon ");
  pro.append("\"");
  pro.append(message);
  pro.append("\"");
  cout << "show this: " << pro << endl;
  wxExecute(wxString(pro.c_str(), wxConvUTF8));
}
void CMD_update::run(tcp::socket& s)
{
  bool state=false;
  try {
      if(type == "ADDIP")
	state = theConfig.add_ip(data);
      else if(type == "DELIP")
	state = theConfig.del_ip(data);
      else if(type == "SETPORT")
	state = theConfig.set_listen_port(boost::lexical_cast<int>(data));
      theConfig.fresh();
  } catch (...) {
      CMD_error e;
      e.set("port not a number");
      e.run(s);
      return;
  }

  if(state)
    reply(s, "OK UPDATE");
  else 
    reply(s, "ERR UPDATE");
}
CMD *parse_cmd(std::string data)
{
  boost::tokenizer<> tok(data);
  list<string> tokens; 
  copy(tok.begin(), tok.end(), back_inserter(tokens));

  long id;
  string cmd;
  if(tokens.size() < 2)
    return CMD_err->set("tokens size < 2");
  try {
      id = boost::lexical_cast<long>(tokens.front());
      tokens.pop_front();
      cmd = tokens.front();
      tokens.pop_front();
  } catch (...) {
      return CMD_err->set("Bad package format");
  }

  tokens.push_back(data);
  BOOST_FOREACH(CMD *c, CMDs){
      //tokens 存储id cmd 之后的条目， 最后一项存储整个数据. 主要给cmd_show使用
      if(c->create(id, cmd, tokens))
	return c;
  }
  //cout << "tokens size: " << tokens.size() << "\n";
  return CMD_err->set("Not An Definited CMD");
}


void CMD::reply(tcp::socket& s, std::string message)
{
  try {
      boost::asio::write(s, boost::asio::buffer(
						str(boost::format("%1% %2%\r\n") % id % message)
					       )
			);
      s.close();
  } catch (boost::system::system_error &e) {
      std::cerr <<  "Exception:" << e.what() << std::endl;
  }
}

std::string Observer::str()
{
  if(is_reg){
      return boost::str(boost::format("%1% OK") % id);
  } else {
      return boost::str(boost::format("-1 %1%") % theConfig.c_name);
  }
}
