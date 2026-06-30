"""Live device REST API smoke tests."""

from __future__ import annotations

import unittest

from vorlaxen.config import TestConfig
from vorlaxen.device import DeviceClient


class DeviceApiTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.cfg = TestConfig.from_env()
        if not cls.cfg.device_base_url:
            raise unittest.SkipTest("Set DEVICE_HOST or DEVICE_IP")
        cls.client = DeviceClient(cls.cfg)

    def test_root_html(self) -> None:
        self.assertIn("<html", self.client.get_root_html().lower())

    def test_status_fields(self) -> None:
        status = self.client.get_status()
        for key in ("ip", "hostname", "deviceId", "ssid", "rssi"):
            self.assertIn(key, status)

    def test_mqtt_fields(self) -> None:
        mqtt = self.client.get_mqtt()
        for key in ("enabled", "broker", "port", "topicPrefix", "connected"):
            self.assertIn(key, mqtt)

    def test_post_message(self) -> None:
        self.assertTrue(self.client.post_message("API smoke", scroll=False).get("ok"))

    def test_post_restore(self) -> None:
        self.assertTrue(self.client.post_display_restore().get("ok"))

    def test_empty_message_rejected(self) -> None:
        with self.assertRaises(RuntimeError):
            self.client.post_message("   ", scroll=False)


if __name__ == "__main__":
    unittest.main()
