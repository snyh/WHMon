#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#define BOOST_THREAD_USE_LIB
#include <boost/thread.hpp>
#include <vector>
#include <list>
#include <set>
#include <iterator>
#include <iostream>
#include <wx/init.h>

#include "cmd.h"
namespace po = boost::program_options;
using namespace std;
using namespace boost::filesystem;
using namespace boost::asio;


Config theConfig;
string app_name;

/*
 * report() 每5s向已经注册的观察者发送本机各种状况
 */
vector<Observer> observers;

void report()
{
  io_service io;
  for(;;){
reportloop:
      try {
	  deadline_timer t(io, boost::posix_time::seconds(5));
	  t.wait();
	  //std::cout << "size:" << observers.size() << "\n";
	  BOOST_FOREACH(Observer o, observers){
	      ip::udp::socket s(io);
	      s.open(ip::udp::v4());
	      s.send_to(buffer(o.str()), o.peer());
	      //std::cout << "Send:(" << o.str() << ") to (" << o.peer() << "\n";
	  }
      } catch (std::exception& e) {
	  std::cout << "report() error:" << e.what();
	  goto reportloop;
      }
  }
}
void policy_server()
{
  using boost::asio::ip::tcp;
  static string  content("<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\" /></cross-domain-policy>");
  boost::asio::io_service io;
  tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 43204));
  for(;;){
      tcp::socket socket(io);
      try {
	  acceptor.accept(socket);
	  boost::asio::write(socket, boost::asio::buffer(content));
      } catch (std::exception& e) {
	  std::cout << "policy_server() error:" << e.what() << "\n";
      }
  }
}

int main(int argc, char **argv)
{
  wxInitialize();
  std::cout << "I'm begin\n";
  //命令行解析
  app_name.assign(argv[0]);
  po::options_description desc("options");
  desc.add_options()
    ("help,h", "help message")
    ("port,p", po::value<int>(), "local listen port")
    ("ip,i", po::value<std::vector<string> >(), "the ip which may control the app.\nused like this \n\t-i 192.168.1.1 -i 192.168.1.2 -i 192.168.1.3 ...")
    ("name,n", po::value<string>(), "classroom name")
    ("install", "install this program")
    ("uninstall", "uninstall this program");

  po::variables_map vm;
  try {
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);
  } catch (exception &e) {
      std::cout << "Exception:" << e.what() << std::endl;
      std::cout << desc << std::endl;
      return -1;
  }

  if(vm.count("help")){
      cout << "My Email: snyh1010@gmail.com or snyh@snyh.org\n";
      cout << desc << "\n";
      return 0;
  }
  if(vm.count("port")){
      theConfig.set_listen_port(vm["port"].as<int>());
  }
  if(vm.count("ip")){
      vector<string> a(vm["ip"].as<vector<string> >());
      BOOST_FOREACH(string ip, a)
	theConfig.add_ip(ip);
  }
  if(vm.count("name")){
      theConfig.set_name(vm["name"].as<string>());
  }
  theConfig.fresh(); //读取配置文件信息

  BOOST_FOREACH(boost::asio::ip::address& ip, theConfig.ips){
      boost::asio::ip::udp::endpoint p(ip, 43201);
      observers.push_back(Observer(p, -1));
  }

  //状态回送线程
  boost::thread report_thread(report);
  report_thread.detach();

  boost::thread policy_thread(policy_server);
  policy_thread;


  //监听 消息处理
  using boost::asio::ip::tcp;
  boost::asio::io_service io_service;
  tcp::acceptor *acceptor;
  tcp::socket socket(io_service);
  try {
      acceptor = new tcp::acceptor(io_service,
				   tcp::endpoint(tcp::v4(), theConfig.l_port)
				  );
  } catch (exception &e) {
      std::cout << "create socket error!  " << e.what() << "\n";
      return -1;
  }


mainloop:
  for(;;){
      try {
	  acceptor->accept(socket);

	  boost::asio::streambuf buf;
	  read_until(socket, buf, "\r\n");

	  std::string data((istreambuf_iterator<char>(&buf)),
			   istreambuf_iterator<char>());
	  std::cout << "Recive:" << data << "\n";
	  std::vector<ip::address>::iterator ip = find(theConfig.ips.begin(), 
						       theConfig.ips.end(),
						       socket.remote_endpoint().
						       address());
	  if(ip != theConfig.ips.end()) {
	      //parse_cmd返回的指针是预先分配好重复利用的的不可以释放
	      CMD *cmd = parse_cmd(std::string(data)); 
	      cmd->run(socket);
	      cout << "run ok\n";
	  } else {
	      std::cout << boost::format("illegal IP: %1%\n") % 
		socket.remote_endpoint().address().to_string();
	      CMD_error e;
	      e.set("Don't attach this system or your will..");
	      e.run(socket);
	      //TODO: 记录非法入侵地址和时间
	  }
      }  catch (exception& e){
	  //client 出现错误时，server可能会出现中断连接异常
	  cout << "main loop catch exception: " << e.what() << "\n";
	  goto mainloop; 
      }
  }
  wxUninitialize();
}
