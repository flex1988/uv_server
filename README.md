# uv_server

    hello world http server depend on libuv and http-parser.

## INSTALL
    git clone --recursive git@github.com:flex1988/uv_server.git
    
    cd uv_server/libuv
    ./autogen.sh
    ./configure
    make
    make install
    
    cd http-server
    make
    
    cd ../
    
    make
