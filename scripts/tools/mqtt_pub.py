#!/usr/bin/env python3
"""Publish an MQTT message to a Vorlaxen device."""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from vorlaxen.config import TestConfig
from vorlaxen.mqtt import broker_reachable, publish_once


def parse_args() -> argparse.Namespace:
    cfg = TestConfig.from_env()
    parser = argparse.ArgumentParser(description="Publish to Vorlaxen MQTT topics")
    parser.add_argument("-H", "--host", default=cfg.mqtt_broker)
    parser.add_argument("-p", "--port", type=int, default=cfg.mqtt_port)
    parser.add_argument("-d", "--device-id", default=cfg.mqtt_device_id)
    parser.add_argument("--prefix", default=cfg.mqtt_prefix)
    parser.add_argument("--restore", action="store_true")
    parser.add_argument("--scroll", action="store_true", default=False)
    parser.add_argument("--scroll-ms", type=int, default=300)
    parser.add_argument("--timeout", type=float, default=cfg.mqtt_timeout_s)
    parser.add_argument("--check-broker", action="store_true")
    parser.add_argument("text", nargs="?", default="")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    prefix = args.prefix.strip("/")
    device_id = args.device_id

    if args.restore:
        topic = f"{prefix}/{device_id}/display/restore"
        payload = ""
    else:
        if not args.text:
            print("Error: provide text or use --restore", file=sys.stderr)
            return 2
        topic = f"{prefix}/{device_id}/display/set"
        payload = json.dumps(
            {"text": args.text, "scroll": args.scroll, "scrollMs": args.scroll_ms}
        )

    if args.check_broker and not broker_reachable(args.host, args.port):
        print(f"Error: broker unreachable at {args.host}:{args.port}", file=sys.stderr)
        return 1

    cfg = TestConfig(
        mqtt_broker=args.host,
        mqtt_port=args.port,
        mqtt_prefix=prefix,
        mqtt_device_id=device_id,
        mqtt_timeout_s=args.timeout,
        mqtt_username="",
        mqtt_password="",
        device_host="",
        device_timeout_s=10,
        strict=True,
    )

    print(f"Publishing to mqtt://{args.host}:{args.port}/{topic}")
    try:
        publish_once(cfg, topic, payload, connect_timeout_s=args.timeout)
    except (RuntimeError, ConnectionError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1

    print("Done.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
