#!/bin/bash

set -e

CONTAINER=aspeed-lpc-control-build

Dockerfile=$(cat << EOF
FROM ubuntu:18.04
RUN apt-get update && apt-get install --no-install-recommends -yy \
	make \
	gcc-arm-linux-gnueabi \
	libc-dev-armel-cross \
	autoconf \
	automake \
	libtool \
	git
RUN groupadd -g ${GROUPS} ${USER} && useradd -d ${HOME} -m -u ${UID} -g ${GROUPS} ${USER}
USER ${USER}
ENV HOME ${HOME}
RUN /bin/bash
EOF
)

docker pull ubuntu:18.04
docker build -t ${CONTAINER} - <<< "${Dockerfile}"

RUN="docker run --rm=true --user=${USER} -w ${PWD} -v ${HOME}:${HOME} -t ${CONTAINER}"

${RUN} autoreconf -i

# arm
${RUN} ./configure --host=arm-linux-gnueabi
${RUN} make
${RUN} make clean

# native
${RUN} ./configure
${RUN} make
