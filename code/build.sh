mkdir -p ../../build
pushd ../../build
g++ -g -o cbemu ../cbemu/code/cbemu.cpp -lpthread -Llib -Wall -Wno-write-strings -Wno-unused-variable

chmod +x cbemu
popd
