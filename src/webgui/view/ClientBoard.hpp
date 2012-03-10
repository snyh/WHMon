#ifndef __CLIENTBOARD__
#define __CLIENTBOARD__
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WProgressBar>
#include <Wt/Ext/TabWidget>
#include <Wt/WTabWidget>
#include <Wt/WWebWidget>
#include <Wt/WModelIndex>
#include <Wt/WEvent>
#include <Wt/WCheckBox>
#include <vector>
#include <string>
#include "../model/StateListener.hpp"

class ClientBoard : public Wt::WContainerWidget {
public:
  enum Style { None, Normal, Edit };
  ClientBoard(User& u, Wt::WContainerWidget *parrent=NULL);
  void setStyle(Style s);
  ~ClientBoard();

private:
  void handleClick(const Wt::WModelIndex& i, const Wt::WMouseEvent& me);
  void handleState(const StateEvent s);
  void editClient(int id, const std::string& name, const std::string& ip);

  void addClient();
  void delClient();

  void addZone();
  void delZone();
  void modifyZone();

  void createTableView(int zone);
  void updateModel(int zone);
  void changedTab(int index);
  Wt::WStandardItemModel* fetchModel(int zone);

  void createViewBox();
  void createEditBox();

  void shutdown();
  void sendMessage();

private:
  DbProxy& _db; //要先初始化 _db 后使用
  User& _user;
  std::map<int, std::string> _zones;
  Wt::Ext::TabWidget* _table;
  Wt::WApplication* _app;
  Wt::WCheckBox* _glob_action;

  Style _style;

  Wt::WContainerWidget* _button_box;
  MyLogger* _logger;

  //因为这2个结构都只会存储少量数据 所以没有使用boost::multi_index
  std::map<int, Wt::WTableView*> _views;
  std::map<int, Wt::WStandardItemModel*> _models; //不要直接使用
  //std::map<int, int, Wt::WModelIndex> _indexes;
  std::map<int, StateEvent::Type> _states;

  boost::signals::connection _con;


  //std::string _vnc_passwd;
};

#endif
