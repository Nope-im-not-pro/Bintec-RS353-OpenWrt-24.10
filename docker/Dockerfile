# OpenWrt 24.10 Buildhost fuer Bintec RS353 (lantiq/xrx200)
# Quellbaum liegt im named volume /build, NICHT im Windows-Bind-Mount
# (Windows-FS ist case-insensitive -> OpenWrt-Buildbaum bricht).
FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential clang flex bison g++ gawk \
        gcc-multilib g++-multilib gettext git libncurses-dev libssl-dev \
        python3 python3-dev python3-distutils python3-setuptools \
        rsync unzip zlib1g-dev file wget swig time ca-certificates \
        libelf-dev ecj fastjar java-propose-classpath \
        java-wrappers jq quilt xsltproc zstd less nano procps \
    && rm -rf /var/lib/apt/lists/*

ARG UID=1000
ARG GID=1000
RUN groupadd -g ${GID} build && \
    useradd -m -u ${UID} -g ${GID} -s /bin/bash build && \
    mkdir -p /build && chown build:build /build

USER build
WORKDIR /build
ENV FORCE_UNSAFE_CONFIGURE=0

CMD ["/bin/bash"]
