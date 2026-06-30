"""HTTP client for Vorlaxen device REST API smoke tests."""

from __future__ import annotations

import json
import urllib.error
import urllib.request
from typing import Any

from .config import TestConfig


class DeviceClient:
    def __init__(self, cfg: TestConfig) -> None:
        if not cfg.device_base_url:
            raise ValueError("DEVICE_HOST or DEVICE_IP is required")
        self.base_url = cfg.device_base_url
        self.timeout_s = cfg.device_timeout_s

    def _request(
        self,
        method: str,
        path: str,
        body: dict[str, Any] | None = None,
        *,
        expect_ok: bool = True,
    ) -> dict[str, Any]:
        url = f"{self.base_url}{path}"
        data = None
        headers = {"Accept": "application/json"}
        if body is not None:
            data = json.dumps(body).encode("utf-8")
            headers["Content-Type"] = "application/json"

        req = urllib.request.Request(url, data=data, headers=headers, method=method)
        try:
            with urllib.request.urlopen(req, timeout=self.timeout_s) as resp:
                raw = resp.read().decode("utf-8")
                payload = json.loads(raw) if raw else {}
                if expect_ok and isinstance(payload, dict) and payload.get("ok") is False:
                    raise RuntimeError(f"{method} {path} returned ok=false: {payload}")
                return payload
        except urllib.error.HTTPError as exc:
            detail = exc.read().decode("utf-8", errors="replace")
            raise RuntimeError(f"{method} {path} HTTP {exc.code}: {detail}") from exc
        except urllib.error.URLError as exc:
            raise ConnectionError(f"{method} {path} failed: {exc.reason}") from exc

    def get_status(self) -> dict[str, Any]:
        return self._request("GET", "/api/status", expect_ok=False)

    def get_mqtt(self) -> dict[str, Any]:
        return self._request("GET", "/api/mqtt", expect_ok=False)

    def post_message(self, text: str, *, scroll: bool = False, scroll_ms: int = 300) -> dict[str, Any]:
        return self._request(
            "POST",
            "/api/message",
            {"text": text, "scroll": scroll, "scrollMs": scroll_ms},
        )

    def post_display_restore(self) -> dict[str, Any]:
        return self._request("POST", "/api/display/restore", {})

    def post_mqtt(self, settings: dict[str, Any]) -> dict[str, Any]:
        return self._request("POST", "/api/mqtt", settings)

    def get_root_html(self) -> str:
        req = urllib.request.Request(f"{self.base_url}/", method="GET")
        try:
            with urllib.request.urlopen(req, timeout=self.timeout_s) as resp:
                return resp.read().decode("utf-8", errors="replace")
        except urllib.error.URLError as exc:
            raise ConnectionError(f"GET / failed: {exc.reason}") from exc
