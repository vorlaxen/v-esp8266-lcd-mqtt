/**
 * Shared UI helpers
 */
function setStatus(element, message, type = "") {
  element.textContent = message;
  element.className = `status-msg ${type}`.trim();
}

async function fetchJson(url, options = {}) {
  const response = await fetch(url, options);
  const data = await response.json();
  if (!response.ok) {
    throw new Error(data.error || "request_failed");
  }
  return data;
}

async function postJson(url, body) {
  return fetchJson(url, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(body)
  });
}
