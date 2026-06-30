#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
SCRIPTS="$ROOT/scripts"

RUN_BUILD=1
RUN_NATIVE=1
RUN_PYTHON=1
RUN_INTEGRATION=0
RUN_DEVICE=0
RUN_E2E=0
STRICT=0
VERBOSE=0
INSTALL_DEPS=0

usage() {
  cat <<'EOF'
Usage: ./scripts/run_tests.sh [options]

Runs firmware build, 57 native C++ tests, and Python unittest suites.

Options:
  --all              Integration + device + E2E (when configured)
  --integration      MQTT broker integration tests
  --device           Device REST API smoke tests (DEVICE_HOST)
  --e2e              Device + MQTT E2E (DEVICE_HOST + broker)
  --strict           Fail when broker/device unavailable
  --no-build         Skip firmware build
  --unit-only        C++ native tests only
  --install-deps     pip install -r scripts/requirements.txt
  -v, --verbose      Verbose PlatformIO output
  -h, --help         Show help

Environment: MQTT_BROKER, MQTT_PORT, DEVICE_HOST, VORLAXEN_TEST_STRICT
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --all) RUN_INTEGRATION=1; RUN_DEVICE=1; RUN_E2E=1 ;;
    --integration) RUN_INTEGRATION=1 ;;
    --device) RUN_DEVICE=1 ;;
    --e2e) RUN_E2E=1 ;;
    --strict) STRICT=1 ;;
    --no-build) RUN_BUILD=0 ;;
    --unit-only) RUN_PYTHON=0 ;;
    --install-deps) INSTALL_DEPS=1 ;;
    -v|--verbose) VERBOSE=1 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown: $1" >&2; usage >&2; exit 2 ;;
  esac
  shift
done

[[ "$STRICT" -eq 1 ]] && export VORLAXEN_TEST_STRICT=1

if [[ "$RUN_PYTHON" -eq 1 && "$RUN_INTEGRATION" -eq 0 && "$RUN_DEVICE" -eq 0 && "$RUN_E2E" -eq 0 ]]; then
  if python3 -c "import socket; s=socket.create_connection(('${MQTT_BROKER:-127.0.0.1}', ${MQTT_PORT:-1883}), 1); s.close()" 2>/dev/null; then
    RUN_INTEGRATION=1
  fi
fi

PASS=0; FAIL=0; SKIP=0; FAILED_STEPS=()

record() {
  local name="$1" code="$2" skipped="${3:-0}"
  if [[ "$skipped" -eq 1 ]]; then SKIP=$((SKIP+1)); echo "[SKIP] $name"
  elif [[ "$code" -eq 0 ]]; then PASS=$((PASS+1)); echo "[PASS] $name"
  else FAIL=$((FAIL+1)); FAILED_STEPS+=("$name"); echo "[FAIL] $name (exit $code)" >&2; fi
}

command -v pio >/dev/null || { echo "pio not found" >&2; exit 127; }

PIO_FLAGS=(); [[ "$VERBOSE" -eq 1 ]] && PIO_FLAGS=(-vv)

if [[ "$RUN_BUILD" -eq 1 ]]; then
  echo "==> Firmware build"
  pio run && record "firmware build" 0 || record "firmware build" 1
fi

if [[ "$RUN_NATIVE" -eq 1 ]]; then
  echo "==> Native C++ tests (57 cases, 6 suites)"
  pio test -e native "${PIO_FLAGS[@]}" && record "native unit tests" 0 || record "native unit tests" 1
fi

if [[ "$RUN_PYTHON" -eq 1 ]]; then
  command -v python3 >/dev/null || { record "python3" 1; exit 1; }
  [[ "$INSTALL_DEPS" -eq 1 ]] && python3 -m pip install -r "$SCRIPTS/requirements.txt"

  if ! python3 -c "import paho.mqtt.client" 2>/dev/null; then
    echo "Warning: pip install -r scripts/requirements.txt" >&2
    [[ "$STRICT" -eq 1 ]] && record "python deps" 1 || { record "python deps" 0 1; RUN_INTEGRATION=0; RUN_E2E=0; }
  fi

  echo "==> Python protocol tests"
  (cd "$SCRIPTS" && PYTHONPATH="$SCRIPTS" python3 -m unittest tests.test_protocol -v) \
    && record "python protocol" 0 || record "python protocol" 1

  if [[ "$RUN_INTEGRATION" -eq 1 ]]; then
    echo "==> Python MQTT integration"
    (cd "$SCRIPTS" && PYTHONPATH="$SCRIPTS" python3 -m unittest tests.test_mqtt_integration -v) \
      && record "python mqtt integration" 0 || record "python mqtt integration" 1
  fi

  if [[ "$RUN_DEVICE" -eq 1 ]]; then
    echo "==> Python device API smoke"
    (cd "$SCRIPTS" && PYTHONPATH="$SCRIPTS" python3 -m unittest tests.test_device_api -v) \
      && record "python device api" 0 || record "python device api" 1
  fi

  if [[ "$RUN_E2E" -eq 1 ]]; then
    echo "==> Python device MQTT E2E"
    (cd "$SCRIPTS" && PYTHONPATH="$SCRIPTS" python3 -m unittest tests.test_device_e2e -v) \
      && record "python device e2e" 0 || record "python device e2e" 1
  fi
fi

echo ""
echo "=== Summary ==="
echo "Passed: $PASS  Failed: $FAIL  Skipped: $SKIP"
if [[ "${#FAILED_STEPS[@]}" -gt 0 ]]; then
  printf 'Failed:\n'; printf '  - %s\n' "${FAILED_STEPS[@]}"
  exit 1
fi
echo "All tests finished."
exit 0
