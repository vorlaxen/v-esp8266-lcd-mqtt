# Native Unit Tests (PlatformIO + Unity)

Host-side tests run on your PC — no ESP8266 hardware required.

## Layout

```
test/
├── support/native/       Arduino/EEPROM/LCD stubs for native builds
├── test_mqtt/            MQTT topics + payload parser (23 tests)
├── test_config/          EEPROM checksum + struct layout (7 tests)
├── test_config_storage/  ConfigModule persistence + migration (8 tests)
├── test_event_bus/       EventBus pub/sub (5 tests)
├── test_events/          Event factories + constants (7 tests)
└── test_turkish_lcd/     Turkish UTF-8 LCD logic (7 tests)
```

Total: **57 tests** across 6 suites.

Each suite has `test_main.cpp` (tests) and `link.cpp` (source files under test).

## Run

```bash
pio test -e native          # all 57 tests
pio test -e native -vv      # verbose
./scripts/run_tests.sh --unit-only
```

## Adding tests

1. Create `test/test_<module>/test_main.cpp`
2. Add `test/test_<module>/link.cpp` with `#include "../../src/..."` for code under test
3. Extend stubs in `test/support/native/` if new Arduino APIs are needed
