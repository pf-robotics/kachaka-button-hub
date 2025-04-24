import { useState, useCallback } from "react";

import JSZip from "jszip";

import { MdFileDownload } from "react-icons/md";

import { Icon } from "./Icon";
import { getHubHttpApiEndpoint } from "./utils";

export function LogDownloadAll({
  files,
}: {
  files: string[];
}) {
  const [progress, setProgress] = useState<number | undefined>();

  const handleDownloadAll = useCallback(() => {
    const zip = new JSZip();
    const fetchNext = (i: number) => {
      if (i === files.length) {
        setProgress(100);
        zip.generateAsync({ type: "blob" }).then((zipBlob) => {
          const url = URL.createObjectURL(zipBlob);
          const a = document.createElement("a");
          a.href = url;
          a.download = "logs.zip";
          a.click();
          URL.revokeObjectURL(url);
        });
        return;
      }
      const path = files[i];
      setProgress((i / files.length) * 100);
      fetch(getHubHttpApiEndpoint("/log", { path: `/${path}` }))
        .then((res) => res.blob())
        .then((blob) => {
          zip.file(path, blob);
          fetchNext(i + 1);
        });
    };
    fetchNext(0);
  }, [files]);

  return (
    <div style={{ display: "flex", alignItems: "center", gap: 8 }}>
      <button type="button" onClick={handleDownloadAll}>
        <Icon>
          <MdFileDownload />
        </Icon>
        一括ダウンロード
      </button>
      {progress !== undefined && `${progress.toFixed(0)}% 完了`}
    </div>
  );
}
