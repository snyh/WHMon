help:
	@echo "use make win or make lin"

win: 
	[ -d ./build/win ] || mkdir -p build/win
	cp ./tool/mingw32.cmake build/win
	cd build/win/ && cmake -DCMAKE_TOOLCHAIN_FILE=./mingw32.cmake ../../ && make

lin:
	[ -d ./build/lin ] || mkdir -p build/lin
	cd build/lin/ && cmake ../../ && make

web-run:
	unzip -o tool/resource.zip -d ./build/lin
	cd ./build/lin/ && ./webgui.wt

package: 
	cp build/win/server.exe tool/nsis/WHMon.exe
	i686-pc-mingw32-makensis tool/nsis/whmon.nsi
	cp tool/nsis/WHMonInstaller.exe .
	cp build/win/src/webgui/webgui.wt.exe webgui.exe && strip webgui.exe && upx webgui.exe
	cp tool/resource.zip webgui.zip && zip webgui.zip webgui.exe
	rm webgui.exe 
