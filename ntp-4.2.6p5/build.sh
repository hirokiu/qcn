./configure --disable-shared --enable-static
make clean
cd libntp
make clean && make
cd ../ntpdate
make clean && make

