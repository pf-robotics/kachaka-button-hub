import { useState, useEffect } from "react";

import { App } from "./App";
import { AppDev } from "./AppDev";
import { AppWiFi } from "./AppWiFi";
import { Rebooting } from "./Rebooting";

export function Router() {
  const [currentHash, setCurrentHash] = useState(window.location.hash);

  useEffect(() => {
    const handleHashChange = () => setCurrentHash(window.location.hash);
    window.addEventListener("hashchange", handleHashChange);
    return () => window.removeEventListener("hashchange", handleHashChange);
  }, []);

  if (currentHash === "#reboot") {
    return <Rebooting />;
  }
  if (currentHash === "#reboot-apply") {
    return <Rebooting apply />;
  }
  if (currentHash === "#wifi") {
    return <AppWiFi />;
  }
  if (currentHash === "#dev") {
    return <AppDev />;
  }
  return <App />;
}
