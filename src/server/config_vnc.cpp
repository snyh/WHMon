#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <fstream>
#include "d3des.h"
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <wx/filename.h>
#include <wx/utils.h>
using namespace std;

string get_passwd(bool raw=false);

static unsigned char s_fixedkey[8] = {23, 82, 107, 6, 35, 78, 88, 7};

/*
extern const boost::array<char, 763642> _RC_VNC;
extern const boost::array<char, 77824> _RC_VNCHooks;
string get_vnc_path()
{
  static wxFileName name(_T("c:\\windows\\system32\\winvnc.exe"));
  static wxFileName hooks(name.GetPath(), _T("VNCHooks.dll"));
  if(!name.FileExists() || hooks.FileExists()){
      ofstream of(name.GetFullPath().mb_str(), ostream::binary);
      copy(_RC_VNC.begin(), _RC_VNC.end(), ostream_iterator<unsigned char>(of));
      of.close();

      hooks.SetPath(name.GetPath());
      of.open(hooks.GetFullPath().mb_str() , ostream::binary);
      copy(_RC_VNCHooks.begin(),
	   _RC_VNCHooks.end(), ostream_iterator<unsigned char>(of));
      of.close();
  }
  return string(name.GetFullPath().mb_str());
}
*/

string get_passwd(bool raw)
{
  ostringstream os;
  srand(time(NULL));
  static long passwd = 10000000 + rand() % 99999999;
  //std::cout << "passwd:" << passwd;
  unsigned char p[8] = {0};
  sprintf((char*)p, "%ld", passwd);
  if(raw) {
      os <<  p;
  } else {
      deskey(s_fixedkey, EN0);
      des(p, p);
      for(int i=0; i<8; i++){
	  os.fill('0');
	  os.width(2);
	  os << std::hex << int(p[i]);
      }
  }
  return os.str();
}

string _passwd() 
{
  string s = get_passwd();
  const char *d = ",";
  s.insert(2, d);
  s.insert(5, d);
  s.insert(8, d);
  s.insert(11, d);
  s.insert(14, d);
  s.insert(17, d);
  s.insert(20, d);
  return s;
}

void config_vnc(bool isconf)
{
  wxFileName name;
  name.AssignTempFileName(_T("vnc"));

  std::ofstream tmp(name.GetFullPath().mb_str());
  tmp << "Windows Registry Editor Version 5.00\n"
    << "[HKEY_LOCAL_MACHINE\\SOFTWARE\\TightVNC]\n"
    << "[HKEY_LOCAL_MACHINE\\SOFTWARE\\TightVNC\\Server]\n"
    << "\"ExtraPorts\"=\"\"\n"
    << "\"QueryTimeout\"=dword:0000001e\n"
    << "\"QueryAcceptOnTimeout\"=dword:00000000\n"
    << "\"LocalInputPriorityTimeout\"=dword:00000003\n"
    << "\"LocalInputPriority\"=dword:00000000\n"
    << "\"BlockRemoteInput\"=dword:00000000\n"
    << "\"BlockLocalInput\"=dword:00000000\n"
    << "\"IpAccessControl\"=\"\"\n"
    << "\"RfbPort\"=dword:0000170c\n"
    << "\"HttpPort\"=dword:000016a8\n"
    << "\"DisconnectAction\"=dword:00000000\n";
  if(isconf)
    tmp << "\"AcceptRfbConnections\"=dword:00000001\n";
  else
    tmp << "\"AcceptRfbConnections\"=dword:00000000\n";
  tmp << "\"UseVncAuthentication\"=dword:00000001\n"
    << "\"UseControlAuthentication\"=dword:00000001\n"
    << "\"LoopbackOnly\"=dword:00000000\n"
    << "\"AcceptHttpConnections\"=dword:00000000\n"
    << "\"LogLevel\"=dword:00000000\n"
    << "\"EnableFileTransfers\"=dword:00000001\n"
    << "\"BlankScreen\"=dword:00000000\n"
    << "\"RemoveWallpaper\"=dword:00000000\n"
    << "\"EnableUrlParams\"=dword:00000001\n"
    << "\"Password\"=hex:" << _passwd() <<  "\n"
    << "\"PasswordViewOnly\"=hex:" << _passwd() << "\n" 
    << "\"ControlPassword\"=hex:" << _passwd() << "\n"
    << "\"AlwaysShared\"=dword:00000000\n"
    << "\"NeverShared\"=dword:00000000\n"
    << "\"DisconnectClients\"=dword:00000001\n"
    << "\"PollingInterval\"=dword:000003e8\n"
    << "\"DisableTrayIcon\"=dword:00000001\n"
    << "\"AllowLoopback\"=dword:00000000\n"
    << "\"VideoRecognitionInterval\"=dword:00000bb8\n"
    << "\"GrabTransparentWindows\"=dword:00000001\n"
    << "\"SaveLogToAllUsersPath\"=dword:00000000\n"
    << "\"RunControlInterface\"=dword:00000000\n"
    << "\"VideoClasses\"=\"\"\n";
  tmp.close();
  wxExecute(_T("regedit /s ") + name.GetFullPath(), wxEXEC_SYNC);
  wxSleep(2);
  remove(name.GetFullPath().mb_str());
};
