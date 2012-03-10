#include "gui.h"
#include "client.hpp"
#include "contianer.hpp"
#include "cmd.hpp"

#include <wx/wx.h>
#include <wx/log.h>

#include <cstdlib>
#include <fstream>
#include <iostream>

#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#define BOOST_THREAD_USE_LIB
#include <boost/thread.hpp>
using namespace std;
using namespace boost;
static wxString str_state[3] = {
    _T("未知"),
    _T("已连接"),
    _T("未连接"),
};
static const wxEventType EVT_RUNVNC = wxNewEventType();
static const wxEventType EVT_STOPVNC = wxNewEventType();
unsigned int Client::next_id = 0;
static boost::mutex update_mutex;
static boost::mutex cmd_mutex;

void send_cmd(int id, string ip, string data, CallBack fun);

inline void logmessage(string s)
{
  wxLogMessage(wxString(s.c_str(), wxConvUTF8));
}


bool MyApp::OnInit() 
{
  MainFrame *m = new MainFrame(NULL);
  m->Init();
  m->Show();
  return true;
}

IMPLEMENT_APP(MyApp)


MainFrame::MainFrame(wxWindow *parent) 
: MainFrameBase(parent), sort_by(1)
{
  wxLog::SetActiveTarget(new wxLogTextCtrl(m_logger));

  try {
      _Load(data);
  } catch (std::exception& e){
      wxLogWarning(_T("Load data error"));
  }
  m_listctrl->InsertColumn(0, _T("教室"));
  m_listctrl->InsertColumn(1, _T("IP"));
  m_listctrl->InsertColumn(2, _T("状态"));
  m_listctrl->InsertColumn(3, _T("ID"));
  m_listctrl->SetColumnWidth(0, 80);
  m_listctrl->SetColumnWidth(1, 100);
  m_listctrl->SetColumnWidth(2, 80);

  //定时器， 每1秒减少client的count 直到0为止。
  timer = new wxTimer(this);
  this->Connect(timer->GetId(), wxEVT_TIMER, wxTimerEventHandler(MainFrame::OnTimer));
  this->Connect(GetId(), EVT_RUNVNC, wxCommandEventHandler(MainFrame::RunVNC));
  this->Connect(GetId(), EVT_STOPVNC, wxCommandEventHandler(MainFrame::StopVNC));

}
void MainFrame::Init()
{
  fill_data();
  timer->Start(1000);

  //监听udp 43021 接受端点传送来的发现包和者心跳包
  void update(boost::function<void(string,string,int)>);
  boost::function<void(string,string, int)> fun = boost::bind(&MainFrame::upstate, this, _1, _2, _3);
  boost::thread t(boost::bind(update, fun));
  t.detach();


  //发送hello 包
  for(int i=0; i<data.size(); i++){
      static boost::function<void(Response&)> cb;
      boost::thread t(boost::bind(send_cmd, i, data[i].ip, "HELLO", cb));
      t.detach();
  }
}

MainFrame::~MainFrame()
{
  try {
      //删除需要删除的条目
      BOOST_FOREACH(int i, deletings){
	  data.erase(data.iterator_to(data[i]));
      }
      _Save(data);
  } catch (std::exception &e) {
      wxLogMessage(_T("Save data error"));
  }
  this->Show(false);

  for(int i=0; i<data.size(); i++){
      if(data[i].count > 0){
	  static boost::function<void(Response&)> cb;
	  boost::thread t(boost::bind(send_cmd, i, data[i].ip, "BYE", cb));
	  t.join();
      }
  }
}

void MainFrame::m_OnImport(wxCommandEvent& event)
{
  wxString file = wxFileSelector(_T("选择将要导入的数据文件"));
  if(!file.empty()){
      std::ifstream is("data");
      try {
	  data.push_back(Client::Create("11-12", "192.168.1.1"));
	  fill_data();
      } catch (std::exception &e) {
	  wxLogMessage(_T("Import clients error"));
      }
  }

}
void MainFrame::m_ListColClick(wxListEvent& event)
{
  //列是从0开始计数 sort_by 是以multi_index中的1开始
  if(sort_by != event.GetColumn() + 1){
      sort_by = event.GetColumn() + 1;
      fill_data();
  }
}

void MainFrame::m_ListRightClick(wxListEvent& event)
{

  //如果有标记为删除的则禁用除"恢复"外的其他选项
  long i=-1;
  for(;;){
      i = m_listctrl->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if(i==-1)break;
      int id = m_listctrl->GetItemData(i);

      if(deletings.find(id) != deletings.end()) {
	  action_menu->FindItemByPosition(0)->Enable(false);
	  action_menu->FindItemByPosition(1)->Enable(false);
	  action_menu->FindItemByPosition(2)->Enable(false);
	  action_menu->FindItemByPosition(3)->Enable(false);
	  action_menu->FindItemByPosition(4)->Enable(false);
	  action_menu->FindItemByPosition(5)->Enable();
	  this->PopupMenu(action_menu);
	  return;
      }
  }


  //多选时禁用远程控制 以及修改选项
  if(m_listctrl->GetSelectedItemCount() > 1){
      action_menu->FindItemByPosition(0)->Enable(false);
      action_menu->FindItemByPosition(1)->Enable(false);
      action_menu->FindItemByPosition(2)->Enable();
      action_menu->FindItemByPosition(3)->Enable();
      action_menu->FindItemByPosition(4)->Enable(false);
      action_menu->FindItemByPosition(5)->Enable();
  } else {
      action_menu->FindItemByPosition(0)->Enable();
      action_menu->FindItemByPosition(1)->Enable();
      action_menu->FindItemByPosition(2)->Enable();
      action_menu->FindItemByPosition(3)->Enable();
      action_menu->FindItemByPosition(4)->Enable();
      action_menu->FindItemByPosition(5)->Enable();
  }
  this->PopupMenu(action_menu);
}

void MainFrame::fill_data()
{
  wxLogDebug(_T("fill_data() by: %d"), sort_by);
  switch (sort_by) {
    case 1: {
		ClientContainer::nth_index<1>::type& name_i = data.get<1>();
		insert_items(name_i);
		break;
	    }
    case 2: {
		ClientContainer::nth_index<2>::type& ip_i = data.get<2>();
		insert_items(ip_i);
		break;
	    }
    case 3: {
		ClientContainer::nth_index<3>::type& state_i = data.get<3>();
		insert_items(state_i);
		break;
	    }
  }
}
template<typename Container>
void MainFrame::insert_items(Container& cc)
{
  while(int(data.size()) > m_listctrl->GetItemCount())
    m_listctrl->InsertItem(0, _T(""));
  int i=0;
  BOOST_FOREACH(const Client& c, cc){
      assert(i >= 0 && i < m_listctrl->GetItemCount());
      m_listctrl->SetItem(i, 0, wxString(c.name.c_str(), wxConvLocal));
      m_listctrl->SetItem(i, 1, wxString(c.ip.c_str(), wxConvLocal));
      m_listctrl->SetItem(i, 2, str_state[c.c_state]);
      m_listctrl->SetItem(i, 3, wxString::Format(_T("%d"), c.id));
      m_listctrl->SetItemData(i, c.id);
      if(deletings.find(c.id) != deletings.end()){
	  //如果已经标记为删除修改背景色
	  m_listctrl->SetItemTextColour(i, *wxRED);
      } else {
	  m_listctrl->SetItemTextColour(i, *wxBLACK);
      }
      i++;
  }
}
void MainFrame::m_OnAddClient(wxCommandEvent& event)
{
  AddDialog dialog(NULL, wxID_ANY);
  if(wxID_OK == dialog.ShowModal()){
      data.push_back(Client::Create(string(dialog.m_name->GetValue().mb_str()),
				    string(dialog.m_ip->GetValue().mb_str())));
      fill_data();
  }
}

void MainFrame::m_viewOnMenuSelection( wxCommandEvent& event ) 
{ 
  long i = m_listctrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if(i==-1)return;

  int id = m_listctrl->GetItemData(i);

  logmessage(str(boost::format("请求远端%1%开启VNC") % data[id].name));

  static boost::function<void(Response&)> cb = boost::bind(&MainFrame::handle_message, this, _1);
  boost::thread t(boost::bind(send_cmd, id, data[id].ip, "OPENVNC VIEW", cb));
  t.detach();
}
void MainFrame::m_controlOnMenuSelection( wxCommandEvent& event ) 
{
  long i = m_listctrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if(i==-1)return;

  int id = m_listctrl->GetItemData(i);
  logmessage(str(boost::format("请求远端%1%开启VNC") % data[id].name));

  static boost::function<void(Response&)> cb = boost::bind(&MainFrame::handle_message, this, _1);
  boost::thread t(boost::bind(send_cmd, id, data[id].ip, "OPENVNC CONTROL", cb));
  t.detach();
  wxLogDebug(_T("control id: %d"), id);
}
void MainFrame::m_shutdownOnMenuSelection( wxCommandEvent& event ) 
{
  long i=-1;
  for(;;){
      i = m_listctrl->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if(i==-1)break;
      int id = m_listctrl->GetItemData(i);
      logmessage(str(boost::format("发送关机命令到%1%") % data[id].name));

      static boost::function<void(Response&)> cb = boost::bind(&MainFrame::handle_message, this, _1);
      boost::thread t(boost::bind(send_cmd, id, data[id].ip, "SHUTDOWN", cb));
      t.detach();
  }
}
void MainFrame::m_messageOnMenuSelection( wxCommandEvent& event ) 
{ 
  long i=-1;
  wxString m = wxGetTextFromUser(_T("要发送的信息"));
  if(m.empty())return;
  for(;;){
      i = m_listctrl->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if(i==-1)break;
      int id = m_listctrl->GetItemData(i);
      logmessage(str(boost::format("发送消息到%1%(%2%)") % data[id].name
		     % id));

      static boost::function<void(Response&)> cb = boost::bind(&MainFrame::handle_message, this, _1);
      boost::thread t(boost::bind(send_cmd, id, data[id].ip, 
				  "SHOW " + string(m.mb_str()),
				  cb));
      t.detach();
  }
}

void MainFrame::m_modifyOnMenuSelection(wxCommandEvent& event)
{
  long i = m_listctrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if(i==-1)return;
  int id = m_listctrl->GetItemData(i);

  AddDialog dialog(NULL, wxID_ANY);

  dialog.m_name->SetValue(wxString(data[id].name.c_str(), wxConvLocal));
  dialog.m_ip->SetValue(wxString(data[id].ip.c_str(), wxConvLocal));

  if(wxID_OK == dialog.ShowModal()){
      Client m(data[id]);
      m.name = string(dialog.m_name->GetValue().mb_str());
      m.ip = string(dialog.m_ip->GetValue().mb_str());
      data.replace(data.iterator_to(data[id]), m);
      fill_data();
  }
}
void MainFrame::m_deleteOnMenuSelection(wxCommandEvent& event)
{
  long i=-1;
  for(;;){
      i = m_listctrl->GetNextItem(i, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if(i==-1)break;
      int id = m_listctrl->GetItemData(i);
      if(deletings.find(id) != deletings.end()){
	  deletings.erase(id);
      } else {
	  deletings.insert(id);
      }
  }
  fill_data();
}

void MainFrame::handle_message(Response& r)
{
  boost::mutex::scoped_lock lock(cmd_mutex);
  wxMutexGuiEnter();
  if(r.id < 0 || r.id >= data.size())
    wxLogFatalError(_T("handle_message() id 范围错误"));

  if (r.code == -1){
      logmessage(str(boost::format("%1% 错误: %2%") % data[r.id].name % r.descript));
  } else if(r.code == 0) {
      if(r.value["CMD"] == "OPENVNC"){
	  wxCommandEvent evt(EVT_RUNVNC, GetId());
	  evt.SetClientData((void*)&r);
	  evt.SetInt(r.id);
	  evt.SetString(wxString(r.value["PASSWD"].c_str(), wxConvLocal));
	  int mode = (r.value["MODE"] == "VIEW");
	  evt.SetExtraLong(mode);
	  wxPostEvent(this, evt);
      } else if(r.value["CMD"] == "CLOSEVNC"){
	  logmessage(str(boost::format("%1% 关闭VNC OK") % data[r.id].name));
      } else if(r.value["CMD"] == "SHOW"){
	  logmessage(str(boost::format("%1% 显示消息 OK") % data[r.id].name));
      } else if(r.value["CMD"] == "TOPROJECT") {
	  //wxLogMessage(_T("%s 发送投影仪代码 OK"), wxString(data[r.id].name.c_str(), wxConvLocal));
      } else if(r.value["CMD"] == "SHUTDOWN") {
	  logmessage(str(boost::format("%1% 关闭计算机 OK") % data[r.id].name));
      }
  } else {
      wxLogFatalError(_T("handle_message() return code  范围错误"));
  }
  wxMutexGuiLeave();
}

void MainFrame::upstate(string ip, string name, int id)
{
  boost::mutex::scoped_lock lock(update_mutex);
  // upstate 只在线程update for中使用 如果在其他地方使用必须锁住

  static boost::function<void(Response&)> cb_null;
  //id == -1 时 发现新端点
  //id > 0 < data.size() 时 为data[id]的心跳包
  //id >0  && id >= data.size() 时异常情况
  if(id == -1) {
      bool find = false;
      BOOST_FOREACH(const Client& c, data){
	  if(c.ip == ip){
	      find = true;
	      id = c.id; //小心修改了参数, 下面的send_cmd hello 需要使用id
	      break;
	  }
      }
      if(!find) {
	  wxMutexGuiEnter();
	  logmessage(str(boost::format("new endpoint %1% founded!") % name));
	  data.push_back(Client::Create(name, ip, 33));
	  fill_data();
	  wxMutexGuiLeave();
      }
      //发送hello cmd
      boost::thread t(boost::bind(send_cmd, id, ip, "HELLO", cb_null));
      t.detach();
  } else if(id>=0) {
      if(id >= data.size()){ //client.exe发送hello后意外结束且data.db中无此id的记录。 此时应该向id发送bye包使其重新发送 发现包.
	  boost::thread t(boost::bind(send_cmd, id, ip, "BYE", cb_null));
	  t.detach();
	  return;
      }



      //更新client.count
      assert(id < data.size());
      Client m(data[id]);
      //cout << "id: " << id << "  m.count" << m.count << "\n";
      if(m.count == 0){ //如果之前client的状态为SC_OFFLINE更新界面
	  m.count = 10;
	  m.c_state = SC_ONLINE;
	  data.replace(data.iterator_to(data[id]), m);
	  update_state(m.id);
      } else {
	  m.count = 10;
	  m.c_state = SC_ONLINE;
	  data.replace(data.iterator_to(data[id]), m);
      }
  }
}

void send_cmd(int id, string ip, string data, CallBack fun)
{
  Cmd cmd(id, ip, data, fun);
  cmd.send();
}

void MainFrame::OnTimer(wxTimerEvent& event)
{
  BOOST_FOREACH(const Client& c, data){
      Client m(c);
      if(m.count>0){
	  m.count--;
	  if(m.count == 0){
	      m.c_state = SC_OFFLINE;
	      data.replace(data.iterator_to(c), m);
	      update_state(m.id); //状态更新
	      logmessage(str(boost::format("与%1% 失去连接") % m.name));
	  } else {
	      data.replace(data.iterator_to(c), m);
	  }
      }
  }
}
void MainFrame::update_state(int id)
{
  //条目的id 和条目的位置并非总是相同
  //只能通过 GetItemData 查找条目位置。
  //由于update_state()函数调用频度比较小
  //因此没有添加其他辅助结构提高效率
  assert(id >= 0 && id < m_listctrl->GetItemCount());

  for(int row=0; row < m_listctrl->GetItemCount(); row++){
      if(id == m_listctrl->GetItemData(row)){
	m_listctrl->SetItem(row, 2, str_state[data[id].c_state]);
	break;
      }
  }
}

void MainFrame::RunVNC(wxCommandEvent& event)
{
  void run_vnc(string passwd, string ip, bool viewonly, int port=5900);
  this->Show(false);
  run_vnc(string(event.GetString().mb_str()), 
	  data[event.GetInt()].ip,
	  bool(event.GetExtraLong()));
  this->Show(true);

  wxCommandEvent evt(EVT_STOPVNC, GetId());
  evt.SetInt(event.GetInt());
  wxPostEvent(this, evt);
}
void MainFrame::StopVNC(wxCommandEvent& event)
{
  int id = event.GetInt();
  this->Show(true);
  static boost::function<void(Response&)> cb;
  boost::thread t(boost::bind(send_cmd, id, data[id].ip, "CLOSEVNC", cb));
  t.detach();
}
