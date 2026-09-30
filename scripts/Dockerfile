FROM ubuntu:24.04

# Install all dependencies for github workflows
RUN apt update -qq && \
	apt install -y --no-install-recommends \
		ca-certificates \
		gpg \
		wget \
		libgl1-mesa-dev \
		uuid-dev \
		clang \
		lldb \
		g++ \
		gdb \
		mingw-w64 \
		wine \
		default-jre-headless \
		git \
		make \
		python3 \
		nodejs \
		xz-utils && \
	wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
	echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
	apt update -qq && \
	apt install -y --no-install-recommends kitware-archive-keyring cmake ninja-build && \
	rm -rf /var/lib/apt/lists/*

# Install Emscripten SDK
RUN git clone https://github.com/emscripten-core/emsdk.git /opt/emsdk && \
	cd /opt/emsdk && \
	./emsdk install latest && \
	./emsdk activate latest

ENV EMSDK=/opt/emsdk
ENV EMSDK_QUIET=1
ENV PATH="/opt/emsdk:/opt/emsdk/upstream/emscripten:${PATH}"

# Only for local development, not needed for github workflows
# WORKDIR /workspace
# CMD ["/bin/bash"]
