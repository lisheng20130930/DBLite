# Microsoft Developer Studio Project File - Name="test" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=test - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "test.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "test.mak" CFG="test - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "test - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "test - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "test - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "test - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "./" /I "../utils" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c /Tp
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"test.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "test - Win32 Release"
# Name "test - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "utils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\utils\Base64.c
# End Source File
# Begin Source File

SOURCE=..\utils\Base64.h
# End Source File
# Begin Source File

SOURCE=..\utils\buffer.c
# End Source File
# Begin Source File

SOURCE=..\utils\buffer.h
# End Source File
# Begin Source File

SOURCE=..\utils\cJSON.c
# End Source File
# Begin Source File

SOURCE=..\utils\cJSON.h
# End Source File
# Begin Source File

SOURCE=..\utils\Crc16.c
# End Source File
# Begin Source File

SOURCE=..\utils\Crc16.h
# End Source File
# Begin Source File

SOURCE=..\utils\Des.c
# End Source File
# Begin Source File

SOURCE=..\utils\Des.h
# End Source File
# Begin Source File

SOURCE=..\utils\Idmap.c
# End Source File
# Begin Source File

SOURCE=..\utils\Idmap.h
# End Source File
# Begin Source File

SOURCE=..\utils\libos.c
# End Source File
# Begin Source File

SOURCE=..\utils\libos.h
# End Source File
# Begin Source File

SOURCE=..\utils\listlib.h
# End Source File
# Begin Source File

SOURCE=..\utils\log.c
# End Source File
# Begin Source File

SOURCE=..\utils\log.h
# End Source File
# Begin Source File

SOURCE=..\utils\Md5.c
# End Source File
# Begin Source File

SOURCE=..\utils\Md5.h
# End Source File
# Begin Source File

SOURCE=..\utils\Rc4.c
# End Source File
# Begin Source File

SOURCE=..\utils\Rc4.h
# End Source File
# Begin Source File

SOURCE=..\utils\stack.c
# End Source File
# Begin Source File

SOURCE=..\utils\stack.h
# End Source File
# Begin Source File

SOURCE=..\utils\stdbool.h
# End Source File
# Begin Source File

SOURCE=..\utils\timer.c
# End Source File
# Begin Source File

SOURCE=..\utils\timer.h
# End Source File
# Begin Source File

SOURCE=..\utils\ttlv.c
# End Source File
# Begin Source File

SOURCE=..\utils\ttlv.h
# End Source File
# Begin Source File

SOURCE=..\utils\Xor.c
# End Source File
# Begin Source File

SOURCE=..\utils\Xor.h
# End Source File
# End Group
# Begin Group "litdb"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\config.c
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\DBM.c
# End Source File
# Begin Source File

SOURCE=.\DBM.h
# End Source File
# Begin Source File

SOURCE=.\litdb.h
# End Source File
# Begin Source File

SOURCE=.\lsg.c
# End Source File
# Begin Source File

SOURCE=.\pack.c
# End Source File
# Begin Source File

SOURCE=.\pack.h
# End Source File
# Begin Source File

SOURCE=.\tcdb.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\test.c
# End Source File
# Begin Source File

SOURCE=.\test_DataBase.c
# End Source File
# Begin Source File

SOURCE=.\test_DataBase.h
# End Source File
# Begin Source File

SOURCE=.\test_User.c
# End Source File
# Begin Source File

SOURCE=.\test_User.h
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\test_DB_Priv.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
