# wiiqt
Collection of wii-related Qt/C++ stuff

### Usage - Linux

1) `sudo add-apt-repository ppa:rock-core/qt4 -y`
2) `sudo apt-get install libqtgui4`
3) `./executable_name`

### Building - Linux

1) `sudo add-apt-repository ppa:rock-core/qt4 -y`
2) `sudo apt install qt4-dev-tools libqt4-dev libqtcore4 libqtgui4 g++ -y`
3) `cd app_path`
4) `qmake -qt=qt4 file_name.pro`
5) `make -f Makefile`
