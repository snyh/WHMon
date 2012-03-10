#include <Wt/WText>
#include <Wt/WBreak>
#include <Wt/WHBoxLayout>
#include <Wt/Ext/Button>
#include <Wt/WLineEdit>
#include <Wt/WDialog>
#include <Wt/WPushButton>
#include <Wt/WGridLayout>
#include <Wt/WFitLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WProgressBar>
#include <Wt/WContainerWidget>
#include <Wt/WScrollArea>
#include <Wt/WWebWidget>
#include <Wt/WCheckBox>
#include <Wt/WTableView>
#include <Wt/WGroupBox>
#include <Wt/WMessageBox>
#include <boost/foreach.hpp>
#include <boost/format.hpp>


#include "../model/cmd.hpp"
#include "../model/DbProxy.hpp"
#include "MyLogger.hpp"
#include "ClientBoard.hpp"
#include "MainView.hpp"

using namespace Wt;
using namespace std;

namespace {
    template<typename Key, typename Value>
      Key findMapKeyByValue(map<Key, Value> datas, Value value) {
	  for(auto i = datas.begin(); i != datas.end(); ++i) {
	      if(i->second == value)
		return i->first;
	  }
	  throw "Out of range";
      }
}


ClientBoard::ClientBoard(User& u, WContainerWidget *parrent)
: WContainerWidget(parrent),
  _db(DbProxy::getInstance()),
  _user(u),
  _table(0),
  _app(WApplication::instance()),
  _style(None)
{
  if(_user.role == User::SuperAdmin)
    _zones = _db.getZoneAllNames();
  else
    _zones = _db.getZoneNames(_user.name);

  WContainerWidget* w = new WContainerWidget(this);
  w->setPadding(0);
  w->setMargin(0);

  WVBoxLayout* layout = new WVBoxLayout();
  w->setLayout(layout);
  layout->setSpacing(0);
  layout->setContentsMargins(0,0,0,0);
  layout->addWidget(_button_box = new WContainerWidget());
  _button_box->setPadding(0);
  _button_box->setMargin(0);
  layout->addWidget(_table = new Ext::TabWidget(), AlignJustify);
  _table->setResizable(true);
  //layout->addWidget(_logger = new MyLogger());

  for(auto i=_zones.begin(); i!=_zones.end(); ++i) {
      cout << "------------\n";
    createTableView(i->first);
  }

  _table->currentChanged().connect(this, &ClientBoard::changedTab);
  _table->currentChanged().emit(_table->currentIndex());

  _con = 
    StateListener::getInstance().stateChanged()
    .connect(boost::bind(&ClientBoard::handleState, this, _1));

  _app->enableUpdates(true);
}

ClientBoard::~ClientBoard()
{
  _app->enableUpdates(false);
  _con.disconnect();
}

void ClientBoard::setStyle(Style s)
{
  if(s == _style)
    return;
  else
    _style = s;

  if(_style == Normal){
      createViewBox();
  } else if(_style == Edit) {
      createEditBox();
  }
}

void ClientBoard::handleState(const StateEvent event)
{
  WApplication::UpdateLock lock(_app);
  if (lock) {
      if (_states.find(event.id) == _states.end())
	_states.insert(make_pair(event.id, event.type));
      else if(_states[event.id] != event.type)
	_states[event.id] = event.type;
      else
	return;

      WTableView* t = static_cast<WTableView*>(_table->currentWidget());
      int zone_n = findMapKeyByValue(_views, t); 

      fetchModel(zone_n);
      _app->triggerUpdate();
  }
}


void ClientBoard::handleClick(const WModelIndex& i, const WMouseEvent& me)
{
  const WAbstractItemModel* m = i.model();
  int row = i.row();
  int id = asNumber(m->data(row, 0));
  string name  = asString(m->data(row, 1)).toUTF8();
  string ip  = asString(m->data(row, 2)).toUTF8();
  if (_style == Normal) {
      MainView dia(id, name, ip);
  } else if (_style == Edit) {
      editClient(id, name, ip);
  }
}

void ClientBoard::editClient(int id, const string& name, const string& ip)
{

  WDialog dia(WString("修改节点资料", UTF8));

  WText *w_id = new WText(WString(boost::lexical_cast<string>(id), UTF8));
  WLineEdit *w_name  = new WLineEdit(WString(name, UTF8));
  WLineEdit *w_ip  = new WLineEdit(WString(ip, UTF8));

  WVBoxLayout* vbox = new WVBoxLayout();

  WHBoxLayout* hbox = new WHBoxLayout();
  hbox->addWidget(new WText("ID"));
  hbox->addWidget(w_id, AlignJustify);
  WContainerWidget* container = new WContainerWidget();
  container->setLayout(hbox);
  vbox->addWidget(container, AlignTop);

  hbox = new WHBoxLayout();
  hbox->addWidget(new WText(WString("名称", UTF8)));
  hbox->addWidget(w_name, AlignJustify);
  WPushButton* b = new WPushButton(tr("ok"));
  b->clicked().connect(&dia, &WDialog::accept);
  hbox->addWidget(b);
  container = new WContainerWidget();
  container->setLayout(hbox);
  vbox->addWidget(container, AlignTop);

  hbox = new WHBoxLayout();
  hbox->addWidget(new WText("IP"));
  hbox->addWidget(w_ip, AlignJustify);
  b = new WPushButton(tr("cancel"));
  b->clicked().connect(&dia, &WDialog::reject);
  hbox->addWidget(b);
  container = new WContainerWidget();
  container->setLayout(hbox);
  vbox->addWidget(container, AlignTop);


  //----------------------
  WGroupBox *group = new WGroupBox(WString("区域", UTF8));
  group->setMaximumSize(WLength::Auto, 300);
  group->setOverflow(WContainerWidget::OverflowAuto);
  //group->setPadding(5);
  vector<int> zones = _db.getZonesByIP(ip);
  typedef pair<int, string> T;
  map<int, WCheckBox*> checkboxs;
  BOOST_FOREACH(const T& p, _zones){
      if(p.first != 0) {
	  string z_name = p.second;
	  WCheckBox* b = new WCheckBox(WString(z_name, UTF8), group);
	  checkboxs.insert(make_pair(p.first, b));
	  new WBreak(group);
	  if(std::find(zones.begin(), zones.end(), p.first) != zones.end())
	    b->setChecked();
      }
  }
  vbox->addWidget(group);
  //-------------------------

  dia.contents()->setLayout(vbox);

  if (dia.exec() == WDialog::Accepted) {
      Client c;
      c.id = id;
      c.name = w_name->text().toUTF8();
      c.ip = w_ip->text().toUTF8();

      string sql_update_client = 
	str(boost::format("UPDATE Clients SET NAME=\"%1%\", IP=\"%2%\" WHERE ID=%3%;")
	    % c.name % c.ip % c.id);
      string sql_delete_zone = "DELETE FROM IP2Zone WHERE IP=\"" + c.ip + "\";";
      string sql_update_zones;
      typedef pair<int, WCheckBox*> _T;
      BOOST_FOREACH(const _T& p, checkboxs) {
	  assert(p.first != 0);
	  if(p.second->isChecked()){
	      sql_update_zones += 
		str(boost::format("INSERT INTO IP2Zone VALUES(\"%1%\", %2%);")
		    % c.ip % p.first);
	  }
      }
      string sql = sql_update_client + sql_delete_zone + sql_update_zones;
      if(_db.runSQL(sql)) {
	  BOOST_FOREACH(const _T& p, checkboxs)
	    fetchModel(p.first);
	  fetchModel(0);
	  WMessageBox::show(tr("succeed"), WString("更新成功", UTF8), Ok);
      } else {
	  WMessageBox::show(tr("failed"), WString("更新失败", UTF8), Ok);
      }
  }
}

void ClientBoard::addClient()
{
  WDialog dia(WString("新建节点资料", UTF8));
  dia.contents()->setMaximumSize(400, WLength::Auto);

  WLineEdit *name  = new WLineEdit();
  WLineEdit *ip  = new WLineEdit();
  ip->setValidator(new WValidator());

  WVBoxLayout* vbox = new WVBoxLayout();

  WHBoxLayout* hbox = new WHBoxLayout();
  hbox->addWidget(new WText(WString("名称", UTF8)));
  hbox->addWidget(name, AlignJustify);
  WPushButton* b = new WPushButton(tr("ok"));
  b->clicked().connect(&dia, &WDialog::accept);
  hbox->addWidget(b);
  WContainerWidget* container = new WContainerWidget();
  container->setLayout(hbox);
  vbox->addWidget(container, AlignTop);

  hbox = new WHBoxLayout();
  hbox->addWidget(new WText(WString("IP", UTF8)));
  hbox->addWidget(ip, AlignJustify);
  b = new WPushButton(tr("cancel"));
  b->clicked().connect(&dia, &WDialog::reject);
  hbox->addWidget(b);
  container = new WContainerWidget();
  container->setLayout(hbox);
  vbox->addWidget(container, AlignTop);

  //----------------------
  WGroupBox *group = new WGroupBox(WString("区域", UTF8));
  group->setMaximumSize(WLength::Auto, 300);
  group->setOverflow(WContainerWidget::OverflowAuto);
  map<int, WCheckBox*> checkboxs;
  typedef pair<int, string> T;
  BOOST_FOREACH(const T& p, _zones){
      if(p.first != 0) {
	  string z_name = p.second;
	  WCheckBox* b = new WCheckBox(WString(z_name, UTF8), group);
	  checkboxs.insert(make_pair(p.first, b));
	  new WBreak(group);
      }
  }
  vbox->addWidget(group);
  //--------------------------
  dia.contents()->setLayout(vbox);
  if(dia.exec() == WDialog::Accepted){
      Client c;
      c.name = name->text().toUTF8();
      c.ip = ip->text().toUTF8();

      _db.addClient(c);

      string sql_update_zones;
      typedef pair<int, WCheckBox*> _T;
      BOOST_FOREACH(const _T& p, checkboxs) {
	  if(p.second->isChecked()){
	      sql_update_zones += 
		str(boost::format("INSERT INTO IP2Zone VALUES(\"%1%\", %2%);")
		    % c.ip % p.first);
	  }
      }

      if (ip->validate() && _db.runSQL(sql_update_zones)) {
	  BOOST_FOREACH(const _T& p, checkboxs) {
	      fetchModel(p.first);
	  }
	  fetchModel(0);
	  WMessageBox::show(tr("succeed"), WString("更新成功", UTF8), Ok);
      } else {
	  WMessageBox::show(tr("failed"), WString("更新失败", UTF8), Ok);
      }

  }
}

void ClientBoard::delClient()
{
  WTableView* t = static_cast<WTableView*>(_table->currentWidget());
  const WModelIndexSet& selecteds = t->selectedIndexes();

  if (!selecteds.empty() && 
      WMessageBox::show(tr("warning"),
			WString("是否要删除所选节点", UTF8), Ok|Cancel) != Ok)
    return;


  WAbstractItemModel* model = t->model();
  bool need_update = false;
  for (auto i = selecteds.begin(); i!=selecteds.end(); ++i) {
      int id = asNumber(model->data(i->row(),0));
      if(!_db.delClient(id)) {
	  string name = asString(model->data(i->row(), 1)).toUTF8();
	  WMessageBox::show(tr("failed"),
			    WString("删除[" + name + "]失败", UTF8), Ok);
      }
      need_update = true;
  }
  if(need_update)
    for(auto i = _zones.begin() ; i != _zones.end(); ++i)
      fetchModel(i->first);
}

void ClientBoard::addZone()
{
  WDialog dialog(WString("添加管理区域", UTF8));

  new WText(WString("区域名称: ",UTF8), dialog.contents());
  WLineEdit edit(dialog.contents());
  new WBreak(dialog.contents());
  WPushButton ok(tr("ok"), dialog.contents());
  WPushButton cancel(tr("cancel"), dialog.contents());

  edit.setFocus();

  edit.enterPressed().connect(&dialog, &WDialog::accept);
  ok.clicked().connect(&dialog, &WDialog::accept);
  cancel.clicked().connect(&dialog, &WDialog::reject);
  if (dialog.exec() == WDialog::Accepted) {
      string _name = edit.text().toUTF8();
      if (!_name.empty() && _db.addZone(_name)) {
	  WMessageBox::show(tr("succeed"), WString("添加成功",UTF8), Ok);
	  _zones = _db.getZoneAllNames(); //更新名称集 否则 下面的findMapKeyByValue无法找到这个名称
	  createTableView(findMapKeyByValue(_zones, _name));
      } else {
	  WMessageBox::show(tr("failed"), WString("区域名不能为空或已存在此区域",UTF8), Ok);
      }
  }
}

void ClientBoard::delZone()
{
  WTableView* t = static_cast<WTableView*>(_table->currentWidget());
  int zone_n = findMapKeyByValue(_views, t); 

  if(zone_n == 0) {
      WMessageBox::show(tr("failed"), WString("不允许删除此区域", UTF8), Ok);
      return;
  }
  if(WMessageBox::show(WString("删除管理区域", UTF8),
		       WString("您确定要删除\""+_zones[zone_n]+"\" 区域吗?\n"
			       "区域被删除后,如果区域内节点无区域管理将自动\n"
			       "添加到 无分类 区域里", UTF8),
		       Ok| Cancel)
     == Ok) {
      if(!_db.delZone(zone_n)) {
	  WMessageBox::show(tr("failed"), WString("未知错误", UTF8), Ok);
      } else {
	  _table->removeTab(_table->currentIndex());
	  fetchModel(0); //更新默认分区
      }
  }
}

void ClientBoard::modifyZone()
{
  WTableView* t = static_cast<WTableView*>(_table->currentWidget());
  int zone_n = findMapKeyByValue(_views, t); 

  WDialog dialog(WString("修改区域名", UTF8));
  new WText(WString("区域名称: ",UTF8), dialog.contents());
  WLineEdit edit(WString(_zones[zone_n], UTF8), dialog.contents());
  new WBreak(dialog.contents());
  WPushButton ok(tr("ok"), dialog.contents());
  WPushButton cancel(tr("cancel"), dialog.contents());

  edit.setFocus();

  edit.enterPressed().connect(&dialog, &WDialog::accept);
  ok.clicked().connect(&dialog, &WDialog::accept);
  cancel.clicked().connect(&dialog, &WDialog::reject);
  if (dialog.exec() == WDialog::Accepted) {
      string _name = edit.text().toUTF8();
      if (!_name.empty() && _db.modifyZone(zone_n, _name)) {
	  _table->setTabText(_table->currentIndex(),
			     WString(_name, UTF8)); //修改 tab title 名称
	  _zones[zone_n] = _name; //修改内部名称
	  WMessageBox::show(tr("succeed"), WString("修改成功", UTF8), Ok);
      } else {
	  WMessageBox::show(("failed"), WString("未知错误 修改失败",UTF8), Ok);
      }
  }
}



void ClientBoard::createTableView(int zone)
{
  cerr << "create Zone: " << zone << "in \n";
  //assert(0 && zone);
  if(_zones.find(zone) == _zones.end()) {
      //更新区域名称集
      if(_user.role == User::SuperAdmin)
	_zones = _db.getZoneAllNames();
      else
	_zones = _db.getZoneNames(_user.name);
  }

  WTableView* t = new WTableView();
  t->setObjectName("init");
  t->setSelectionMode(ExtendedSelection);
  t->doubleClicked().connect(this, &ClientBoard::handleClick);
  /*
     WScrollArea* scroll =  new WScrollArea(this);
     scroll->setVerticalScrollBarPolicy(WScrollArea::ScrollBarAsNeeded);
     scroll->setWidget(t);
     scroll->resize(WLength::Auto, 800);
     */
  t->resize(WLength::Auto, 800);

  _table->addTab(t, WString(_zones[zone], UTF8));

  _views.insert(make_pair(zone, t));

}

/*
 * 更新指定区域的数据， 如果没有此区域则建立数据
 */
WStandardItemModel* ClientBoard::fetchModel(int zone)
{
  string sql =
    "SELECT ID, Clients.IP, NAME "
    "FROM Clients, IP2Zone "
    "WHERE Clients.IP = IP2Zone.IP "
    "AND ZONE = " + boost::lexical_cast<string>(zone);
  DbData d;
  _db.runSQL(sql, d);

  vector<Client> clients;
  for (int i=d.count()-1; i>=0; --i) {
      Client c;
      c.id = d.fetch<int>(i, "ID");
      c.ip = d.fetch<string>(i, "IP");
      c.name = d.fetch<string>(i, "NAME");
      clients.push_back(c);
  }

  Wt::WStandardItemModel* m;
  if(_models.find(zone) == _models.end()) {
      m = new Wt::WStandardItemModel(this);
      _models.insert(make_pair(zone, m));
  } else {
      m = _models[zone];
  }

  m->clear();
  m->insertColumns(0, 4);
  m->setHeaderData(0, boost::any(Wt::WString::fromUTF8("ID")));
  m->setHeaderData(1, boost::any(Wt::WString::fromUTF8("名称")));
  m->setHeaderData(2, boost::any(Wt::WString::fromUTF8("IP")));
  m->setHeaderData(3, boost::any(Wt::WString::fromUTF8("状态")));
  m->insertRows(0, clients.size());
  for (int i=clients.size()-1; i>=0; --i) {
      m->setData(i, 0, clients[i].id);
      m->setData(i, 1, WString(clients[i].name, UTF8));
      m->setData(i, 2, clients[i].ip);

      int id = clients[i].id;
      if(_states.find(id) == _states.end())
	m->setData(i, 3, WString("离线", UTF8));
      else if(_states[id] == StateEvent::OnLine)
	m->setData(i, 3, WString("在线", UTF8));
      else if(_states[id] == StateEvent::OffLine)
	m->setData(i, 3, WString("离线", UTF8));
  }

  return m;
}
void ClientBoard::changedTab(int index)
{
  WTableView* t = static_cast<WTableView*>(_table->currentWidget());

  if(t->objectName() == "init") {
      int zone_n = findMapKeyByValue(_views, t); 
      t->setModel(fetchModel(zone_n));
      _app->triggerUpdate();
      t->setObjectName("");
  }
}

void ClientBoard::createEditBox ()
{
  _button_box->clear();

  WHBoxLayout *hLayout = new WHBoxLayout();
  _button_box->setLayout(hLayout, AlignTop | AlignLeft);
  hLayout->setContentsMargins(0, 0, 0, 0);

  Ext::Button *b;
  hLayout->addWidget(b = new Ext::Button(WString("添加节点", UTF8)));
  b->clicked().connect(this, &ClientBoard::addClient);

  hLayout->addWidget(b = new Ext::Button(WString("删除节点", UTF8)));
  b->clicked().connect(this, &ClientBoard::delClient);


  hLayout->addWidget(b = new Ext::Button(WString("添加区域", UTF8)));
  b->clicked().connect(this, &ClientBoard::addZone);
  b->setMargin(10, Left);

  hLayout->addWidget(b = new Ext::Button(WString("修改区域", UTF8)));
  b->clicked().connect(this, &ClientBoard::modifyZone);

  hLayout->addWidget(b = new Ext::Button(WString("删除区域", UTF8)),
		     AlignRight);
  b->clicked().connect(this, &ClientBoard::delZone);
}
void ClientBoard::createViewBox()
{
  _button_box->clear();

  WHBoxLayout *hLayout = new WHBoxLayout();
  _button_box->setLayout(hLayout, AlignTop | AlignLeft);
  hLayout->setContentsMargins(0, 0, 0, 0);

  Ext::Button *b;
  hLayout->addWidget(b = new Ext::Button(WString("发送信息", UTF8)));
  b->clicked().connect(this, &ClientBoard::sendMessage);

  hLayout->addWidget(b = new Ext::Button(WString("远程关机", UTF8)));
  b->clicked().connect(this, &ClientBoard::shutdown);

  hLayout->addWidget(_glob_action = new WCheckBox(WString("全局作用", UTF8)));
}


void ClientBoard::sendMessage()
{






  WTableView* t = static_cast<WTableView*>(_table->currentWidget());
  const WModelIndexSet& selecteds = t->selectedIndexes();

  if (_glob_action->checkState() == Checked) {
      if (WMessageBox::show(tr("warning"),
			WString("此动作将向本区域所有节点下发，继续?", UTF8),
			Ok|Cancel) == Ok) {
      } else {
	  return; //如果中断操作则直接返回
      }
  } else if(selecteds.empty()){
      WMessageBox::show(tr("warning"),
			WString("请至少选择一个节点", UTF8), Ok);
      return;
  }

  WDialog dia(WString("远程发送消息", UTF8));
  new WText(WString("消息内容:", UTF8), dia.contents());
  WLineEdit *message = new WLineEdit(dia.contents());
  message->setMinimumSize(400, WLength::Auto);
  new WBreak(dia.contents());
  WPushButton *b = new WPushButton(tr("ok"), dia.contents());
  b->clicked().connect(&dia, &WDialog::accept);
  b = new WPushButton(tr("cancel"), dia.contents());
  b->clicked().connect(&dia, &WDialog::reject);
  if (WDialog::Accepted == dia.exec()) {
  }



  WAbstractItemModel* model = t->model();
  if (_glob_action->checkState() == Checked) {
      for (int row = model->rowCount()-1; row>=0;  row--) {
	  int id = asNumber(model->data(row,0));
	  string ip = asString(model->data(row, 2)).toUTF8();
	  if(_states[id] == StateEvent::OnLine)
	    cmd_async_send(id, ip, "SHOW " + message->text().toUTF8());
      }
  } else {
      for (auto i = selecteds.begin(); i!=selecteds.end(); ++i) {
	  int id = asNumber(model->data(i->row(),0));
	  string name = asString(model->data(i->row(), 1)).toUTF8();
	  string ip = asString(model->data(i->row(), 2)).toUTF8();
	  cmd_async_send(id, ip, "SHOW " + message->text().toUTF8());
      }
  }
}
void ClientBoard::shutdown()
{

  if (_glob_action->checkState() == Checked) {
      if (WMessageBox::show(tr("warning"),
			    WString("此动作将向本区域所有节点下发，继续?", UTF8),
			    Ok|Cancel) == Ok) {
	  WTableView* t = static_cast<WTableView*>(_table->currentWidget());
	  WAbstractItemModel* model = t->model();
	  for (int row = model->rowCount()-1; row>=0;  row--) {
	      int id = asNumber(model->data(row,0));
	      string ip = asString(model->data(row, 2)).toUTF8();
	      if(_states[id] == StateEvent::OnLine)
		cmd_async_send(id, ip, "SHUTDOWN");
	  }

      } else {
	  return; //如果中断操作则直接返回
      }

      return; //如果是全局动作则 运行之后直接返回
  }

  WTableView* t = static_cast<WTableView*>(_table->currentWidget());
  const WModelIndexSet& selecteds = t->selectedIndexes();

  if(selecteds.empty()){
      WMessageBox::show(tr("warning"),
			WString("请至少选择一个节点", UTF8), Ok);
      return;
  }
  if(WMessageBox::show(tr("warning"),
		       WString("确定要关闭所选计算机?", UTF8), Ok|Cancel)
     != Ok)
    return;


  WAbstractItemModel* model = t->model();
  for (auto i = selecteds.begin(); i!=selecteds.end(); ++i) {
      int id = asNumber(model->data(i->row(),0));
      string name = asString(model->data(i->row(), 1)).toUTF8();
      string ip = asString(model->data(i->row(), 2)).toUTF8();
      cmd_async_send(id, ip, "SHUTDWON");
  }
}
