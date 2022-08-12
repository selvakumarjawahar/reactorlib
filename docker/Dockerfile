FROM  ubuntu:focal

RUN ln -fs /usr/share/zoneinfo/Asia/Kolkata /etc/localtime

RUN apt-get -y update && apt-get install -y \
                         build-essential \
                         cmake \
                         curl \
                         git \
                         python3 \
                         clang-tools \
                         clang-format\
                         clang-tidy \
                         libasan5 \
                         libubsan0 


RUN git clone https://github.com/catchorg/Catch2.git && \
         cd Catch2 && \
         git checkout v2.x && \
         cp -r single_include/catch2 /usr/include/ && \
         cd .. && \
         rm -fr Catch2

RUN curl -L -o boost_1_78_0.tar.gz https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.tar.gz && \
         tar xzf boost_1_78_0.tar.gz && \
         cd boost_1_78_0 && \
         ./bootstrap.sh && \
         ./b2 link=static install || true && \
         cd .. && \
         rm -fr boost_1_78_0.tar.gz && \
         rm -fr boost_1_78_0

RUN git clone https://github.com/gabime/spdlog.git && \
        cd spdlog && mkdir build && cd build && \
        cmake .. && make -j && make install && \
        cd .. && \
        rm -fr spdlog

