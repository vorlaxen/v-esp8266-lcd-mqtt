"""Environment-driven configuration for tests and tooling."""

from __future__ import annotations

import os
from dataclasses import dataclass


def _env_bool(name: str, default: bool) -> bool:
    raw = os.environ.get(name)
    if raw is None:
        return default
    return raw.strip().lower() in {"1", "true", "yes", "on"}


@dataclass(frozen=True)
class TestConfig:
    mqtt_broker: str
    mqtt_port: int
    mqtt_prefix: str
    mqtt_device_id: str
    mqtt_timeout_s: float
    mqtt_username: str
    mqtt_password: str
    device_host: str
    device_timeout_s: float
    strict: bool

    @classmethod
    def from_env(cls) -> TestConfig:
        return cls(
            mqtt_broker=os.environ.get("MQTT_BROKER", "127.0.0.1"),
            mqtt_port=int(os.environ.get("MQTT_PORT", "1883")),
            mqtt_prefix=os.environ.get("MQTT_PREFIX", "vorlaxen").strip("/"),
            mqtt_device_id=os.environ.get("MQTT_DEVICE_ID", "test000"),
            mqtt_timeout_s=float(os.environ.get("MQTT_TEST_TIMEOUT", "8")),
            mqtt_username=os.environ.get("MQTT_USERNAME", ""),
            mqtt_password=os.environ.get("MQTT_PASSWORD", ""),
            device_host=os.environ.get("DEVICE_HOST", os.environ.get("DEVICE_IP", "")),
            device_timeout_s=float(os.environ.get("DEVICE_TEST_TIMEOUT", "10")),
            strict=_env_bool("VORLAXEN_TEST_STRICT", False),
        )

    def topic(self, *parts: str) -> str:
        return "/".join(parts)

    def display_set_topic(self) -> str:
        return self.topic(self.mqtt_prefix, self.mqtt_device_id, "display/set")

    def display_restore_topic(self) -> str:
        return self.topic(self.mqtt_prefix, self.mqtt_device_id, "display/restore")

    def status_topic(self) -> str:
        return self.topic(self.mqtt_prefix, self.mqtt_device_id, "status")

    def availability_topic(self) -> str:
        return self.topic(self.mqtt_prefix, self.mqtt_device_id, "availability")

    @property
    def device_base_url(self) -> str:
        host = self.device_host.strip()
        if not host:
            return ""
        if host.startswith("http://") or host.startswith("https://"):
            return host.rstrip("/")
        return f"http://{host}"
