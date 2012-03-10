#ifndef __CLEINT_HPP__
#define __CLEINT_HPP__
#include <wx/wx.h>
#include <string>
#include <set>

#include "gui.h"
#include "contianer.hpp"
#include "cmd.hpp"


class MyApp : public wxApp {
public:
  virtual bool OnInit();
};
DECLARE_APP(MyApp)


class MainFrame : public MainFrameBase {
public: 
  MainFrame(wxWindow *parent);
  void Init();
  virtual ~MainFrame();
private:
  virtual void OnCloseFrame(wxCloseEvent& event){Destroy();}
  virtual void OnExitClick(wxCommandEvent& event){Destroy();}
  virtual void m_OnImport(wxCommandEvent& event);
  virtual void m_ListColClick(wxListEvent& event);
  virtual void m_ListRightClick(wxListEvent& event);
  virtual void m_OnAddClient(wxCommandEvent& event);

  virtual void m_viewOnMenuSelection(wxCommandEvent& event);
  virtual void m_controlOnMenuSelection(wxCommandEvent& event); 
  virtual void m_shutdownOnMenuSelection(wxCommandEvent& event);
  virtual void m_messageOnMenuSelection(wxCommandEvent& event);
  virtual void m_modifyOnMenuSelection(wxCommandEvent& event);
  virtual void m_deleteOnMenuSelection(wxCommandEvent& event);

  void OnTimer(wxTimerEvent& event);
  void RunVNC(wxCommandEvent& event);
  void StopVNC(wxCommandEvent& event);
private:
  void upstate(std::string ip, std::string c_name, int id=-1);
  void handle_message(Response& r);
  void update_state(int row);

  template<typename Container> void insert_items(Container&);
  void fill_data();

  ClientContainer data;
  std::set<int> deletings; //标记需要删除的数据
  int sort_by;
  wxTimer *timer;
};
#endif
