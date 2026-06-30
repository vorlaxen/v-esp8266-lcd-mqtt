"""MQTT helpers for Vorlaxen tests and tooling."""

from __future__ import annotations

import socket
import time
import uuid
from typing import Callable

try:
    import paho.mqtt.client as mqtt
    import paho.mqtt.enums as mqtt_enum
except ImportError as exc:
    raise SystemExit(
        "Missing dependency paho-mqtt. Install: pip install -r scripts/requirements.txt"
    ) from exc

from .config import TestConfig


def broker_reachable(host: str, port: int, timeout_s: float = 2.0) -> bool:
    try:
        with socket.create_connection((host, port), timeout=timeout_s):
            return True
    except OSError:
        return False


def publish_once(
    cfg: TestConfig,
    topic: str,
    payload: str | bytes,
    *,
    qos: int = 0,
    retain: bool = False,
    connect_timeout_s: float | None = None,
) -> None:
    deadline = time.time() + (connect_timeout_s or cfg.mqtt_timeout_s)
    connected = False
    publish_ok = False
    last_rc: mqtt_enum.MQTTErrorCode | None = None

    def on_connect(client, userdata, flags, reason_code, properties):
        nonlocal connected, publish_ok, last_rc
        last_rc = reason_code
        if reason_code != mqtt_enum.MQTTErrorCode.MQTT_ERR_SUCCESS:
            return
        connected = True
        info = client.publish(topic, payload, qos=qos, retain=retain)
        info.wait_for_publish(timeout=cfg.mqtt_timeout_s)
        publish_ok = not info.rc
        client.disconnect()

    client = mqtt.Client(
        callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
        client_id=f"vorlaxen-tool-{uuid.uuid4().hex[:8]}",
        protocol=mqtt.MQTTv311,
    )
    if cfg.mqtt_username:
        client.username_pw_set(cfg.mqtt_username, cfg.mqtt_password or None)
    client.on_connect = on_connect

    client.connect(cfg.mqtt_broker, cfg.mqtt_port, keepalive=30)
    client.loop_start()

    while time.time() < deadline and not connected:
        time.sleep(0.05)

    client.loop_stop()
    client.disconnect()

    if not connected:
        rc_text = last_rc.name if last_rc is not None else "unknown"
        raise RuntimeError(
            f"MQTT connect failed to {cfg.mqtt_broker}:{cfg.mqtt_port} (rc={rc_text})"
        )
    if not publish_ok:
        raise RuntimeError(f"MQTT publish failed for topic {topic}")


class MqttTestClient:
    """Subscribes to topics and collects messages for integration tests."""

    def __init__(self, cfg: TestConfig) -> None:
        self.cfg = cfg
        self._messages: dict[str, list[str]] = {}
        self._connected = False
        self._connect_error: str | None = None
        self._client = mqtt.Client(
            callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
            client_id=f"vorlaxen-test-{uuid.uuid4().hex[:8]}",
            protocol=mqtt.MQTTv311,
        )
        if cfg.mqtt_username:
            self._client.username_pw_set(cfg.mqtt_username, cfg.mqtt_password or None)
        self._client.on_connect = self._on_connect
        self._client.on_message = self._on_message

    def _on_connect(self, client, userdata, flags, reason_code, properties):
        if reason_code != mqtt_enum.MQTTErrorCode.MQTT_ERR_SUCCESS:
            self._connect_error = f"connect rc={reason_code}"
            return
        self._connected = True

    def _on_message(self, client, userdata, msg):
        payload = msg.payload.decode("utf-8", errors="replace")
        self._messages.setdefault(msg.topic, []).append(payload)

    def connect(self) -> None:
        try:
            self._client.connect(self.cfg.mqtt_broker, self.cfg.mqtt_port, keepalive=30)
        except OSError as exc:
            raise ConnectionError(
                f"Broker unavailable at {self.cfg.mqtt_broker}:{self.cfg.mqtt_port}: {exc}"
            ) from exc
        self._client.loop_start()
        self.wait_until(lambda: self._connected or self._connect_error is not None)
        if self._connect_error:
            raise ConnectionError(self._connect_error)

    def subscribe(self, topic: str, qos: int = 0) -> None:
        self._client.subscribe(topic, qos=qos)

    def publish(self, topic: str, payload: str | bytes, *, retain: bool = False, qos: int = 0):
        info = self._client.publish(topic, payload, qos=qos, retain=retain)
        info.wait_for_publish(timeout=self.cfg.mqtt_timeout_s)
        if info.rc:
            raise RuntimeError(f"publish failed rc={info.rc} topic={topic}")

    def wait_for_message(self, topic: str, timeout_s: float | None = None) -> str:
        deadline = time.time() + (timeout_s or self.cfg.mqtt_timeout_s)

        def has_message() -> bool:
            return bool(self._messages.get(topic))

        self.wait_until(has_message, timeout_s=timeout_s or self.cfg.mqtt_timeout_s)
        if time.time() > deadline and topic not in self._messages:
            raise TimeoutError(
                f"No message on {topic} within {timeout_s or self.cfg.mqtt_timeout_s}s"
            )
        return self._messages[topic][-1]

    def wait_until(self, predicate: Callable[[], bool], timeout_s: float | None = None) -> None:
        deadline = time.time() + (timeout_s or self.cfg.mqtt_timeout_s)
        while time.time() < deadline:
            if predicate():
                return
            time.sleep(0.05)
        raise TimeoutError(f"Condition not met within {timeout_s or self.cfg.mqtt_timeout_s}s")

    def messages_on(self, topic: str) -> list[str]:
        return list(self._messages.get(topic, []))

    def close(self) -> None:
        self._client.loop_stop()
        self._client.disconnect()
