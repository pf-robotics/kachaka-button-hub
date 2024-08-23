import { useState, useCallback } from "react";

import { getHubHost } from "./utils";

import { TopHeader } from "./TopHeader";

const HUB_HOST = getHubHost();

export function AppDev() {
  const [url, setUrl] = useState("");

  const handleOtaSubmit = useCallback(() => {
    fetch(`http://${HUB_HOST}/ota/trigger_ota_by_url`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ url }),
    });
  }, [url]);

  return (
    <>
      <TopHeader>カチャカボタンHub - 開発者UI</TopHeader>

      <div className="content">
        <h2>任意URLでOTA</h2>
        <div style={{ marginBottom: 4 }}>
          <textarea
            rows={5}
            value={url}
            onChange={(e) => setUrl(e.target.value)}
            placeholder="OTA Image URL"
            style={{ width: "90vw" }}
          />
        </div>
        <div>
          <button type="button" onClick={handleOtaSubmit} disabled={url === ""}>
            書き込む
          </button>
        </div>
      </div>
    </>
  );
}
