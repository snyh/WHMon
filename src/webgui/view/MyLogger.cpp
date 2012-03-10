#include "MyLogger.hpp"

#include <Wt/WText>
#include <Wt/WBreak>
#include <Wt/WLayout>

using namespace std;
using namespace Wt;
MyLogger::MyLogger(WContainerWidget* parent)
     : Ext::Panel(parent),
     _capacity(100)
{
  this->layout()->addWidget(
  new WText("logggggggggggggggggggggggggggggggggggggggggggggggggggggggggger")
  );
  /*
  this->layout()->addWidget(
  new WBreak());

  this->layout()->addWidget(
  new WText("<b color=red>color red</b>"));
  this->layout()->addWidget(
  new WText("<b color=red>color red</b>"));
  this->layout()->addWidget(
  new WText("<b color=red>color red</b>"));
  this->layout()->addWidget(
  new WText("<b color=red>color red</b>"));
  */
}

MyLogger::MyLogger(size_t capacity, WContainerWidget* parent)
     : Ext::Panel(parent),
     _capacity(capacity)
{
  this->layout()->addWidget(
  new WText("logggggggggggggggggggggggggggggggggggggggggggggggggggggggggger")
  );
  this->layout()->addWidget(
  new WBreak());

  this->layout()->addWidget(
  new WText("<b color=red>color red</b>"));
}
