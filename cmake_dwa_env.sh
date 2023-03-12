#!/usr/bin/bash

export JSONCPP_ROOT=/rel/rez/third_party/jsoncpp/0.6.0.2
export LIBUNWIND_ROOT=/rel/rez/third_party/unwind/1.0.x.1.0.0.0
export LOG4CPLUS_ROOT=/rel/rez/dwa/log4cplus/1.1.2.x.2.0.0.2/os-CentOS-7/cpp_std-14
export LUA_DIR=/rel/rez/third_party/lua/5.3.5.x.1.0.0.0
export LOGGINGBASE_ROOT=/rel/rez/dwa/logs/10.10.0.0/os-CentOS-7/opt_level-optdebug/refplat-vfx2020.3
export ISPC=/rel/third_party/intelispc/1.14.1/bin/ispc

# TBB and Boost have native cmake support. Just add their paths to the CMAKE_PREFIX_PATH a cmake will find their Configs
export CMAKE_PREFIX_PATH=/rel/rez/dwa/tbb/2020.2.0.x.1.0.0.0/os-CentOS-7/cpp_std-14:/rel/rez/dwa/boost/1.70.0.x.2.1.0.0/cpp_std-14/python-3.7.7.x.1/zlib-1.2.8.x.2

