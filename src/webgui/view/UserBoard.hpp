#ifndef __USERBOARD__
#define __USERBOARD__
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WModelIndex>
#include <Wt/WTableView>
#include "../model/DbProxy.hpp"

class UserBoard : public Wt::WContainerWidget {
public:
  UserBoard(User& u, WContainerWidget *parrent=NULL);
private:
  void manageUsers();
  void commitModifyPasswd(Wt::WLineEdit* e1, Wt::WLineEdit* e2, Wt::WLineEdit* e3);

  void handleClick(const Wt::WModelIndex& i, const Wt::WMouseEvent& me);
  void addUser();
  void delUser(Wt::WTableView* t);


private:
  User& _user;
  DbProxy& _db;
  Wt::WStandardItemModel* _model;
};

#endif
