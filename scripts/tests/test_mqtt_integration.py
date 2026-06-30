"""MQTT integration tests against a live Mosquitto broker."""

from __future__ import annotations

import json
import unittest

from vorlaxen.config import TestConfig
from vorlaxen.mqtt import MqttTestClient, broker_reachable


class MqttIntegrationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.cfg = TestConfig.from_env()
        if not broker_reachable(cls.cfg.mqtt_broker, cls.cfg.mqtt_port):
            msg = f"Broker unreachable at {cls.cfg.mqtt_broker}:{cls.cfg.mqtt_port}"
            if cls.cfg.strict:
                raise AssertionError(msg)
            raise unittest.SkipTest(msg)

    def setUp(self) -> None:
        self.client = MqttTestClient(self.cfg)
        self.client.connect()
        self.client.subscribe(self.cfg.status_topic())
        self.client.subscribe(self.cfg.availability_topic())

    def tearDown(self) -> None:
        self.client.close()

    def test_status_roundtrip(self) -> None:
        body = {"online": True, "deviceId": self.cfg.mqtt_device_id, "ip": "127.0.0.1"}
        self.client.publish(self.cfg.status_topic(), json.dumps(body), retain=True)
        received = json.loads(self.client.wait_for_message(self.cfg.status_topic()))
        self.assertEqual(received["deviceId"], body["deviceId"])

    def test_availability_roundtrip(self) -> None:
        self.client.publish(self.cfg.availability_topic(), "online", retain=True)
        self.assertEqual("online", self.client.wait_for_message(self.cfg.availability_topic()))

    def test_display_set_json(self) -> None:
        self.client.publish(
            self.cfg.display_set_topic(),
            json.dumps({"text": "Integration", "scroll": False}),
        )

    def test_display_set_plain(self) -> None:
        self.client.publish(self.cfg.display_set_topic(), "Plain text")

    def test_display_restore_empty(self) -> None:
        self.client.publish(self.cfg.display_restore_topic(), "")


if __name__ == "__main__":
    unittest.main()
