#ifndef __MYLOGGER__
#define __MYLOGGER__

#include <map>
#include <string>
#include <Wt/Ext/Panel>
#include <Wt/WTextArea>

class MyLogger : public Wt::Ext::Panel {
public:
  typedef std::pair<std::string, std::string> MessageType;
  MyLogger(Wt::WContainerWidget* parent=NULL);
  MyLogger(size_t capacity, Wt::WContainerWidget* parent=NULL);
private:
  size_t _capacity;
  Wt::WTextArea* _text_area;
  std::map<int, MessageType> _log;
};

#endif
