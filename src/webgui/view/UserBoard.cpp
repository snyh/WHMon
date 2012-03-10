#include <Wt/WText>
#include <Wt/WBreak>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WComboBox>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WLengthValidator>
#include <Wt/WAbstractItemModel>
#include <Wt/WGroupBox>
#include <Wt/WCheckBox>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <string>
#include <vector>
#include <map>

#include "UserBoard.hpp"

using namespace Wt;
using namespace std;
UserBoard::UserBoard(User& u, WContainerWidget *parrent)
    : WContainerWidget(parrent),
     _user(u),
     _db(DbProxy::getInstance())
{
  new WText(WString("修改密码", UTF8), this);
  new WBreak(this);
  new WText(WString("原始密码:", UTF8), this);
  WLineEdit* e1 = new WLineEdit(this);
  new WText(WString("新密码:", UTF8), this);
  WLineEdit* e2 = new WLineEdit(this);
  new WText(WString("验证密码:", UTF8), this);
  WLineEdit* e3 = new WLineEdit(this);

  e1->setEchoMode(WLineEdit::Password);
  e1->setValidator(new WLengthValidator(5, 20));
  e2->setEchoMode(WLineEdit::Password);
  e2->setValidator(new WLengthValidator(5, 20));
  e3->setEchoMode(WLineEdit::Password);
  e3->setValidator(new WLengthValidator(5, 20));
  WPushButton* commit = new WPushButton(WString("提交", UTF8), this);
  commit->clicked().connect(boost::bind(&UserBoard::commitModifyPasswd,
				   this, e1, e2, e3));
  if(_user.role == User::SuperAdmin)
    manageUsers();
}

void UserBoard::manageUsers()
{
  DbData d;
  _db.runSQL("SELECT * FROM Users", d);
  _model = new WStandardItemModel();
  _model->insertColumns(0, 2);
  _model->setHeaderData(0, boost::any(WString::fromUTF8("用户")));
  _model->setHeaderData(1, boost::any(WString::fromUTF8("权限")));
  _model->insertRows(0, d.count());
  for (int i=d.count()-1; i>=0; --i) {
      _model->setData(i, 0, d.fetch<string>(i, "NAME"));

      User::ROLE r = User::ROLE(d.fetch<int>(i, "ROLE"));
      if (r == User::SuperAdmin)
	_model->setData(i, 1, string("超级管理员"));
      else if (r == User::Admin)
	_model->setData(i, 1, string("管理员"));
      else if (r == User::Visitor)
	_model->setData(i, 1, string("普通权限"));
      else
	_model->setData(i, 1, string("内部错误请设置权限"));

      _model->setData(i, 0, int(r), UserRole);
  }

  WTableView* t = new WTableView(this);
  t->setSelectionMode(SingleSelection);
  t->doubleClicked().connect(this, &UserBoard::handleClick);
  t->setModel(_model);
  t->resize(400, 500);

  WPushButton* add_user = new WPushButton(WString("添加用户", UTF8), this);
  add_user->clicked().connect(boost::bind(&UserBoard::addUser, this)); 
  WPushButton* del_user = new WPushButton(WString("删除当前所选用户", UTF8), this);
  del_user->clicked().connect(boost::bind(&UserBoard::delUser, this, t)); 
}

void UserBoard::addUser()
{
  WDialog dia(WString("添加用户", UTF8));

  WVBoxLayout* vbox = new WVBoxLayout();

  WHBoxLayout* hbox = new WHBoxLayout();
  hbox->addWidget(new WText(WString("用户", UTF8)));
  WLineEdit* e_user = new WLineEdit();
  e_user->setValidator(new WLengthValidator(5, 20));
  hbox->addWidget(e_user);
  WPushButton* b = new WPushButton(tr("ok"));
  b->clicked().connect(&dia, &WDialog::accept);
  hbox->addWidget(b);
  WContainerWidget* container = new WContainerWidget();
  container->setLayout(hbox);
  vbox->addWidget(container, AlignTop);

  hbox = new WHBoxLayout();
  hbox->addWidget(new WText(WString("密码", UTF8)));
  WLineEdit* e_passwd = new WLineEdit();
  e_passwd->setValidator(new WLengthValidator(5, 20));
  hbox->addWidget(e_passwd);
  b = new WPushButton(tr("cancel"));
  b->clicked().connect(&dia, &WDialog::reject);
  hbox->addWidget(b);
  container = new WContainerWidget();
  container->setLayout(hbox);
  vbox->addWidget(container, AlignTop);

  dia.contents()->setLayout(vbox);

  if (dia.exec() == WDialog::Accepted) {
      User u;
      u.name = e_user->text().toUTF8();
      u.passwd = e_passwd->text().toUTF8();
      u.role = User::Visitor;
      if(_db.addUser(u)) {
	  _model->insertRow(0);
	  _model->setData(0, 0, u.name);
	  _model->setData(0, 1, string("普通权限"));
	  _model->setData(0, 0, int(User::Visitor), UserRole);
	  WMessageBox::show(tr("succeed"), WString("添加成功 请编辑用户权限", UTF8), Ok);
      } else {
	  WMessageBox::show(tr("warning"), WString("添加失败 写数据库错误", UTF8), Ok);
      }
  }
}
void UserBoard::delUser(WTableView* t)
{
  const WModelIndexSet& selecteds = t->selectedIndexes();
  if (selecteds.empty()) return;

  for (auto i = selecteds.begin(); i!=selecteds.end(); ++i) {
      string user = asString(_model->data(i->row(),0)).toUTF8();
      if(!_db.delUser(user)) {
	  string name = asString(_model->data(i->row(), 1)).toUTF8();
	  WMessageBox::show(tr("failed"),
			    WString("删除[" + name + "]失败", UTF8), Ok);
      } else {
	  _model->removeRow(i->row());
      }
  }
}


void UserBoard::handleClick(const WModelIndex& __i, const WMouseEvent& me)
{
  int row = __i.row();


  string user = asString(_model->data(row, 0)).toUTF8();
  User::ROLE role = User::ROLE(asNumber(_model->data(row, 0, UserRole)));

  WDialog dia(WString("用户权限设定", UTF8));

  WVBoxLayout* vbox = new WVBoxLayout();

  WHBoxLayout* hbox = new WHBoxLayout();
  hbox->addWidget(new WText(WString("用户", UTF8)));
  hbox->addWidget(new WText(WString(user, UTF8)), AlignJustify);
  WPushButton* b = new WPushButton(tr("ok"));
  b->clicked().connect(&dia, &WDialog::accept);
  hbox->addWidget(b);
  WContainerWidget* container = new WContainerWidget();
  container->setLayout(hbox);
  vbox->addWidget(container, AlignTop);

  hbox = new WHBoxLayout();
  hbox->addWidget(new WText(WString("权限", UTF8)));
  WComboBox* com_role = new WComboBox();
  com_role->addItem(WString("超级管理员", UTF8));
  com_role->addItem(WString("管理员", UTF8));
  com_role->addItem(WString("普通用户", UTF8));
  com_role->setCurrentIndex(role);
  hbox->addWidget(com_role, AlignJustify);
  b = new WPushButton(tr("cancel"));
  b->clicked().connect(&dia, &WDialog::reject);
  hbox->addWidget(b);
  container = new WContainerWidget();
  container->setLayout(hbox);
  vbox->addWidget(container, AlignTop);

  WGroupBox *group = new WGroupBox(WString("可管理区域", UTF8));
  group->setMaximumSize(WLength::Auto, 300);
  group->setOverflow(WContainerWidget::OverflowAuto);
  //group->setPadding(5);
  vector<int> u_zones = _db.getZonesByUser(user);
  map<int, string> all_zones = _db.getZoneAllNames();

  map<int, WCheckBox*> checkboxs;
  for (auto i=all_zones.begin(); i!=all_zones.end(); ++i) {
      if (i->first !=0 ) {
	  string z_name = i->second;
	  WCheckBox* b = new WCheckBox(WString(z_name, UTF8), group);
	  checkboxs.insert(make_pair(i->first, b));
	  new WBreak(group);
	  if(std::find(u_zones.begin(), u_zones.end(), i->first) != u_zones.end())
	    b->setChecked();
      }
  }
  vbox->addWidget(group);

  dia.contents()->setLayout(vbox);

  if (dia.exec() == WDialog::Accepted) {
      string sql =
	str(boost::format("UPDATE Users SET ROLE=%1% WHERE NAME=\"%2%\";") % com_role->currentIndex() % user);
      sql += "DELETE FROM User2Zones WHERE USER=\"" + user + "\";";
      for (auto i=checkboxs.begin(); i!=checkboxs.end(); ++i) {
	  if(User::ROLE(com_role->currentIndex()) == User::SuperAdmin) {
	      sql +=
		str(boost::format("INSERT INTO User2Zones VALUES(\"%1%\", %2%);")
		    % user % i->first);
	  } else if (i->second->isChecked()) {
	      sql +=
		str(boost::format("INSERT INTO User2Zones VALUES(\"%1%\", %2%);")
		    % user % i->first);
	  }
      }
      if(User::ROLE(com_role->currentIndex()) == User::SuperAdmin) {
	  sql +=
	    str(boost::format("INSERT INTO User2Zones VALUES(\"%1%\", %2%);")
		% user % 0);
      }
      if (_db.runSQL(sql)) {
	  WMessageBox::show(tr("succeed"), WString("更新成功 下次登录时生效", UTF8), Ok);
	  _model->setData(row, 1, com_role->currentText());
      } else {
	  WMessageBox::show(tr("failed"), WString("写数据库 更新失败", UTF8), Ok);
      }
  }
}

void UserBoard::commitModifyPasswd(WLineEdit* e1, 
				   WLineEdit* e2,
				   WLineEdit* e3)
{
  if (e1->validate() != WValidator::Valid ||
      e2->validate() != WValidator::Valid ||
      e3->validate() != WValidator::Valid)
    return;
  if (e1->text().toUTF8() != _user.passwd) {
      WMessageBox::show(tr("warning"), 
			WString("原始密码不匹配", UTF8),
			Ok);
      return;
  }
  if (e2->text() != e3->text()) {
      WMessageBox::show(tr("warning"), 
			WString("两次新密码不匹配", UTF8),
			Ok);
      return;
  }
  User u = _user;
  u.passwd = e3->text().toUTF8();
  if (_db.modifyUser(u)) {
      _user = u;
      WMessageBox::show(tr("succeed"), 
			WString("密码修改成功", UTF8),
			Ok);
  } else {
      WMessageBox::show(tr("warning"), 
			WString("密码修改失败 未知错误", UTF8),
			Ok);
  }
}
