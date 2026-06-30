# Vorlaxen Scripts

Organized tooling and automated tests for firmware validation.

## Layout

```
scripts/
├── run_tests.sh          Main test runner (firmware + C++ + Python)
├── start_test_broker.sh  Docker Mosquitto for local integration tests
├── mqtt_pub.py           CLI wrapper (→ tools/mqtt_pub.py)
├── requirements.txt      Python dependencies
├── vorlaxen/             Shared Python library
│   ├── config.py         Environment configuration
│   ├── mqtt.py           MQTT client helpers
│   └── device.py         Device REST client
├── tools/
│   └── mqtt_pub.py       Publish LCD messages via MQTT
└── tests/                Python unittest suites
    ├── test_protocol.py
    ├── test_mqtt_integration.py
    ├── test_device_api.py
    └── test_device_e2e.py
```

## Quick start

```bash
pip install -r scripts/requirements.txt
./scripts/run_tests.sh
./scripts/run_tests.sh --all --strict   # includes device tests
python3 scripts/tools/mqtt_pub.py -H 192.168.1.10 -d 123456 "Hello"
```

## Python tests only

```bash
cd scripts && python3 -m unittest discover -s tests -p 'test_*.py' -v
```
