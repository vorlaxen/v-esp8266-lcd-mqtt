const statusIp = document.getElementById("statusIp");
const statusHost = document.getElementById("statusHost");
const statusSsid = document.getElementById("statusSsid");
const statusRssi = document.getElementById("statusRssi");
const statusDeviceId = document.getElementById("statusDeviceId");
const messageForm = document.getElementById("messageForm");
const messageText = document.getElementById("messageText");
const scrollEnabled = document.getElementById("scrollEnabled");
const scrollMs = document.getElementById("scrollMs");
const charCount = document.getElementById("charCount");
const lineInfo = document.getElementById("lineInfo");
const previewCanvas = document.getElementById("previewCanvas");
const previewReadout = document.getElementById("previewReadout");
const previewWrap = document.getElementById("previewWrap");
const statusMsg = document.getElementById("statusMsg");
const refreshBtn = document.getElementById("refreshBtn");
const restoreBtn = document.getElementById("restoreBtn");
const clearBtn = document.getElementById("clearBtn");

const preview = createLcdPreviewController({
  canvas: previewCanvas,
  readout: previewReadout,
  wrap: previewWrap,
  scrollMsInput: scrollMs,
  scrollEnabledInput: scrollEnabled
});

function updatePreview() {
  const chars = toChars(messageText.value);
  charCount.textContent = `${chars.length} / ${Vorlaxen.MAX_MESSAGE_LEN}`;

  const layout = preview.update(messageText.value);

  if (layout.mode === "empty") {
    lineInfo.textContent = "empty";
    return;
  }

  lineInfo.textContent =
    layout.mode === "1line" ? "1 line" :
    layout.mode === "2line" ? "2 lines (static)" :
    layout.scroll ? "single-line scroll (full text below)" :
    "2 lines (truncated)";
}

async function refreshStatus() {
  try {
    const data = await Api.getStatus();
    statusIp.textContent = data.ip || "-";
    statusHost.textContent = data.hostname || "-";
    statusHost.href = data.hostname ? `http://${data.hostname}/` : "#";
    statusSsid.textContent = data.ssid || "-";
    statusRssi.textContent = data.rssi ? `${data.rssi} dBm` : "-";
    statusDeviceId.textContent = data.deviceId || "-";
  } catch {
    setStatus(statusMsg, "Could not load status", "error");
  }
}

async function sendMessage() {
  const text = messageText.value.trim();
  if (!text) {
    setStatus(statusMsg, "Message cannot be empty", "error");
    return;
  }

  try {
    await Api.sendMessage(
      text,
      scrollEnabled.checked,
      Number(scrollMs.value) || Vorlaxen.DEFAULT_SCROLL_MS
    );
    setStatus(statusMsg, "Message sent to LCD", "ok");
  } catch {
    setStatus(statusMsg, "Could not send message", "error");
  }
}

async function restoreNetworkDisplay() {
  try {
    await Api.restoreDisplay();
    setStatus(statusMsg, "LCD restored to network info", "ok");
  } catch {
    setStatus(statusMsg, "Could not restore LCD", "error");
  }
}

messageText.addEventListener("input", updatePreview);
scrollEnabled.addEventListener("change", updatePreview);
scrollMs.addEventListener("input", updatePreview);

messageForm.addEventListener("submit", async (event) => {
  event.preventDefault();
  await sendMessage();
});

refreshBtn.addEventListener("click", refreshStatus);
restoreBtn.addEventListener("click", restoreNetworkDisplay);
clearBtn.addEventListener("click", () => {
  messageText.value = "";
  updatePreview();
  setStatus(statusMsg, "Text cleared (LCD unchanged)", "ok");
});

document.querySelectorAll(".quick-btn").forEach((button) => {
  button.addEventListener("click", () => {
    messageText.value = button.dataset.text || "";
    updatePreview();
  });
});

updatePreview();
refreshStatus();
setInterval(refreshStatus, Vorlaxen.STATUS_REFRESH_MS);
