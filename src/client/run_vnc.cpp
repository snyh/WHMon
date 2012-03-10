#include <string>
#include <wx/wx.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <iterator>
#include <iomanip>
#include <stdio.h>
#include <iostream>

static const char *tmp_file = "_tmp_config";

void run_vnc(std::string passwd, std::string ip, bool viewonly, int port)
{
  std::cout << "passwd:" << passwd << "\n"; //TODO: DEBUG INFO
  assert(passwd.length()  >= 16);


  std::ofstream tmp(tmp_file);
  tmp << "[connection]\n"
     << "host=" << ip << "\n"
     << "port=" << port << "\n"
     << "password=" << passwd
     << "[options]\n"
     << "viewonly=" << viewonly << "\n";
  tmp.close();

  std::string cmd("vncviewer.exe");
  cmd.append(" /config \"");
  cmd.append(tmp_file);
  cmd.append("\"");
  cmd.append(" /notoolbar");
  wxExecute(wxString(cmd.c_str(), wxConvLocal), wxEXEC_SYNC);
  tmp.close();
  remove(tmp_file);
}
