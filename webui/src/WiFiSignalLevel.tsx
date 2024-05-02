import { MdOutlineSignalWifiBad } from "react-icons/md";
import { MdOutlineSignalWifi0Bar } from "react-icons/md";
import { MdNetworkWifi1Bar } from "react-icons/md";
import { MdNetworkWifi2Bar } from "react-icons/md";
import { MdNetworkWifi3Bar } from "react-icons/md";
import { MdNetworkWifi } from "react-icons/md";
import { MdOutlineSignalWifiStatusbar4Bar } from "react-icons/md";

export function WiFiSignalLevel({
  online,
  rssi,
}: {
  online: boolean;
  rssi: number | undefined;
}) {
  const title = `WiFi Signal Strength: ${rssi} dBm`;
  if (!online || rssi === undefined) return <MdOutlineSignalWifiBad />;
  if (rssi <= -90) return <MdOutlineSignalWifi0Bar title={title} />;
  if (rssi <= -80) return <MdNetworkWifi1Bar title={title} />;
  if (rssi <= -70) return <MdNetworkWifi2Bar title={title} />;
  if (rssi <= -67) return <MdNetworkWifi3Bar title={title} />;
  if (rssi <= -55) return <MdNetworkWifi title={title} />;
  return <MdOutlineSignalWifiStatusbar4Bar title={title} />;
}
