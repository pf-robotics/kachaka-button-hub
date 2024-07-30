import { useState, useEffect, useCallback, useMemo } from "react";

import { App } from "./App";
import { AppDev } from "./AppDev";
import { AppWiFi } from "./AppWiFi";
import { Rebooting } from "./Rebooting";
import { Page } from "./BottomNav";

export function Router() {
  const [currentHash, setCurrentHash] = useState(window.location.hash);

  useEffect(() => {
    const handleHashChange = () => setCurrentHash(window.location.hash);
    window.addEventListener("hashchange", handleHashChange);
    return () => window.removeEventListener("hashchange", handleHashChange);
  }, []);

  const page = useMemo(() => {
    const match = currentHash.match(/^#page=(\w+)$/);
    if (match && ["home", "sheet", "info", "settings"].includes(match[1])) {
      return match[1] as Page;
    }
    return "home";
  }, [currentHash]);
  const setPage = useCallback((page: string) => {
    window.location.hash = `#page=${page}`;
    setCurrentHash(window.location.hash);
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
  return <App page={page} setPage={setPage} />;
}
