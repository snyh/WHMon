#include <iostream>
#include <string>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WBorderLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WText>
#include <Wt/WTreeNode>
#include <Wt/WIconPair>
#include <Wt/WFitLayout>
#include <Wt/WOverlayLoadingIndicator>
#include <Wt/SyncLock>
#include <Wt/WAnchor>

#include <Wt/Ext/Container>
#include <Wt/Ext/Panel>
#include <Wt/Ext/TabWidget>

#include "WHMonWebClient.hpp"
#include "view/LoginView.hpp"
#include "view/UserBoard.hpp"
#include "view/MainView.hpp"
#include "view/ClientBoard.hpp"

DbProxy DbProxy::_instance("whmon.dat");
StateListener StateListener::_instance;

using namespace Wt;
using namespace std;

WHMonWebClient::WHMonWebClient(const WEnvironment& env)
  : WApplication(env),
  board_clients(0),
  board_managePrivate(0)
{
  if(!env.agentIsChrome() && !env.agentIsGecko()) {
      new WText(WString("当前不支持IE等浏览器 请使用Chrome 或 Firefox尝试", UTF8),
		root());
      new WBreak(root());
      new WAnchor("http://www.google.com/chrome", "Google Chrome", root());
      new WBreak(root());
      new WAnchor("http://www.mozilla.com", "Firefox", root());
      return;
  }
  setLoadingIndicator(new WOverlayLoadingIndicator());

  useStyleSheet(appRoot() + "whmon.css");
  messageResourceBundle().use(appRoot() + "whmon");
  //require(appRoot() + "resources/md5.js");

  setTitle(WString::tr("WHMonTitle"));

  Ext::Container *viewPort = new Ext::Container(root());
  WBorderLayout *layout = new WBorderLayout(viewPort);

  /* header */
  Ext::Panel *header = new Ext::Panel();
  header->setCollapsible(true);
  header->setBorder(false);
  WText *head = new WText(WString::tr("header"));
  head->setStyleClass("north");
  header->setLayout(new WFitLayout());
  header->layout()->addWidget(head);
  header->resize(WLength::Auto, 35);
  layout->addWidget(header, WBorderLayout::North);

  /* 左边 */
  Ext::Panel *west = new Ext::Panel();
  west->setTitle(WString::tr("main.controlboard"));
  west->resize(200, WLength::Auto);
  west->setResizable(true);
  west->setCollapsible(true);
  west->setAnimate(true);
  west->setAutoScrollBars(true);
  layout->addWidget(west, WBorderLayout::West);

  /* 主面板 */
  _center = new Ext::Panel();
  _center->setTitle(WString::tr("main.mainboard"));
  _center->setCollapsible(true);
  _center->setLayout(new WFitLayout());
  _center->resize(800, WLength::Auto);
  _center->collapsed().connect(this, &WHMonWebClient::lock);
  //_center->setAutoScrollBars(true);
  layout->addWidget(_center, WBorderLayout::Center);


  useStyleSheet("ext/resources/css/xtheme-gray.css");

  bool is_ok = false;
  while (!is_ok) {
      LoginView l(getUser());
      l.exec();
      if (l.isOK()) {
	is_ok = true;
	west->setTitle(WString::tr("main.controlboard") 
		       + WString("  当前用户:" + getUser().name, UTF8));
      }
  }


  _center->layout()->addWidget(createWelecome()); //显示欢迎界面
  west->layout()->addWidget(createControlTree()); //根据当前用户权限建立控制面板


  // this->enableUpdates(true);
}

void WHMonWebClient::lock()
{
  WDialog dia(WString("挂机锁", UTF8));
  new WText(WString("设置挂机密码", UTF8), dia.contents());
  WLineEdit* e = new WLineEdit(dia.contents());
  e->setEchoMode(WLineEdit::Password);
  WPushButton* b = new WPushButton(WString::tr("ok"), dia.contents());
  b->clicked().connect(&dia, &WDialog::accept);
  dia.exec();

  bool is_ok = false;
  while (!is_ok) {
      WDialog dia(WString("挂机锁", UTF8));
      new WText(WString("挂机密码", UTF8), dia.contents());
      WLineEdit* e2 = new WLineEdit(dia.contents());
      e2->setEchoMode(WLineEdit::Password);
      WPushButton* b = new WPushButton(WString::tr("ok"), dia.contents());
      b->clicked().connect(&dia, &WDialog::accept);
      dia.exec();
      if(e2->text() == e->text())
	is_ok = true;
  }
  _center->expand();
}

WHMonWebClient::~WHMonWebClient()
{
  //  this->enableUpdates(false);
}

WWidget* WHMonWebClient::createWelecome()
{
  _current_board = new WText(WString::tr("welcome"));
  return _current_board;
}

WWidget *WHMonWebClient::createControlTree()
{
  WIconPair *mapIcon = 
    new WIconPair(appRoot() + "resources/collapse.gif",
		  appRoot() + "resources/expand.gif",
		  false);
  WTreeNode *root_node = new WTreeNode("", mapIcon);
  root_node->setLoadPolicy(WTreeNode::NextLevelLoading);

  root_node->expand();
  createZoneNode(root_node);

  if(getUser().role == User::SuperAdmin || getUser().role == User::Admin)
    createClientBoard(root_node);

  createUserBoard(root_node);

  createLogout(root_node);

  return root_node;
}




void WHMonWebClient::createZoneNode(WTreeNode* rootNode)
{
  WTreeNode *n = createNode(WString::tr("main.viewbyzone"), rootNode, &WHMonWebClient::showZoneNode);
  n->displayedChildCount();
}
void WHMonWebClient::showZoneNode()
{
  if(!board_clients)
    board_clients= new ClientBoard(getUser());

  static_cast<ClientBoard*>(board_clients)->setStyle(ClientBoard::Normal);
  setMainBoard(WString::tr("main.viewbyzone"), board_clients);
}

void WHMonWebClient::createClientBoard(WTreeNode* rootNode)
{
  createNode(WString::tr("main.clientboard"), rootNode, &WHMonWebClient::showClientBoard);
}
void WHMonWebClient::showClientBoard()
{
  if(!board_clients)
    board_clients = new ClientBoard(getUser());

  static_cast<ClientBoard*>(board_clients)->setStyle(ClientBoard::Edit);
  setMainBoard(WString::tr("main.clientboard"), board_clients);
}

void WHMonWebClient::createLogout(WTreeNode* rootNode)
{
  createNode(WString::tr("main.logout"), rootNode, &WHMonWebClient::logout);
}
void WHMonWebClient::logout()
{
  this->redirect("/");
}

void WHMonWebClient::createUserBoard(WTreeNode* rootNode)
{
  createNode(WString::tr("main.userboard"), rootNode, &WHMonWebClient::showUserBoard);
}
void WHMonWebClient::showUserBoard()
{
  if(!board_managePrivate)
    delete board_managePrivate;

  board_managePrivate= new UserBoard(getUser());
  setMainBoard(WString::tr("main.userboard"), board_managePrivate);

}

WTreeNode *WHMonWebClient::createNode(const WString& label,
				      WTreeNode *parentNode,
				      ShowBoard f)
{

  WTreeNode *node = new WTreeNode(label, 0, parentNode);
  node->label()->setTextFormat(PlainText);
  if(f)
    node->label()->clicked().connect(this, f);

  return node;
}

void WHMonWebClient::setMainBoard(const WString& title, WWidget *board)
{
  assert(board != NULL);
  _center->setTitle(title);

  if(board == _current_board)
    return;

  _center->layout()->removeWidget(_current_board);
  _center->layout()->addWidget(board);

  _current_board = board;

}


WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new WHMonWebClient(env);
  return app;
}

int main(int argc, char **argv)
{
  int _argc = 9;
  char* _argv[] = {
      (char*)"webgui.wt",
      (char*)"--http-port", (char*)"8080",
      (char*)"--http-address", (char*)"0.0.0.0",
      (char*)"--docroot", (char*)".",
      (char*)"--accesslog", (char*)"out.log",
  };
  return WRun(_argc, _argv, &createApplication);
}

