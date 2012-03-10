cmake_minimum_required(VERSION 2.6.8)
set(CMAKE_CXX_FLAGS "-std=gnu++0x -DBOOST_THREAD_USE_LIB")
add_executable(webgui.wt 
  		WHMonWebClient.cpp
		model/DbProxy.cpp 
		model/StateListener.cpp 
		model/cmd.cpp 
		model/singleton.cpp 
		view/LoginView.cpp 
		view/MainView.cpp
		view/LoginView.cpp 
		view/UserBoard.cpp 
		view/ClientBoard.cpp 
		view/MyLogger.cpp
		)
target_link_libraries(webgui.wt
	${Wt_EXT_LIBRARY}
	${Wt_HTTP_LIBRARY}
	${Wt_LIBRARY} 
	${Boost_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	sqlite3
	)
if(WIN32)
	target_link_libraries(webgui.wt
		mswsock ssl crypto crypt32 hpdf png GraphicsMagick++ GraphicsMagick lcms tiff freetype jasper jpeg png15 xml2 lzma bz2 z ws2_32 m pthread shlwapi pcre intl iconv kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32
		)
endif(WIN32)
