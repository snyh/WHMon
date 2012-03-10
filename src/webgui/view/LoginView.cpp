#include <Wt/WApplication>
#include <Wt/WLabel>
#include <Wt/WBreak>
#include <Wt/WLengthValidator>
#include <Wt/WMessageBox>
#include <Wt/WTemplate>
#include <Wt/WDialog>
#include <string>
#include <boost/bind.hpp>

#include "LoginView.hpp"
#include "MainView.hpp"
#include "../WHMonWebClient.hpp"

using namespace std;
using namespace Wt;

LoginView::LoginView(User& u)
     : WDialog(WString::tr("login.title")),
      _user(u)
{
  if(!isFirstLogin()) {
      createLayout();
      resize(200, WLength::Auto);
  } else
    createChangePasswd();
  setMargin(10);
}

void LoginView::createChangePasswd()
{
  _changed_passwd = new WTemplate(contents());
  _changed_passwd->setTemplateText(WString::tr("login.first"));
  WLineEdit* _passwd = new WLineEdit(contents());
  _passwd->setValidator(new WLengthValidator(5, 120));
  _passwd->setEchoMode(WLineEdit::Password);
  WLineEdit* _verify = new WLineEdit(contents());
  _verify->setValidator(new WLengthValidator(5, 120));
  _verify->setEchoMode(WLineEdit::Password);
  _verify->enterPressed().connect(this, &LoginView::changeDefaultPasswd);
  WPushButton* _ok = new WPushButton("OK", contents());
  _ok->clicked().connect(this, &LoginView::changeDefaultPasswd2);
  _changed_passwd->bindWidget("passwd", _passwd);
  _changed_passwd->bindWidget("verify", _verify);
  _changed_passwd->bindWidget("OK", _ok);
}

bool LoginView::isFirstLogin()
{
  User u = DbProxy::getInstance().getUser("admin");
  if (u.passwd == "admin") 
    return true;
  else
    return false;

}
void LoginView::createLayout()
{
  WLabel* userNameL = new WLabel(tr("login.userName"), contents());
  userNameL->setMargin(10, Left | Right);
  _user_name = new WLineEdit(contents());
  _user_name->setValidator(new WLengthValidator(5, 20, contents()));
  userNameL->setBuddy(_user_name);
  _user_name->setFocus();

  new WBreak(contents());

  WLabel* passwdL = new WLabel(tr("login.userPasswd"), contents());
  passwdL->setMargin(10, Left | Right);
  _user_passwd =  new WLineEdit(contents());
  passwdL->setBuddy(_user_passwd);
  _user_passwd->setValidator(new WLengthValidator(5, 20, contents()));
  _user_passwd->setEchoMode(WLineEdit::Password);
  _user_passwd->enterPressed().connect(this, &LoginView::loginCheck);

  new WBreak(contents());

  _login_button = new WPushButton(tr("login.loginButton"),
				  contents());
  //_login_button->clicked().connect(_user_passwd->jsRef() + ".value = " + "hex_md5(" + _user_passwd->jsRef() + ");");
  _login_button->clicked().connect(this, &LoginView::loginCheck2);
  //_user_passwd->enterPressed().connect(_user_passwd->jsRef() + ".value = " + "hex_md5(" + _user_passwd->jsRef() + ");");

}
void LoginView::loginCheck2(const WMouseEvent& me)
{
  loginCheck();
}
void LoginView::loginCheck()
{
  if (_user_name->validate() != WValidator::Valid ||
      _user_passwd->validate() != WValidator::Valid)
    return;
  //TODO 对密码进行加密传输

  //服务器端 验证密码
  string user = _user_name->text().toUTF8();
  string passwd = _user_passwd->text().toUTF8();
  User u = DbProxy::getInstance().getUser(user);
  if (!user.empty() && passwd == u.passwd) {
      _user = u;
      _isOK = true;
      this->accept();
  } else {
      _isOK = false;
      WMessageBox::show(WString::tr("Failed"), 
			WString::tr("login.failed"), Ok);
      _user_name->setFocus();
  }
}

void LoginView::changeDefaultPasswd()
{
  WLineEdit* _passwd = _changed_passwd->resolve<WLineEdit*>("passwd");
  WLineEdit* _verify = _changed_passwd->resolve<WLineEdit*>("verify");
  if (_passwd->validate() != WValidator::Valid ||
      _verify->validate() != WValidator::Valid)
    return;
  //TODO 对密码进行加密传输
  //doJavaScript(user_passwd->jsRef() + ".value = " + "hex_md5(" + user_passwd->jsRef() + ");");

  //服务器端 验证密码
  string passwd = _passwd->text().toUTF8();
  string verify = _verify->text().toUTF8();
  if (passwd != verify) {
      WMessageBox::show(tr("Failed"), tr("login.verifyFailed"), Ok);
  } else {
      User u = DbProxy::getInstance().getUser("admin");
      u.passwd = passwd;
      DbProxy::getInstance().modifyUser(u);
      this->accept();
  }
  _isOK = false; //更改密码后 需要重新登录
}
void LoginView::changeDefaultPasswd2(const WMouseEvent& me)
{
  changeDefaultPasswd();
}

