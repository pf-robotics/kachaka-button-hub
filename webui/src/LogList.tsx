import { useEffect, useState, useCallback, useMemo } from "react";

import { MdDelete } from "react-icons/md";
import { MdFileDownload } from "react-icons/md";
import { FaRegFile } from "react-icons/fa";

import { Icon } from "./Icon";
import { Box } from "./Box";
import { Modal } from "./Modal";
import { LogDownloadAll } from "./LogDownloadAll";

function LogItem({
  hubHost,
  path,
  size,
  onDelete,
}: {
  hubHost: string;
  path: string;
  size: number;
  onDelete: () => void;
}) {
  const [openConfirm, setOpenConfirm] = useState(false);

  const handleDownload = useCallback(
    () =>
      window.open(`http://${hubHost}/log?path=/${path}&download=true`, "_self"),
    [hubHost, path],
  );

  return (
    <Box style={{ display: "flex", flexDirection: "row", maxWidth: 300 }}>
      <a
        href={`http://${hubHost}/log?path=/${path}`}
        style={{
          textDecoration: "none",
          color: "inherit",
          display: "flex",
          flexDirection: "row",
          alignItems: "center",
          paddingRight: 16,
          gap: 16,
          flex: 1,
        }}
      >
        <Icon>
          <FaRegFile />
        </Icon>
        <span
          style={{
            display: "flex",
            flexDirection: "column",
            gap: 4,
          }}
        >
          <span style={{ fontWeight: "bold" }}>{path}</span>
          <span style={{ fontSize: "small", color: "var(--kachaka-gray-4)" }}>
            {(size / 1024).toFixed(1)} KB
          </span>
        </span>
      </a>
      <button type="button" className="icon" onClick={handleDownload}>
        <Icon>
          <MdFileDownload />
        </Icon>
      </button>
      <button
        type="button"
        className="icon"
        onClick={() => setOpenConfirm(true)}
      >
        <Icon>
          <MdDelete />
        </Icon>
      </button>
      <Modal open={openConfirm} onClose={() => setOpenConfirm(false)}>
        <p>本当に削除しますか?</p>
        <div style={{ display: "flex", justifyContent: "flex-end", gap: 16 }}>
          <button type="button" onClick={onDelete}>
            削除
          </button>
          <button type="button" onClick={() => setOpenConfirm(false)}>
            いいえ
          </button>
        </div>
      </Modal>
    </Box>
  );
}

interface LogListResponse {
  total_bytes: number;
  used_bytes: number;
  files: { [path: string]: number };
}

export function LogList({
  hubHost,
  onDelete,
}: {
  hubHost: string;
  onDelete: (path: string) => Promise<void>;
}) {
  const [response, setResponse] = useState<LogListResponse>();

  useEffect(() => {
    fetch(`http://${hubHost}/log`)
      .then((res) => res.json())
      .then((data) => setResponse(data));
  }, [hubHost]);

  const files = useMemo(
    () => (response === undefined ? [] : Object.keys(response.files).sort()),
    [response],
  );

  const handleDeleteLog = useCallback(
    (path: string) =>
      onDelete(`/${path}`).then(() =>
        setResponse((prev) => {
          if (prev === undefined) return prev;
          const newLogs = { ...prev, files: { ...prev.files } };
          delete newLogs.files[path];
          return newLogs;
        }),
      ),
    [onDelete],
  );

  return (
    <>
      {response === undefined ? (
        <p>読み込み中...</p>
      ) : (
        <>
          <div style={{ marginLeft: 16 }}>
            <LogDownloadAll hubHost={hubHost} files={files} />
          </div>
          <div style={{ display: "flex", flexFlow: "wrap row" }}>
            {Object.entries(response.files)
              .sort(([a], [b]) => (a < b ? 1 : -1))
              .map(([path, size]) => (
                <LogItem
                  key={path}
                  hubHost={hubHost}
                  path={path}
                  size={size}
                  onDelete={() => handleDeleteLog(path)}
                />
              ))}
          </div>
          <div
            style={{
              marginLeft: 16,
              fontSize: "small",
              color: "var(--kachaka-gray-4)",
            }}
          >
            <div>
              使用中: {Math.round(response.used_bytes / 1024).toLocaleString()}{" "}
              KB (
              {((response.used_bytes / response.total_bytes) * 100).toFixed(1)}
              %)
            </div>
            <div>
              総容量: {Math.round(response.total_bytes / 1024).toLocaleString()}{" "}
              KB
            </div>
          </div>
        </>
      )}
    </>
  );
}
