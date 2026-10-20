# Multi-stage build for RailEase C++ Backend & Full-Stack Server
FROM gcc:12-bookworm AS builder

WORKDIR /app

# Copy headers and source code
COPY include/ ./include/
COPY server.cpp railway.cpp ./

# Build with optimizations and pthread
RUN g++ -std=c++17 -O2 server.cpp -pthread -o server

# Minimal production runtime image
FROM debian:bookworm-slim

WORKDIR /app

# Install standard runtime libraries
RUN apt-get update && apt-get install -y --no-install-recommends libstdc++6 ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Copy compiled binary and static data
COPY --from=builder /app/server ./server
COPY public/ ./public/
COPY trains.txt passengers.txt ./

# Default port (Render/Railway will inject PORT dynamically)
EXPOSE 8080
ENV PORT=8080

CMD ["./server"]
