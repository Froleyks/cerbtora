FROM debian:trixie-slim
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      bash ca-certificates git \
      build-essential cmake meson ninja-build \
      pkg-config m4 libgmp-dev

WORKDIR /cerbtora
COPY . .
RUN make

