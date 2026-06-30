"""Device + MQTT end-to-end tests."""

from __future__ import annotations

import json
import time
import unittest

from vorlaxen.config import TestConfig
from vorlaxen.device import DeviceClient
from vorlaxen.mqtt import MqttTestClient, broker_reachable, publish_once


class DeviceMqttE2ETests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.cfg = TestConfig.from_env()
        if not cls.cfg.device_base_url:
            raise unittest.SkipTest("Set DEVICE_HOST or DEVICE_IP")
        if not broker_reachable(cls.cfg.mqtt_broker, cls.cfg.mqtt_port):
            raise unittest.SkipTest("Broker unreachable")
        cls.http = DeviceClient(cls.cfg)

    def test_device_mqtt_connected(self) -> None:
        mqtt = self.http.get_mqtt()
        self.assertTrue(mqtt.get("enabled"))
        self.assertTrue(mqtt.get("connected"))

    def test_mqtt_display_set(self) -> None:
        device_id = self.http.get_status()["deviceId"]
        self.assertEqual(device_id, self.cfg.mqtt_device_id)

        publish_once(
            self.cfg,
            self.cfg.display_set_topic(),
            json.dumps({"text": f"E2E {int(time.time())}", "scroll": False}),
        )
        self.assertTrue(self.http.post_message("E2E verify", scroll=False).get("ok"))

    def test_mqtt_restore(self) -> None:
        publish_once(self.cfg, self.cfg.display_restore_topic(), "")
        self.assertTrue(self.http.post_display_restore().get("ok"))


if __name__ == "__main__":
    unittest.main()
