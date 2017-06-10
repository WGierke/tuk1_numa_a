FROM ubuntu

RUN apt-get update \
    && apt-get install -y build-essential libnuma-dev cmake gdb \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
