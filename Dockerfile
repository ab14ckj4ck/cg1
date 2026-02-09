FROM ubuntu:22.04

RUN apt-get update 
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get install -y --no-install-recommends \
    cmake \
    build-essential \
    gcc g++ \
    ninja-build \
    imagemagick \
    git \
    gdb \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/*

WORKDIR /cg1

CMD /bin/zsh