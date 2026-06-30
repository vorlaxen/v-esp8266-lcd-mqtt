#!/usr/bin/env bash
# Start a local Mosquitto broker in Docker for integration/E2E tests.
set -euo pipefail

IMAGE="${MQTT_DOCKER_IMAGE:-eclipse-mosquitto:2}"
NAME="${MQTT_DOCKER_NAME:-vorlaxen-mosquitto-test}"
PORT="${MQTT_PORT:-1883}"

if docker ps --format '{{.Names}}' | grep -qx "$NAME"; then
  echo "Broker container already running: $NAME (port $PORT)"
  exit 0
fi

if docker ps -a --format '{{.Names}}' | grep -qx "$NAME"; then
  echo "Starting existing container: $NAME"
  docker start "$NAME" >/dev/null
else
  echo "Creating Mosquitto container $NAME on port $PORT"
  docker run -d --name "$NAME" -p "${PORT}:1883" "$IMAGE" >/dev/null
fi

echo "Waiting for broker on 127.0.0.1:${PORT}..."
for _ in $(seq 1 30); do
  if python3 -c "import socket; s=socket.create_connection(('127.0.0.1', ${PORT}), 1); s.close()" 2>/dev/null; then
    echo "Broker ready."
    exit 0
  fi
  sleep 0.5
done

echo "Error: broker did not become ready in time." >&2
exit 1
