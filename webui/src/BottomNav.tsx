import {
  MdHome,
  MdInfo,
  MdOutlineHome,
  MdOutlineInfo,
  MdOutlineSettings,
  MdOutlineTableView,
  MdSettings,
  MdTableView,
} from "react-icons/md";

export type Page = "home" | "sheet" | "info" | "settings";

export function BottomNav({
  page,
  onPageChange,
}: {
  page: Page;
  onPageChange: (page: Page) => void;
}) {
  return (
    <div className="bottomnav">
      <div
        className={page === "home" ? "selected" : undefined}
        onClick={() => onPageChange("home")}
      >
        {page === "home" ? <MdHome /> : <MdOutlineHome />}
        <span>ホーム</span>
      </div>
      <div
        className={page === "sheet" ? "selected" : undefined}
        onClick={() => onPageChange("sheet")}
      >
        {page === "sheet" ? <MdTableView /> : <MdOutlineTableView />}
        <span>表形式</span>
      </div>
      <div
        className={page === "info" ? "selected" : undefined}
        onClick={() => onPageChange("info")}
      >
        {page === "info" ? <MdInfo /> : <MdOutlineInfo />}
        <span>ログ</span>
      </div>
      <div
        className={page === "settings" ? "selected" : undefined}
        onClick={() => onPageChange("settings")}
      >
        {page === "settings" ? <MdSettings /> : <MdOutlineSettings />}
        <span>設定</span>
      </div>
    </div>
  );
}
