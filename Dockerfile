FROM ubuntu:14.04
MAINTAINER Nobuaki Sukegawa <nsukeg@gmail.com>

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
      software-properties-common \
      python-software-properties
RUN add-apt-repository -y ppa:beineri/opt-qt55-trusty
RUN apt-get update && apt-get install -y \
      libgl1-mesa-dev \
      qt55base \
      qt55declarative \
      ninja-build \
      cmake \
      python \
      gcc \
      g++ \
      clang \
      git \
      python-setuptools \
      python-yaml \
      python-mako \
      golang \
      cmake \
      ninja-build \
      zlib1g-dev

ENV PROJECT_DIR=/opt/protobuf-qml
ADD . $PROJECT_DIR
WORKDIR $PROJECT_DIR
ENV DEPS_DIR=$PROJECT_DIR/build/deps
ENV PATH=$DEPS_DIR/bin:$PATH \
    LDFLAGS="-L$DEPS_DIR/lib $LDFLAGS" \
    LIBRARY_PATH=$DEPS_DIR/lib:$LIBRARY_PATH \
    LD_LIBRARY_PATH=$DEPS_DIR/lib:$LD_LIBRARY_PATH \
    C_INCLUDE_DIR=$DEPS_DIR/include:$C_INCLUDE_DIR \
    CPLUS_INCLUDE_DIR=$DEPS_DIR/include:$CPLUS_INCLUDE_DIR

RUN tools/build_dependencies.py --shared

# Enable source by replacing sh with bash
RUN ln -snf /bin/bash /bin/sh
RUN . /opt/qt55/bin/qt55-env.sh
RUN . tools/setup_env.sh
RUN tools/bootstrap.py --qt5dir /opt/qt55/lib/cmake
RUN ninja -C out/Debug
RUN ninja -C out/Release

CMD /bin/bash
