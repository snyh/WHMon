// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef __WHMONWEBCLIENT__
#define __WHMONWEBCLIENT__

#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include "model/DbProxy.hpp"

class MyLogger;
namespace Wt {
    class WWidget;
    namespace Ext {
	class Panel;
    }
}

class WHMonWebClient : public Wt::WApplication
{
public:
  WHMonWebClient(const Wt::WEnvironment& env);
  User& getUser() { return _user; }
  ~WHMonWebClient();

private:
  Wt::WWidget *createControlTree();

  typedef void (WHMonWebClient::*ShowBoard)();
  Wt::WTreeNode *createNode(const Wt::WString& label,
			    Wt::WTreeNode *parentNode,
			    ShowBoard f);
  void setMainBoard(const Wt::WString& title, Wt::WWidget *board);

  Wt::WWidget* createWelecome();

  void lock();

private:
  void createZoneNode(Wt::WTreeNode*); //按区域游览节点
  void showZoneNode();

  void createUserBoard(Wt::WTreeNode*);
  void showUserBoard();

  void createViewAll(Wt::WTreeNode*);
  void showViewAll();

  void createClientBoard(Wt::WTreeNode*);
  void showClientBoard();

  void createLogout(Wt::WTreeNode*);
  void logout();

private:
  User _user;
  Wt::WWidget* _current_board;
  Wt::Ext::Panel* _center;

  Wt::WWidget* board_clients;
  Wt::WWidget* board_managePrivate;

         // ID, client._state_count
  std::map<int, Wt::WModelIndex> _states;
  std::map<int, Wt::WStandardItemModel*> _modles;
};

#endif 
