#include <windows.h>


int main(int argc, char** argv)
{
  /*
  printf("原始数据: %s\n", argv[2]);
  printf("U2G : %s\n", U2G(argv[2]));
  */
  if(argc == 3)
    MessageBox(NULL , argv[2], argv[1], MB_OK);
  else if(argc == 2)
    MessageBox(NULL , argv[1], "", MB_OK);
}
