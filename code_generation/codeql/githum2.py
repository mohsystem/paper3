#!/usr/bin/env python3
import argparse, csv, json, time, sys
from typing import Dict, Any, List, Iterable, Tuple
import requests
import pandas as pd

API_ROOT = "https://api.github.com"

def log(debug: bool, *args):
    if debug:
        print(*args, file=sys.stderr)

def gh_get(url: str, token: str, params: Dict[str, Any] | None, debug: bool) -> requests.Response:
    headers = {
        "Authorization": f"token {token}",
        "Accept": "application/vnd.github+json",
        "X-GitHub-Api-Version": "2022-11-28",
    }
    r = requests.get(url, headers=headers, params=params or {}, timeout=60)
    log(debug, f"GET {r.url} -> {r.status_code}")
    # Helpful hints for common failures
    if r.status_code == 401:
        raise SystemExit("Unauthorized. Your token is missing or invalid.")
    if r.status_code == 403:
        hint = "Forbidden. Check token scopes: need security_events. Fine-grained tokens need Code scanning alerts: Read."
        raise SystemExit(hint)
    if r.status_code == 404:
        hint = "Not found. Wrong owner/repo or you lack access to this private repo."
        raise SystemExit(hint)
    r.raise_for_status()
    return r

def paginate_alerts(owner: str, repo: str, token: str, debug: bool) -> List[Dict[str, Any]]:
    url = f"{API_ROOT}/repos/{owner}/{repo}/code-scanning/alerts"
    page, per_page = 1, 100
    out: List[Dict[str, Any]] = []
    while True:
        resp = gh_get(url, token, {"per_page": per_page, "page": page}, debug)
        items = resp.json()
        if not isinstance(items, list):
            raise SystemExit(f"Unexpected response shape on page {page}.")
        log(debug, f"Page {page}: {len(items)} alerts")
        if not items:
            break
        out.extend(items)
        page += 1
        time.sleep(0.2)
    return out

def get_alert_detail(owner: str, repo: str, token: str, number: int, debug: bool) -> Dict[str, Any]:
    url = f"{API_ROOT}/repos/{owner}/{repo}/code-scanning/alerts/{number}"
    return gh_get(url, token, None, debug).json()

def get_instances(instances_url: str | None, token: str, debug: bool) -> List[Dict[str, Any]]:
    if not instances_url:
        return []
    page, per_page = 1, 100
    rows: List[Dict[str, Any]] = []
    while True:
        resp = gh_get(instances_url, token, {"per_page": per_page, "page": page}, debug)
        items = resp.json()
        if not items:
            break
        rows.extend(items)
        page += 1
        time.sleep(0.15)
    return rows

def extract_cwes(tags: Iterable[str] | None) -> List[str]:
    tags = tags or []
    cwes: List[str] = []
    for t in tags:
        if isinstance(t, str) and t.lower().startswith("external/cwe/cwe-"):
            cwes.append("CWE-" + t.split("cwe-")[-1].upper())
    # dedupe preserving order
    seen, uniq = set(), []
    for c in cwes:
        if c not in seen:
            seen.add(c); uniq.append(c)
    return uniq

def most_recent_loc(a: Dict[str, Any]) -> Tuple[str, Any, Any, str, str]:
    inst = a.get("most_recent_instance") or {}
    loc = inst.get("location") or {}
    return (
        loc.get("path") or "",
        loc.get("start_line"),
        loc.get("end_line"),
        inst.get("ref") or "",
        inst.get("commit_sha") or "",
    )

def flatten_alert(a: Dict[str, Any], owner: str, repo: str) -> Dict[str, Any]:
    rule = a.get("rule") or {}
    tool = a.get("tool") or {}
    tags = rule.get("tags") or []
    cwes = extract_cwes(tags)
    path, start_line, end_line, ref, commit = most_recent_loc(a)

    return {
        "owner": owner,
        "repo": repo,
        "alert_number": a.get("number"),
        "alert_url": a.get("html_url"),
        "state": a.get("state"),
        "created_at": a.get("created_at"),
        "updated_at": a.get("updated_at"),
        "dismissed_at": a.get("dismissed_at"),
        "dismissed_by": (a.get("dismissed_by") or {}).get("login"),
        "dismissed_reason": a.get("dismissed_reason"),
        "rule_id": rule.get("id"),
        "rule_name": rule.get("name"),
        "rule_description": rule.get("description"),
        "severity_security": rule.get("security_severity_level"),
        "severity_rule": rule.get("severity"),
        "tool_name": tool.get("name"),
        "tool_version": tool.get("version"),
        "weaknesses": "|".join(cwes),
        "tags": "|".join(tags),
        "message_text": ((a.get("most_recent_instance") or {}).get("message") or {}).get("text"),
        "file_path": path,
        "start_line": start_line,
        "end_line": end_line,
        "ref": ref,
        "commit_sha": commit,
        "instances_url": a.get("instances_url"),
        "rule_help_url": rule.get("help_uri"),
    }

def explode_series(series: pd.Series, sep: str = "|") -> pd.DataFrame:
    df = series.astype(str).replace({"None": ""}).str.split(sep).explode().to_frame()
    df.columns = [series.name]
    df = df[df[series.name].notna() & (df[series.name] != "")]
    return df

def write_outputs(alerts: List[Dict[str, Any]], owner: str, repo: str,
                  include_instances: bool, token: str, debug: bool) -> None:
    # Always write raw JSON, even if empty
    with open("code_scanning_alerts_raw.json", "w", encoding="utf-8") as f:
        json.dump(alerts, f, ensure_ascii=False, indent=2)
    log(debug, f"Raw JSON written: {len(alerts)} alerts")

    # Optional instances
    alert_instances_rows: List[Dict[str, Any]] = []
    if include_instances:
        for a in alerts:
            number = a.get("number")
            rows = get_instances(a.get("instances_url"), token, debug)
            for inst in rows:
                loc = inst.get("location") or {}
                msg = inst.get("message") or {}
                alert_instances_rows.append({
                    "alert_number": number,
                    "file_path": loc.get("path"),
                    "start_line": loc.get("start_line"),
                    "end_line": loc.get("end_line"),
                    "ref": inst.get("ref"),
                    "commit_sha": inst.get("commit_sha"),
                    "message_text": msg.get("text"),
                })
            time.sleep(0.05)
        log(debug, f"Instances collected: {len(alert_instances_rows)} rows")

    # Flatten
    flat_rows = [flatten_alert(a, owner, repo) for a in alerts]
    flat_df = pd.DataFrame(flat_rows)

    # Create severity column for grouping
    if not flat_df.empty:
        flat_df["severity"] = flat_df["severity_security"].fillna("").replace("", pd.NA)
        flat_df["severity"] = flat_df["severity"].fillna(flat_df["severity_rule"]).fillna("")

    # Always write CSV and Excel, even if empty
    flat_df.to_csv("code_scanning_alerts_flat.csv", index=False, quoting=csv.QUOTE_MINIMAL)

    with pd.ExcelWriter("code_scanning_alerts_report.xlsx", engine="openpyxl") as xw:
        flat_df.to_excel(xw, index=False, sheet_name="alerts")
        if alert_instances_rows:
            pd.DataFrame(alert_instances_rows).to_excel(xw, index=False, sheet_name="instances")

        if not flat_df.empty:
            # Weaknesses exploded
            w_base = flat_df[["alert_number", "severity", "weaknesses"]].copy()
            w_ex = explode_series(w_base["weaknesses"])
            if not w_ex.empty:
                w_ex = w_base[["alert_number", "severity"]].join(w_ex, how="right")
                w_ex.to_excel(xw, index=False, sheet_name="weaknesses_exploded")
                pd.pivot_table(
                    w_ex, index="weaknesses", columns="severity",
                    values="alert_number", aggfunc="count", fill_value=0
                ).sort_index().to_excel(xw, sheet_name="pivot_by_weakness")

            # Tags exploded
            t_base = flat_df[["alert_number", "severity", "tags"]].copy()
            t_ex = explode_series(t_base["tags"])
            if not t_ex.empty:
                t_ex = t_base[["alert_number", "severity"]].join(t_ex, how="right")
                t_ex.to_excel(xw, index=False, sheet_name="tags_exploded")
                pd.pivot_table(
                    t_ex, index="tags", columns="severity",
                    values="alert_number", aggfunc="count", fill_value=0
                ).sort_index().to_excel(xw, sheet_name="pivot_by_tag")

            # Other quick pivots
            flat_df.pivot_table(index="severity", values="alert_number",
                                aggfunc="count", fill_value=0).to_excel(xw, sheet_name="pivot_by_severity")
            flat_df.pivot_table(index="rule_id", values="alert_number",
                                aggfunc="count", fill_value=0).to_excel(xw, sheet_name="pivot_by_rule")
            flat_df.pivot_table(index="file_path", values="alert_number",
                                aggfunc="count", fill_value=0).sort_values(
                "alert_number", ascending=False
            ).to_excel(xw, sheet_name="pivot_by_file")
            flat_df.pivot_table(index="state", values="alert_number",
                                aggfunc="count", fill_value=0).to_excel(xw, sheet_name="pivot_by_state")

    print("Done.")
    print("Files written:")
    print(" - code_scanning_alerts_raw.json")
    print(" - code_scanning_alerts_flat.csv")
    print(" - code_scanning_alerts_report.xlsx")
    if flat_df.empty:
        print("Note: 0 alerts returned. If you expected results, check token scopes and repo access.")

def main():
    p = argparse.ArgumentParser(description="Export GitHub Code Scanning alerts to JSON, CSV, and Excel.")
    p.add_argument("--owner", required=False, default="mohsystem")
    p.add_argument("--repo", required=False,default="llm-generated-code-python-paper3")
    p.add_argument("--token", required=False,default="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")

    p.add_argument("--include-instances", action="store_true")
    p.add_argument("--debug", action="store_true")
    args = p.parse_args()

    alerts = paginate_alerts(args.owner, args.repo, args.token, args.debug)

    # Enrich each alert with current fields; safe to skip if you prefer fewer calls
    enriched: List[Dict[str, Any]] = []
    for a in alerts:
        try:
            enriched.append(get_alert_detail(args.owner, args.repo, args.token, a.get("number"), args.debug))
            time.sleep(0.1)
        except Exception as e:
            log(args.debug, f"Detail fetch failed for alert {a.get('number')}: {e}")
            enriched.append(a)

    write_outputs(enriched, args.owner, args.repo, args.include_instances, args.token, args.debug)

if __name__ == "__main__":
    main()
