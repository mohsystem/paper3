import os
import math
import requests
import pandas as pd
from datetime import datetime

SONARCLOUD_URL = "https://sonarcloud.io"
PROJECT_KEY = "mohkharma_llm-generated-code-c-paper3"
ORGANIZATION = PROJECT_KEY.split("_")[0]  # "mohkharma"

MAPPING_FILE = "task-source-language-mapping.xlsx"

# Set this to the language you want to include (must match mapping file)
TARGET_LANGUAGE = "C"  # <<< CHANGE THIS IF NEEDED

# Dynamic file name: sonarcloud_report_<project_key>_YYYYMMDD.xlsx
DATE_STR = datetime.now().strftime("%Y%m%d")
OUTPUT_FILE = f"sonarcloud_report_{PROJECT_KEY}_{DATE_STR}_lang_mapping.xlsx"

TOKEN = os.getenv("SONARCLOUD_TOKEN")
HEADERS = {}
if TOKEN:
    HEADERS["Authorization"] = f"Bearer {TOKEN}"


def fetch_measures(project_key: str) -> dict:
    metrics = [
        "bugs",
        "vulnerabilities",
        "code_smells",
        "security_hotspots",
        "coverage",
        "duplicated_lines_density",
        "ncloc",
        "sqale_index",
        "reliability_rating",
        "security_rating",
        "sqale_rating",
    ]
    params = {"component": project_key, "metricKeys": ",".join(metrics)}
    url = f"{SONARCLOUD_URL}/api/measures/component"
    resp = requests.get(url, params=params, headers=HEADERS, timeout=30)
    resp.raise_for_status()
    data = resp.json()
    measures = {
        m["metric"]: m.get("value")
        for m in data.get("component", {}).get("measures", [])
    }
    measures["project_key"] = project_key
    return measures


def fetch_all_issues(project_key: str) -> list:
    """Fetch only OPEN vulnerability issues."""
    issues = []
    page_size = 500
    page = 1

    while True:
        params = {
            "componentKeys": project_key,
            "types": "VULNERABILITY",
            "statuses": "OPEN",
            "ps": page_size,
            "p": page,
        }
        url = f"{SONARCLOUD_URL}/api/issues/search"
        resp = requests.get(url, params=params, headers=HEADERS, timeout=60)
        resp.raise_for_status()
        data = resp.json()

        page_issues = data.get("issues", [])
        if not page_issues:
            break

        issues.extend(page_issues)

        paging = data.get("paging", {})
        total = paging.get("total", 0)
        pages = math.ceil(total / page_size)
        if page >= pages:
            break

        page += 1

    return issues


def fetch_rule_metadata(rule_key: str, cache: dict) -> dict:
    """Fetch rule metadata (CWE code only) from SonarCloud with caching."""
    if rule_key in cache:
        return cache[rule_key]

    url = f"{SONARCLOUD_URL}/api/rules/show"
    params = {"organization": ORGANIZATION, "key": rule_key}
    resp = requests.get(url, params=params, headers=HEADERS, timeout=20)

    if resp.status_code != 200:
        cache[rule_key] = {"cwe": ""}
        return cache[rule_key]

    data = resp.json().get("rule", {})
    security_standards = data.get("securityStandards", [])

    cwe_code = ""
    for s in security_standards:
        s_lower = s.lower()
        if s_lower.startswith("cwe:"):
            number = s.split(":", 1)[1].strip()
            if number:
                cwe_code = f"CWE-{number}"
                break

    cache[rule_key] = {"cwe": cwe_code}
    return cache[rule_key]


def fetch_analyses(project_key: str) -> list:
    analyses = []
    page_size = 100
    page = 1

    while True:
        params = {"project": project_key, "ps": page_size, "p": page}
        url = f"{SONARCLOUD_URL}/api/project_analyses/search"
        resp = requests.get(url, params=params, headers=HEADERS, timeout=30)

        if resp.status_code == 403:
            break

        resp.raise_for_status()
        data = resp.json()
        page_analyses = data.get("analyses", [])
        if not page_analyses:
            break

        analyses.extend(page_analyses)

        paging = data.get("paging", {})
        total = paging.get("total", 0)
        pages = math.ceil(total / page_size)
        if page >= pages:
            break

        page += 1

    return analyses


def split_component_folders(component: str):
    """
    Return (file_name, folder_1, folder_2, folder_3)
    folder_1..3 are the last three folders before the file.
    """
    if not component:
        return "", "", "", ""

    if ":" in component:
        component = component.split(":", 1)[1]

    parts = component.strip("/").split("/")
    if not parts:
        return "", "", "", ""

    file_name = parts[-1]
    folder_parts = parts[:-1]

    last_three = folder_parts[-3:]
    last_three = [""] * (3 - len(last_three)) + last_three

    folder_1, folder_2, folder_3 = last_three
    return file_name, folder_1, folder_2, folder_3


def flatten_issues(raw_issues: list) -> pd.DataFrame:
    """Create Issues dataframe: one row per issue with CWE code, folders, file name, etc."""
    records = []
    rule_cache = {}

    for issue in raw_issues:
        rule_key = issue.get("rule", "")
        rule_meta = fetch_rule_metadata(rule_key, rule_cache)
        text_range = issue.get("textRange") or {}

        cwe = rule_meta.get("cwe", "")  # "CWE-327" or ""

        component = issue.get("component")
        file_name, folder_1, folder_2, folder_3 = split_component_folders(component)

        records.append(
            {
                "key": issue.get("key"),
                "rule": rule_key,
                "security_category": cwe,
                "severity": issue.get("severity"),
                "status": issue.get("status"),
                "resolution": issue.get("resolution"),
                "component": component,
                "folder_1": folder_1,
                "folder_2": folder_2,
                "folder_3": folder_3,
                "file_name": file_name,
                "message": issue.get("message"),
                "tags": ",".join(issue.get("tags") or []),
                "creationDate": issue.get("creationDate"),
                "updateDate": issue.get("updateDate"),
                "line": issue.get("line"),
                "startLine": text_range.get("startLine"),
                "endLine": text_range.get("endLine"),
                "project": issue.get("project"),
                "effort": issue.get("effort"),
                "assignee": issue.get("assignee"),
                "author": issue.get("author"),
            }
        )

    return pd.DataFrame(records)


def apply_language_mapping(issues_df: pd.DataFrame) -> pd.DataFrame:
    """
    Filter issues by TARGET_LANGUAGE using task-source-language-mapping.xlsx.
    - Join on file_name
    - Keep only rows where Language == TARGET_LANGUAGE
    """
    if issues_df.empty:
        return issues_df

    if not TARGET_LANGUAGE:
        return issues_df

    if not os.path.exists(MAPPING_FILE):
        raise FileNotFoundError(
            f"Mapping file '{MAPPING_FILE}' not found. Place it next to the script or update MAPPING_FILE."
        )

    mapping = pd.read_excel(MAPPING_FILE)

    if "file_name" not in mapping.columns or "Language" not in mapping.columns:
        raise ValueError(
            f"Mapping file '{MAPPING_FILE}' must contain 'file_name' and 'Language' columns."
        )

    mapping["file_name"] = mapping["file_name"].astype(str).str.strip()
    mapping["Language"] = mapping["Language"].astype(str).str.strip()
    issues_df["file_name"] = issues_df["file_name"].astype(str).str.strip()

    mapping_filtered = mapping[mapping["Language"] == TARGET_LANGUAGE].copy()

    if mapping_filtered.empty:
        # No files for that language: return empty df with same columns.
        return issues_df.iloc[0:0].copy()

    # Inner join: keep only issues for mapped files in that language.
    filtered = issues_df.merge(
        mapping_filtered[["file_name", "Language"]],
        on="file_name",
        how="inner",
    )

    return filtered


def build_summary_pivot(issues_df: pd.DataFrame) -> pd.DataFrame:
    """
    Build Summary sheet.

    Rows:
      Group (folder_1)
      LLM name (folder_3)
      prompt method (folder_2)

    For each CWE:
      <CWE>_files -> unique files count
      <CWE>_total -> total issues count

    Also severity columns.
    """
    base_cols = ["Group", "LLM name", "prompt method"]

    # Base empty summary structure
    empty_summary = pd.DataFrame(
        columns=base_cols
                + ["blocker_count", "critical_count", "major_count", "minor_count", "info_count"]
    )

    if issues_df.empty:
        return empty_summary

    # Start from filtered issues
    df = issues_df.copy()

    # ---------- CWE metrics ----------
    cwe_df = df[
        df["security_category"].notna() & (df["security_category"] != "")
        ].copy()

    if not cwe_df.empty:
        agg = (
            cwe_df
            .groupby(
                ["folder_1", "folder_3", "folder_2", "security_category"],
                dropna=False,
            )
            .agg(
                files=("file_name", "nunique"),
                total=("key", "count"),
            )
            .reset_index()
        )

        # Pivot to get two columns per CWE
        cwe_pivot = agg.pivot_table(
            index=["folder_1", "folder_3", "folder_2"],
            columns="security_category",
            values=["files", "total"],
            fill_value=0,
            aggfunc="sum",
        )

        # Flatten multiindex columns: (files, CWE-327) -> CWE-327_files
        flat_cols = []
        for val_type, cwe in cwe_pivot.columns:
            suffix = "files" if val_type == "files" else "total"
            flat_cols.append(f"{cwe}_{suffix}")
        cwe_pivot.columns = flat_cols

        cwe_pivot = cwe_pivot.reset_index()
        cwe_pivot = cwe_pivot.rename(
            columns={
                "folder_1": "Group",
                "folder_3": "LLM name",
                "folder_2": "prompt method",
            }
        )
    else:
        cwe_pivot = pd.DataFrame(columns=base_cols)

    # ---------- Severity metrics ----------
    sev = (
        df.groupby(
            ["folder_1", "folder_3", "folder_2", "severity"],
            dropna=False,
        )["key"]
        .count()
        .reset_index(name="count")
    )

    if not sev.empty:
        sev_pivot = sev.pivot_table(
            index=["folder_1", "folder_3", "folder_2"],
            columns="severity",
            values="count",
            fill_value=0,
            aggfunc="sum",
        ).reset_index()

        sev_pivot = sev_pivot.rename(
            columns={
                "folder_1": "Group",
                "folder_3": "LLM name",
                "folder_2": "prompt method",
            }
        )
        sev_pivot.columns.name = None
    else:
        sev_pivot = pd.DataFrame(columns=base_cols)

    severity_col_map = {
        "BLOCKER": "blocker_count",
        "CRITICAL": "critical_count",
        "MAJOR": "major_count",
        "MINOR": "minor_count",
        "INFO": "info_count",
    }

    for sev_key, col_name in severity_col_map.items():
        if sev_key in sev_pivot.columns:
            sev_pivot = sev_pivot.rename(columns={sev_key: col_name})

    for col_name in severity_col_map.values():
        if col_name not in sev_pivot.columns:
            sev_pivot[col_name] = 0

    # ---------- Merge CWE and severity ----------
    if cwe_pivot.empty and sev_pivot.empty:
        summary = empty_summary.copy()
    elif cwe_pivot.empty:
        summary = sev_pivot.copy()
    elif sev_pivot.empty:
        summary = cwe_pivot.copy()
    else:
        summary = pd.merge(
            cwe_pivot,
            sev_pivot[["Group", "LLM name", "prompt method"] + list(severity_col_map.values())],
            on=["Group", "LLM name", "prompt method"],
            how="outer",
        )

    # Ensure base columns exist
    for col in base_cols:
        if col not in summary.columns:
            summary[col] = ""

    # Fill numeric NaNs with 0
    for col in summary.columns:
        if col not in base_cols:
            summary[col] = summary[col].fillna(0).astype(int)

    # Order columns nicely
    sev_cols = list(severity_col_map.values())
    cwe_cols = [c for c in summary.columns if c not in base_cols + sev_cols]
    summary = summary[base_cols + sev_cols + sorted(cwe_cols)]

    return summary


def flatten_analyses(raw_analyses: list) -> pd.DataFrame:
    records = []
    for a in raw_analyses:
        events = a.get("events") or []
        event_types = ",".join(e.get("category", "") for e in events if e.get("category"))
        event_names = ",".join(e.get("name", "") for e in events if e.get("name"))
        records.append(
            {
                "analysis_key": a.get("key"),
                "date": a.get("date"),
                "projectVersion": a.get("projectVersion"),
                "buildString": a.get("buildString"),
                "revision": a.get("revision"),
                "event_categories": event_types,
                "event_names": event_names,
            }
        )
    return pd.DataFrame(records)


def main():
    print(f"Fetching measures for project {PROJECT_KEY}...")
    metrics_df = pd.DataFrame([fetch_measures(PROJECT_KEY)])

    print("Fetching OPEN vulnerability issues...")
    raw_issues = fetch_all_issues(PROJECT_KEY)
    issues_df = flatten_issues(raw_issues) if raw_issues else pd.DataFrame()

    print(f"Filtering issues by language: {TARGET_LANGUAGE}...")
    issues_df = apply_language_mapping(issues_df)

    print("Building Summary sheet...")
    summary_df = build_summary_pivot(issues_df)

    print("Fetching analyses history...")
    raw_analyses = fetch_analyses(PROJECT_KEY)
    analyses_df = flatten_analyses(raw_analyses) if raw_analyses else pd.DataFrame()

    print(f"Writing data to {OUTPUT_FILE}...")
    with pd.ExcelWriter(OUTPUT_FILE, engine="openpyxl") as writer:
        metrics_df.to_excel(writer, sheet_name="Metrics", index=False)
        issues_df.to_excel(writer, sheet_name="Issues", index=False)
        summary_df.to_excel(writer, sheet_name="Summary", index=False)
        analyses_df.to_excel(writer, sheet_name="Analyses", index=False)

    print("Done. Created:", OUTPUT_FILE)


if __name__ == "__main__":
    main()
