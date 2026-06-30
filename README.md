# Vorlaxen ESP8266 LCD MQTT

WiFi-connected IoT firmware for the ESP8266. Provisions over a captive portal, serves a local web panel, and drives a 16×2 I2C LCD — including Turkish characters.

---

## Features

- **WiFi provisioning** — connect once via a setup hotspot; credentials stored in EEPROM (config v3)
- **Web panel** — view network status and send messages to the LCD from any browser on your LAN
- **mDNS** — reach the device at `http://v-XXXXXX.local/` without looking up its IP
- **LCD preview** — the panel mirrors what appears on the physical display, including scroll mode and Turkish glyphs
- **Factory reset** — hold a GPIO button to wipe WiFi credentials and re-enter setup (MQTT settings are kept)
- **MQTT** — control the LCD from Home Assistant, Node-RED, or any MQTT client
- **Modular firmware** — clean module layout with an event bus and dependency injection

---

## Hardware

| Part | Details |
|------|---------|
| Board | NodeMCU v2 (ESP-12E) |
| Display | 16×2 I2C LCD, address `0x27` |
| LCD wiring | SDA → D2, SCL → D1 |
| Reset button | D5 → GND (hold 500 ms) |

---

## Quick Start

**Prerequisites:** [PlatformIO](https://platformio.org/) installed, board connected via USB.

```bash
# 1. Build
pio run

# 2. Flash firmware
pio run -t upload

# 3. Flash web assets (required — panel and setup UI live here)
pio run -t uploadfs

# 4. Open serial monitor (115200 baud)
pio device monitor
```

Run `uploadfs` whenever you change files inside `data/`.

---

## First-Time Setup

1. Power on the device. With no saved WiFi, it opens a hotspot named **VESPSetup**.
2. Connect your phone or laptop to **VESPSetup**.
3. Open **http://192.168.4.1/** — the captive portal should appear automatically (Android/iOS/macOS/Windows probe URLs are handled).
4. Pick your home WiFi network, enter the password, and tap **Connect**.
5. The device saves credentials, reboots into station mode, joins your router, and shows its IP on the LCD.
6. Open the panel at **`http://v-XXXXXX.local/`** (replace `XXXXXX` with the device ID shown on the LCD or in the serial log).

---

## Web Panel

Once connected to your WiFi, the panel lets you:

- See IP, hostname, SSID, signal strength, and device ID
- Type a message and push it to the LCD (max 64 characters)
- Preview layout: 1 line (≤16 chars), 2 lines (17–32), or single-line scroll (33+ when scroll is enabled)
- Enable single-line scroll and adjust scroll speed
- Restore the default network-info screen

The LCD preview on the page matches the scroll and line-wrap behaviour of the physical display.

MQTT is configured via the REST API (`GET`/`POST /api/mqtt`), not through the web panel UI.

---

## Network Access

| Interface | Scope | Notes |
|-----------|-------|-------|
| HTTP web panel + REST API | LAN only (port 80) | Advertised via mDNS; not exposed by the firmware itself |
| Setup hotspot | Provisioning only | **VESPSetup** AP at `192.168.4.1` |
| MQTT | Outbound client → broker | Remote control goes through your broker, not direct ESP HTTP |

Typical home setup: keep the broker on your LAN (or reach it via Home Assistant / VPN). Port-forwarding the broker is optional; the ESP’s web API stays on the local network unless you explicitly forward port 80 to the device.

---

## Factory Reset

Hold **D5** to GND for **500 ms** (after a 3 s boot grace period).

WiFi credentials are cleared, the LCD shows a confirmation message, and the device re-enters provisioning mode (**VESPSetup** hotspot). Device ID and MQTT settings in EEPROM are **not** cleared.

---

## Configuration

### Build flags

Set these in `platformio.ini`:

| Flag | Default | Purpose |
|------|---------|---------|
| `HOSTNAME_PREFIX` | `v` | mDNS name prefix → `v-123456.local` |
| `SETUP_AP_SSID` | `VESPSetup` | Hotspot name during provisioning |
| `FACTORY_RESET_PIN` | `D5` | GPIO pin for factory reset |
| `MQTT_TOPIC_PREFIX` | `vorlaxen` | Default MQTT topic prefix |
| `MQTT_DEFAULT_PORT` | `1883` | Default MQTT broker port |
| `MQTT_DEFAULT_BROKER` | *(empty)* | Optional compile-time broker address |

### Source constants

| File | Contents |
|------|----------|
| `include/core/DeviceConstants.h` | Timeouts, LCD pins, message limits, MQTT intervals |
| `include/core/UiStrings.h` | On-screen status messages |
| `include/config/ConfigTypes.h` | EEPROM layout (v3), checksum helpers |

Settings are stored in EEPROM (`CONFIG_VERSION` 3, 512 bytes) with automatic migration from older layouts.

---

## REST API

Successful mutations return `{"ok": true}`. Errors use `{"ok": false, "error": "<code>"}`.

Read endpoints (`GET /api/status`, `GET /api/mqtt`, `GET /api/setup/scan`) return plain JSON objects without an `ok` wrapper.

### Panel (connected to home WiFi)

**GET `/api/status`**

```json
{
  "ip": "192.168.1.42",
  "hostname": "v-123456.local",
  "deviceId": "123456",
  "ssid": "MyNetwork",
  "rssi": -62
}
```

**POST `/api/message`**

```json
{ "text": "Hello world", "scroll": true, "scrollMs": 300 }
```

| Field | Required | Default | Notes |
|-------|----------|---------|-------|
| `text` | yes | — | Max 64 characters; longer input is truncated |
| `scroll` | no | `true` | Single-line scroll for text longer than 32 characters |
| `scrollMs` | no | `300` | Scroll speed in milliseconds |

Also accepts form fields (`text`, `scroll`, `scrollMs`) for compatibility.

**POST `/api/display/restore`** — switch the LCD back to the network-info screen.

**GET `/api/mqtt`** — read MQTT settings and connection state.

```json
{
  "enabled": true,
  "broker": "192.168.1.10",
  "port": 1883,
  "username": "",
  "hasPassword": false,
  "topicPrefix": "vorlaxen",
  "connected": true,
  "lastConnectState": 0
}
```

Passwords are never returned; `hasPassword` indicates whether one is stored.

**POST `/api/mqtt`** — save MQTT settings (stored in EEPROM, applied immediately).

```json
{
  "enabled": true,
  "broker": "192.168.1.10",
  "port": 1883,
  "username": "",
  "password": "",
  "topicPrefix": "vorlaxen"
}
```

Omit `password` to keep the existing stored password. If `topicPrefix` is empty, it defaults to `MQTT_TOPIC_PREFIX`.

| Error code | When |
|------------|------|
| `broker_required` | MQTT enabled but broker is empty |
| `invalid_body` / `invalid_json` | Malformed request |

### Provisioning (VESPSetup hotspot only)

**GET `/api/setup/scan`**

```json
{
  "networks": [
    { "ssid": "MyNetwork", "rssi": -62, "secure": true }
  ]
}
```

**POST `/api/setup/connect`**

```json
{ "ssid": "MyNetwork", "password": "secret" }
```

| Error code | When |
|------------|------|
| `ssid_required` | SSID missing |
| `invalid_body` / `invalid_json` | Malformed request |

---

## MQTT

The device connects to an MQTT broker as a **client** when WiFi is up and MQTT is enabled. Configure via `POST /api/mqtt` or at build time with `MQTT_DEFAULT_BROKER`.

Authentication is optional: if `username` is set, the device connects with username/password. Topic ACL and TLS are broker-side concerns; the firmware uses plain TCP on port 1883.

### Topics

Replace `{id}` with your device ID (e.g. `123456`). The prefix defaults to `vorlaxen` but is configurable via `topicPrefix`.

| Topic | Direction | Purpose |
|-------|-----------|---------|
| `{prefix}/{id}/display/set` | Subscribe | Send text to the LCD |
| `{prefix}/{id}/display/restore` | Subscribe | Restore network-info screen |
| `{prefix}/{id}/status` | Publish | Device status (JSON, retained) |
| `{prefix}/{id}/availability` | Publish | `online` / `offline` (LWT) |

### Display message formats

Plain text:

```
Hello from MQTT
```

JSON:

```json
{ "text": "Hello from MQTT", "scroll": true, "scrollMs": 300 }
```

The `message` field is accepted as an alias for `text`. `scrollMs` below 50 ms is clamped to the default (300 ms).

Restore screen:

```json
{ "action": "restore" }
```

Or publish any payload (including empty) to `display/restore`.

### Status payload

Published to `{prefix}/{id}/status` (retained):

```json
{
  "online": true,
  "deviceId": "123456",
  "ip": "192.168.1.42",
  "hostname": "v-123456.local",
  "ssid": "MyNetwork",
  "rssi": -62
}
```

When offline, `online` is `false` and network fields are omitted.

### Home Assistant

On connect, the device publishes a discovery message to `homeassistant/text/vorlaxen_{id}/config`. With the MQTT integration enabled, the LCD appears as a **Text** entity you can automate.

Example publish (Python helper — no `mosquitto_pub` required):

```bash
python3 scripts/mqtt_pub.py -H 192.168.1.10 -d 123456 "Hello MQTT"
python3 scripts/mqtt_pub.py -H 192.168.1.10 -d 123456 --restore
python3 scripts/mqtt_pub.py -H 192.168.1.10 -d 123456 "Scroll me" --scroll --scroll-ms 400
```

Or with Mosquitto CLI:

```bash
mosquitto_pub -h 192.168.1.10 -t vorlaxen/123456/display/set \
  -m '{"text":"Hello HA","scroll":false}'
```

---

## Testing

**57 native C++ tests** across 6 suites + **20 Python tests** (protocol, MQTT integration, optional device/E2E).

See [test/README.md](test/README.md) and [scripts/README.md](scripts/README.md) for details.

### Quick run

```bash
pip install -r scripts/requirements.txt
./scripts/run_tests.sh

# With Mosquitto broker (or let run_tests.sh auto-detect localhost:1883)
./scripts/start_test_broker.sh
./scripts/run_tests.sh --integration --strict

# Full suite against your device
DEVICE_HOST=192.168.1.42 MQTT_DEVICE_ID=123456 MQTT_BROKER=192.168.1.10 \
  ./scripts/run_tests.sh --all --strict
```

### Test matrix

| Layer | Command | Count |
|-------|---------|-------|
| Firmware build | `pio run` | — |
| MQTT topics + parser | `pio test -e native -f test_mqtt` | 23 |
| Config checksum + layout | `pio test -e native -f test_config` | 7 |
| ConfigModule EEPROM | `pio test -e native -f test_config_storage` | 8 |
| EventBus | `pio test -e native -f test_event_bus` | 5 |
| Events + constants | `pio test -e native -f test_events` | 7 |
| Turkish LCD UTF-8 | `pio test -e native -f test_turkish_lcd` | 7 |
| **Native total** | `pio test -e native` | **57** |
| Python protocol | `cd scripts && python3 -m unittest tests.test_protocol -v` | 6 |
| Python MQTT integration | `tests.test_mqtt_integration` | 5 |
| Device API smoke | `tests.test_device_api` | 6 |
| Device MQTT E2E | `tests.test_device_e2e` | 3 |
| **Python total** | | **20** |

Device and E2E tests require `DEVICE_HOST` and are skipped unless `--device`, `--e2e`, or `--all` is passed.

### Environment variables

| Variable | Default | Purpose |
|----------|---------|---------|
| `MQTT_BROKER` | `127.0.0.1` | Broker host |
| `MQTT_PORT` | `1883` | Broker port |
| `MQTT_PREFIX` | `vorlaxen` | Topic prefix |
| `MQTT_DEVICE_ID` | `test000` | Device ID in topics |
| `DEVICE_HOST` | — | Device IP/URL for API/E2E (`DEVICE_IP` alias) |
| `VORLAXEN_TEST_STRICT` | `0` | Fail instead of skip |

### CI

GitHub Actions runs firmware build, all **57** native tests, Python protocol + MQTT integration (Mosquitto service container), and `./scripts/run_tests.sh --integration --strict --no-build`.

---

## Project Layout

```
ESP/
├── platformio.ini          Build config (nodemcuv2 + native test env)
├── .github/workflows/      CI (build + unit + MQTT integration)
├── include/                Public headers (mirrors src/)
├── scripts/                See scripts/README.md
│   ├── run_tests.sh        Main test runner
│   ├── mqtt_pub.py         CLI wrapper → tools/mqtt_pub.py
│   ├── vorlaxen/           Python test library
│   ├── tools/              CLI tools (mqtt_pub)
│   └── tests/              Python unittest suites
├── test/                   57 native C++ tests — see test/README.md
│   └── support/native/     Arduino/EEPROM/LCD stubs
├── src/
│   ├── main.cpp            Entry point
│   ├── app/                Application composition root
│   ├── config/             EEPROM, ConfigModule, factory reset
│   ├── core/               Event bus, constants, logging
│   ├── display/            LCD driver, Turkish glyphs
│   ├── mqtt/               Topic builder, payload parser (testable C API)
│   └── network/            WiFi, HTTP, MQTT, provisioning
├── data/web/               LittleFS assets (HTML, CSS, JS)
│   ├── index.html          Main panel
│   └── setup.html          Captive portal setup UI
```

`Application` is the composition root: it wires modules and defines init/loop order. Modules communicate through an injected `EventBus` instead of calling each other directly.

Init order: `ConfigModule` → `DisplayModule` → `WebServerModule` → `MqttModule` → `FactoryResetModule` → `NetworkModule`.

Loop order: factory reset → network → MQTT → web server → display.

---

## Troubleshooting

| Problem | What to try |
|---------|-------------|
| Setup page is blank or 404 | Run `pio run -t uploadfs` |
| Cannot reach `v-XXXXXX.local` | Use the IP address from the LCD or serial log; check that your client supports mDNS |
| Device keeps reopening setup | Saved WiFi may be wrong — factory reset and provision again |
| LCD shows nothing | Check I2C address (`0x27`) and wiring (SDA=D2, SCL=D1) |
| Serial shows `LittleFS mount failed` | Flash the filesystem: `pio run -t uploadfs` |
| MQTT not connecting | Check settings via `GET /api/mqtt`; verify broker IP and port 1883 |
| Home Assistant entity missing | Enable MQTT integration; restart device after saving MQTT settings |
| Remote LCD control fails | Ensure the broker is reachable; check topic prefix and device ID match |

---

## License

Proprietary — Vorlaxen Labs.
