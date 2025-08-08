FROM ubuntu:20.04

WORKDIR /app

ARG DEBIAN_FRONTEND=noninteractive

# APT packages
RUN apt update && apt install -y --no-install-recommends git ninja-build gperf \
    ccache dfu-util device-tree-compiler wget xz-utils file \
    make gcc libsdl2-dev curl

# Python
RUN apt install software-properties-common -y
RUN add-apt-repository ppa:deadsnakes/ppa && apt update
RUN apt install python3.10 python3.10-venv python3.10-dev -y
RUN rm /usr/bin/python3 && ln -s python3.10 /usr/bin/python3
# install pip
RUN curl -sS https://bootstrap.pypa.io/get-pip.py | python3.10

# Install recent CMake
RUN if [ $(uname -m) = "x86_64" ]; then export ARCH=x86_64; else export ARCH=aarch64; fi && \
    mkdir -p /opt/cmake && \
    cd /opt/cmake && \
    wget https://github.com/Kitware/CMake/releases/download/v3.21.3/cmake-3.21.3-linux-$ARCH.sh  && \
    sh cmake-3.21.3-linux-$ARCH.sh --prefix=/opt/cmake --skip-license && \
    ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake && \
    rm /opt/cmake/cmake-3.21.3-linux-$ARCH.sh

# Zephyr SDK
RUN if [ $(uname -m) = "x86_64" ]; then export ARCH=x86_64; else export ARCH=aarch64; fi && \
    cd /opt && \
    wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.1/zephyr-sdk-0.16.1_linux-${ARCH}_minimal.tar.xz && \
    tar xf zephyr-sdk-0.16.1_linux-${ARCH}_minimal.tar.xz && \
    cd zephyr-sdk-0.16.1 && \
    ./setup.sh -t arm-zephyr-eabi -c
ENV ZEPHYR_TOOLCHAIN_VARIANT=zephyr
ENV ZEPHYR_SDK_INSTALL_DIR=/opt/zephyr-sdk-0.16.1

# Install west and the nRF Connect SDK
RUN python3.10 -m pip install pip==21.2.4
RUN pip install west==1.2.0
RUN pip install ecdsa==0.17.0
RUN mkdir /ncs
RUN cd /ncs && west init -m https://github.com/nrfconnect/sdk-nrf --mr v3.0.0
RUN cd /ncs && west update
RUN cd /ncs && west zephyr-export
RUN pip3 install -r /ncs/zephyr/scripts/requirements.txt

ENV ZEPHYR_BASE="/ncs/zephyr"
