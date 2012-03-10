#ifndef __MAINVIEW__
#define __MAINVIEW__

#include <Wt/WDialog>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WProgressBar>
#include <Wt/WPushButton>
#include "../model/cmd.hpp"


class MainView : public Wt::WObject{
public:
  MainView (int id, const std::string& name, 
	    const std::string& ip);
  ~MainView();
private:
  void shutdown();
  void sendMessage();

  void showSelectDialog();
  void showMainDialog(bool viewonly);
  void onReceivePasswd(Response res);
  void onCompleted();

private:
  int _id;
  std::string _ip;
  std::string _name;
  std::string _passwd;

  Wt::WPushButton* _button_view;
  Wt::WPushButton* _button_control;
  Wt::WLineEdit* _message;
  Wt::WApplication* _app;
  Wt::WProgressBar* _progress;
};


#endif
