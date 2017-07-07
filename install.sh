sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update 
sudo apt-get -y install build-essential
sudo apt-get -y install git
sudo apt-get -y install python
sudo apt-get -y install apt-file
sudo apt-get -y install software-properties-common
sudo apt-get -y install cmake
sudo apt-file update
sudo apt-get -y install gcc-5 g++-5 libboost-all-dev libeigen3-dev libopencv-dev opencv-data
sudo apt-get update 
git clone --recursive https://github.com/kentegrate/4dface.git
cd 4dface && mkdir build && cd build && cmake -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc-5 -DCMAKE_CXX_COMPILER=g++-5 -DOpenCV_haarcascades_DIR=/usr/share/opencv/haarcascades/ ../ && make -j4 && sudo make install



