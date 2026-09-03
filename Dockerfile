# Faz Ai - imagem Docker (servidor C++ + frontend web)
# Build multi-stage: compila o servidor e depois monta uma imagem final leve.

FROM ubuntu:24.04 AS build
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++ make ca-certificates curl \
    libpq-dev libpqxx-dev libssl-dev nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY server/ ./server/
RUN curl -sL https://raw.githubusercontent.com/yhirose/cpp-httplib/v0.15.3/httplib.h -o ./server/httplib.h \
    && g++ -std=c++17 -O2 -pthread ./server/server.cpp -o /src/fazai_server \
       $(pkg-config --cflags --libs libpqxx) -lssl -lcrypto

FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends \
    libpq5 libpqxx-7.8t64 libssl3 ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /src/fazai_server ./fazai_server
COPY web/ ./web/

ENV PORT=8080
EXPOSE 8080
CMD ["./fazai_server"]
