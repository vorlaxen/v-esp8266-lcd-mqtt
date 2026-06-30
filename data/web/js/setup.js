const ssidSelect = document.getElementById("ssid");
const passwordInput = document.getElementById("password");
const setupForm = document.getElementById("setupForm");
const connectBtn = document.getElementById("connectBtn");
const scanBtn = document.getElementById("scanBtn");
const statusMsg = document.getElementById("statusMsg");

async function loadNetworks() {
  ssidSelect.innerHTML = '<option value="">Scanning...</option>';

  try {
    const data = await Api.scanNetworks();
    ssidSelect.innerHTML = "";

    if (!data.networks || data.networks.length === 0) {
      ssidSelect.innerHTML = '<option value="">No networks found</option>';
      return;
    }

    data.networks.forEach((network) => {
      if (!network.ssid) return;

      const option = document.createElement("option");
      option.value = network.ssid;
      option.textContent = `${network.ssid} (${network.rssi} dBm)`;
      ssidSelect.appendChild(option);
    });
  } catch {
    setStatus(statusMsg, "Network scan failed", "error");
  }
}

setupForm.addEventListener("submit", async (event) => {
  event.preventDefault();

  const ssid = ssidSelect.value.trim();
  const password = passwordInput.value;

  if (!ssid) {
    setStatus(statusMsg, "Select a WiFi network", "error");
    return;
  }

  connectBtn.disabled = true;
  connectBtn.textContent = "Connecting...";

  try {
    await Api.connectWifi(ssid, password);
    setStatus(statusMsg, "Saved. Device is connecting to your router...", "ok");
  } catch {
    setStatus(statusMsg, "Connection failed", "error");
    connectBtn.disabled = false;
    connectBtn.textContent = "Connect";
  }
});

scanBtn.addEventListener("click", loadNetworks);

loadNetworks();
setInterval(loadNetworks, Vorlaxen.SETUP_SCAN_MS);
