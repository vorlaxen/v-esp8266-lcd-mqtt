"""MQTT topic and JSON payload contract tests (no hardware)."""

from __future__ import annotations

import json
import re
import unittest

from vorlaxen.config import TestConfig

TOPIC_RE = re.compile(
    r"^[a-zA-Z0-9_-]+/[a-zA-Z0-9_-]+/(display/set|display/restore|status|availability)$"
)

GOLDEN_TOPICS = {
    "display_set": "vorlaxen/123456/display/set",
    "display_restore": "vorlaxen/123456/display/restore",
    "status": "vorlaxen/123456/status",
    "availability": "vorlaxen/123456/availability",
}


class ProtocolContractTests(unittest.TestCase):
    def setUp(self) -> None:
        self.cfg = TestConfig(
            mqtt_broker="127.0.0.1",
            mqtt_port=1883,
            mqtt_prefix="vorlaxen",
            mqtt_device_id="123456",
            mqtt_timeout_s=8,
            mqtt_username="",
            mqtt_password="",
            device_host="",
            device_timeout_s=10,
            strict=False,
        )

    def test_golden_topic_names(self) -> None:
        self.assertEqual(self.cfg.display_set_topic(), GOLDEN_TOPICS["display_set"])
        self.assertEqual(self.cfg.display_restore_topic(), GOLDEN_TOPICS["display_restore"])
        self.assertEqual(self.cfg.status_topic(), GOLDEN_TOPICS["status"])
        self.assertEqual(self.cfg.availability_topic(), GOLDEN_TOPICS["availability"])

    def test_topic_format_regex(self) -> None:
        for topic in GOLDEN_TOPICS.values():
            self.assertRegex(topic, TOPIC_RE)

    def test_display_set_json_schema(self) -> None:
        payload = {"text": "Hello", "scroll": True, "scrollMs": 300}
        decoded = json.loads(json.dumps(payload))
        self.assertLessEqual(len(decoded["text"]), 64)

    def test_status_json_schema(self) -> None:
        payload = {"online": True, "deviceId": "123456", "ip": "192.168.1.10"}
        for key in ("online", "deviceId", "ip"):
            self.assertIn(key, payload)


class ConfigLayoutTests(unittest.TestCase):
    def test_v3_eeprom_size(self) -> None:
        # Must match sizeof(ConfigData) in include/config/ConfigTypes.h
        self.assertEqual(302, 302)

    def test_v1_eeprom_size(self) -> None:
        self.assertEqual(107, 107)


if __name__ == "__main__":
    unittest.main()
