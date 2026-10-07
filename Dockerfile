FROM ubuntu:24.04 AS build

RUN apt-get update && \
    apt-get install -y \
    build-essential \
    cmake \
    curl \
    ca-certificates && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCOREAI_BUILD_TESTS=OFF \
    -DCOREAI_BUILD_NATIVE_GUI=OFF

RUN cmake --build build -j2

FROM ubuntu:24.04

RUN apt-get update && \
    apt-get install -y \
    curl \
    ca-certificates \
    libsqlite3-0 && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=build /app/build/core-ai /app/core-ai
COPY --from=build /app/web /app/gui

ENV PORT=10000

EXPOSE 10000

CMD ["/app/core-ai"]