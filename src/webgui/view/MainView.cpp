#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WDialog>
#include <Wt/WFlashObject>
#include <Wt/Ext/Button>
#include <Wt/WLineEdit>
#include <Wt/WText>
#include <Wt/WBreak>
#include <Wt/WPushButton>
#include <boost/signal.hpp>

#include "MainView.hpp"

using namespace std;
using namespace Wt;

MainView::MainView (int id, const string& name, const string& ip)
	: WObject(NULL),
	_id(id),
	_ip(ip),
	_name(name),
	_app(WApplication::instance())
{
  showSelectDialog();
}

MainView::~MainView()
{
  cmd_async_send(_id, _ip, "CLOSEVNC", Cmd::nullCB);
}

void disconnect_signal(boost::signals::connection con)
{
  con.disconnect();
}

void MainView::showSelectDialog()
{
  boost::signals::connection con =
    cmd_async_send(_id, _ip, "OPENVNC CONTROL",
		   boost::bind(&MainView::onReceivePasswd, this, _1));

  WDialog dia(WString(_name, UTF8));
  new WText(WString("选择连接模式", UTF8), dia.contents());
  new WBreak(dia.contents());
  _button_view = new WPushButton(WString("远程查看", UTF8), dia.contents());
  _button_view->setDisabled(true);
  _button_control  = new WPushButton(WString("远程协助", UTF8), dia.contents());
  _button_control->setDisabled(true);

  WPushButton* b = new WPushButton(WString("取消", UTF8), dia.contents());
  b->clicked().connect(boost::bind(disconnect_signal, con));
  b->clicked().connect(&dia, &WDialog::accept);


  new WBreak(dia.contents());
  _progress = new WProgressBar(dia.contents());
  _progress->setRange(0.0, 1.0);
  _progress->setValue(0.5);
  _progress->setFormat(WString("正在获得临时密码", UTF8));
  _progress->progressCompleted().
    connect(boost::bind(&MainView::onCompleted, this));

  dia.exec();
}

void MainView::onCompleted() 
{
  if(!_passwd.empty()) {
      _button_view->clicked()
	.connect(boost::bind(&MainView::showMainDialog, this, true));
      _button_view->enable();
      _button_control->clicked()
	.connect(boost::bind(&MainView::showMainDialog, this, false));

      _button_control->setDisabled(false);
      _button_view->setDisabled(false);
  }
}

void MainView::onReceivePasswd(Response rep)
{
  cout << "On Received!\n";

  WApplication::UpdateLock lock(_app);
  if(lock) {
      if(rep.value.find("PASSWD") != rep.value.end())
	_passwd = rep.value["PASSWD"];
      if(_passwd.empty()) {
	  _progress->setFormat(WString("无法获得临时密码", UTF8));
	  _progress->setValue(1);
      } else {
	  _progress->setFormat(WString("成功获得临时密码", UTF8));
	  _progress->setValue(1);
      } 
      _app->triggerUpdate();
  } else {
      assert(0);
  }
}

void MainView::showMainDialog(bool viewonly)
{
  WDialog dia(WString(_name, UTF8));

  WVBoxLayout *vbox = new WVBoxLayout();
  vbox->setContentsMargins(0, 0, 0, 0);
  vbox->setSpacing(0);

  WHBoxLayout *hbox= new WHBoxLayout();
  hbox->setContentsMargins(0, 0, 0, 0);
  hbox->setSpacing(0);
  //hbox->addWidget(new WText(WString("消息:", UTF8)), AlignLeft);
  hbox->addWidget(_message = new WLineEdit(), AlignMiddle);
  _message->resize(WLength::Auto, 20);
  Ext::Button *b;
  hbox->addWidget(b = new Ext::Button(WString("发送消息", UTF8)));
  b->clicked().connect(this, &MainView::sendMessage);
  hbox->addWidget(b = new Ext::Button(WString("关机", UTF8)), AlignRight);
  b->clicked().connect(this, &MainView::shutdown);

  hbox->addWidget(b = new Ext::Button(WString("断开连接", UTF8)), AlignRight);
  b->clicked().connect(&dia, &WDialog::accept);

  WContainerWidget* wc = new WContainerWidget();
  wc->setLayout(hbox);
  vbox->addWidget(wc);

  WFlashObject* f = new WFlashObject("resources/Flashlight.swf");
  vbox->addWidget(f, AlignJustify);

  f->setFlashParameter("allowScriptAccess", "always");
  f->setFlashParameter("allowFullscreen", "true");
  f->setFlashParameter("wmode", "opaque");
  f->setFlashVariable("securityPort", "43204");
  f->setFlashVariable("host", _ip);
  f->setFlashVariable("port", "5900");
  f->setFlashVariable("viewOnly", viewonly ? "true": "false");
  f->setFlashVariable("password", _passwd);
  f->setFlashVariable("autoConnect", "true");
  f->setFlashVariable("hideControls", "false");
  f->setMinimumSize(1024, 700);

  dia.contents()->setLayout(vbox);
  dia.contents()->setMinimumSize(800, WLength::Auto);
  dia.exec();

}

void MainView::shutdown()
{
  string msg = _message->text().toUTF8();
  cmd_async_send(_id, _ip, "SHUTDOWN", Cmd::nullCB);
}

void MainView::sendMessage()
{
  string msg = _message->text().toUTF8();
  cmd_async_send(_id, _ip, "SHOW " + msg, Cmd::nullCB);
}

