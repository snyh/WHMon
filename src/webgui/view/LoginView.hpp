// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef __LOGIN_H_
#define __LOGIN_H_

#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WTemplate>
#include <Wt/WDialog>

#include "../model/DbProxy.hpp"


class LoginView : public Wt::WDialog
{
public:
  LoginView(User& u);
  bool isOK() const { return _isOK; }

private:

  bool isFirstLogin();
  void createLayout();
  void createChangePasswd();

  void loginCheck();
  void loginCheck2(const Wt::WMouseEvent& me);
  void changeDefaultPasswd();
  void changeDefaultPasswd2(const Wt::WMouseEvent& me);
private:
  Wt::WLineEdit* _user_name;
  Wt::WLineEdit* _user_passwd;
  Wt::WPushButton* _login_button;

  Wt::WTemplate* _changed_passwd;
  bool _isOK;
  User& _user;
};

#endif // LOGIN_H_
