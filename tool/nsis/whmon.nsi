!include nsDialogs.nsh

Name "WHmon"
InstallDir "$windir\WHMon"
outfile "WHMonInstaller.exe"
licensedata "license.txt"
RequestExecutionLevel admin

page license
page custom readVar readVarLeave
page instfiles

VAR C_IP
VAR C_NAME
VAR S_IP
VAR S_NAME
Function readVar
    nsDialogs::Create 1018
    Pop $0
    ${NSD_CreateLabel} 0 0 20% 20 服务器IP地址
    Pop $0
    ${NSD_CreateText} 80 0 80% 20 "192.168.12.157"
    Pop $C_IP

    ${NSD_CreateLabel} 0 25 20% 20 教室名称
    Pop $0
    ${NSD_CreateText} 80 25 80% 20 "NONAME"
    Pop $C_NAME
    nsDialogs::Show
FunctionEnd
Function readVarLeave
    ${NSD_GetText} $C_IP $0
    StrCpy $S_IP $0
    ${NSD_GetText} $C_NAME $0
    StrCpy $S_NAME $0
FunctionEnd


section 
setoutpath $INSTDIR

execwait "sc stop whmon"
execwait "sc delete whmon"
execwait "sc stop tvnserver"
execwait "sc delete tvnserver"

file license.txt
file WHMon.exe
file hstart.exe
file srvany.exe
file show.exe
file instsrv.exe
file /nonfatal screenhooks.dll
file tvnserver.exe

execwait "$INSTDIR\instsrv.exe WHMon $INSTDIR\srvany.exe"
delete "$INSTDIR\instsrv.exe"
exec "$INSTDIR\tvnserver.exe -install"

WriteRegStr HKLM "SYSTEM\CurrentControlSet\Services\WHMon\Parameters" "Application" "$INSTDIR\WHMon.exe -i $S_IP -p 43200 -n $S_NAME"
WriteRegStr HKLM "SYSTEM\CurrentControlSet\Services\WHMon" "Description" "文华学院辅助系统"

WriteRegDWord HKLM "Software\TightVNC\Server" "AcceptRfbConnections" 0x0
execwait "sc config whmon type= own type= interact"
execwait "sc start whmon"
execwait "sc start tvnserver"


#设置防火墙规则
execwait "netsh firewal add allowedprogram c:\windows\whmon\whmon.exe WHMon enable"
execwait "netsh firewal add allowedprogram c:\windows\whmon\tvnserver.exe TightVNC enable"
sectionend

section "uninstall"
execwait "sc stop tvnserver"
execwait "sc stop whmon"
execwait "sc delete tvnserver"
execwait "sc delete whmon"
delete "$INSTDIR\tvnserver.exe"
delete "$INSTDIR\WHMon.exe"
delete "$INSTDIR\srvany.exe"
delete "$INSTDIR\screenhooks.dll"
RMDir $INSTDIR
sectionend
