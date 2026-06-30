/**
 * REST API client for the Vorlaxen panel
 */
const Api = {
  async getStatus() {
    return fetchJson("/api/status");
  },

  async sendMessage(text, scroll, scrollMs) {
    const data = await postJson("/api/message", { text, scroll, scrollMs });
    if (!data.ok) throw new Error(data.error || "send_failed");
    return data;
  },

  async restoreDisplay() {
    const data = await postJson("/api/display/restore", {});
    if (!data.ok) throw new Error("restore_failed");
    return data;
  },

  async scanNetworks() {
    return fetchJson("/api/setup/scan");
  },

  async connectWifi(ssid, password) {
    const data = await postJson("/api/setup/connect", { ssid, password });
    if (!data.ok) throw new Error(data.error || "connect_failed");
    return data;
  }
};
