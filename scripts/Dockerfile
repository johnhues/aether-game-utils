FROM ubuntu:24.04

# Install all dependencies for github workflows
RUN apt update -qq && \
	apt install -y --no-install-recommends \
		ca-certificates \
		gpg \
		wget \
		curl \
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
		xz-utils && \
	wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
	echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
	apt update -qq && \
	apt install -y --no-install-recommends kitware-archive-keyring cmake ninja-build && \
	rm -rf /var/lib/apt/lists/*

# Node for test/web/check.js, which needs its built in WebSocket (Node 22+)
ARG TARGETARCH
RUN case "${TARGETARCH}" in \
		amd64) NODE_ARCH=x64; NODE_SHA=4f2b175a6d53a5c617a80dba2676238a0688cef28c6da4eba35c7d844e66fcfb ;; \
		arm64) NODE_ARCH=arm64; NODE_SHA=6e51a2ada00f0011cc266f65d98922b403d7ad88f809ca2aa615a5fe1ce6e86c ;; \
	esac && \
	wget -q "https://nodejs.org/dist/v24.22.3/node-v24.22.3-linux-${NODE_ARCH}.tar.xz" -O /tmp/node.tar.xz && \
	echo "${NODE_SHA}  /tmp/node.tar.xz" | sha256sum -c - && \
	tar xJf /tmp/node.tar.xz -C /usr/local --strip-components=1 && \
	rm /tmp/node.tar.xz

# Chromium for test/web/check.js, with the system libraries it needs
ENV PLAYWRIGHT_BROWSERS_PATH=/opt/ms-playwright
RUN npx -y playwright@1.65.0 install --with-deps chromium && \
	rm -rf /var/lib/apt/lists/* /root/.npm

# clang-cl, lld-link and xwin for ubuntu_clangcl.yml. MSVC STL 14.44 needs clang
# 19 or newer. xwin downloads the MSVC CRT and Windows SDK.
RUN apt update -qq && \
	apt install -y --no-install-recommends clang-tools-20 lld-20 llvm-20 && \
	rm -rf /var/lib/apt/lists/*
RUN case "${TARGETARCH}" in \
		amd64) XWIN_ARCH=x86_64; XWIN_SHA=d870eb4b2f390878af6da1ccd3cf321d22fcb72720984853b4be732ae597fc88 ;; \
		arm64) XWIN_ARCH=aarch64; XWIN_SHA=6d56d28537a86f37aa3d041318898f25ee3100c6b6ec332ad873c28faf37be23 ;; \
	esac && \
	wget -q "https://github.com/Jake-Shadle/xwin/releases/download/0.10.0/xwin-0.10.0-${XWIN_ARCH}-unknown-linux-musl.tar.gz" -O /tmp/xwin.tar.gz && \
	echo "${XWIN_SHA}  /tmp/xwin.tar.gz" | sha256sum -c - && \
	tar xzf /tmp/xwin.tar.gz -C /usr/local/bin --strip-components=1 "xwin-0.10.0-${XWIN_ARCH}-unknown-linux-musl/xwin" && \
	rm /tmp/xwin.tar.gz

# Only for local development, not needed for github workflows
# WORKDIR /workspace
# CMD ["/bin/bash"]
