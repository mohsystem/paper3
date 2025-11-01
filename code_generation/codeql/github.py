#!/usr/bin/env python3
import argparse, csv, time, requests

def fetch_alerts(owner, repo, token):
    headers = {
        "Authorization": f"token {token}",
        "Accept": "application/vnd.github+json"
    }
    url = f"https://api.github.com/repos/{owner}/{repo}/code-scanning/alerts"
    page, per_page = 1, 100
    all_items = []
    while True:
        r = requests.get(url, headers=headers, params={"page": page, "per_page": per_page}, timeout=30)
        if r.status_code == 404:
            raise SystemExit(f"Repo not found or no access: {owner}/{repo}")
        r.raise_for_status()
        items = r.json()
        if not items:
            break
        all_items.extend(items)
        page += 1
        time.sleep(0.2)
    return all_items

def extract_cwes(tags):
    cwes = []
    for t in tags or []:
        # Tags like "external/cwe/cwe-79" -> "CWE-79"
        if isinstance(t, str) and t.lower().startswith("external/cwe/cwe-"):
            cwes.append("CWE-" + t.split("cwe-")[-1].upper())
    # Deduplicate while preserving order
    seen, out = set(), []
    for c in cwes:
        if c not in seen:
            seen.add(c); out.append(c)
    return out

def normalize_severity(rule):
    # Prefer security_severity_level (critical, high, medium, low), else rule.severity, else blank
    return (rule or {}).get("security_severity_level") or (rule or {}).get("severity") or ""

def write_csv(alerts, outpath):
    fields = [
        "number","severity","weaknesses","tags",
        "state","rule_id","tool","created_at","url"
    ]
    with open(outpath, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        for a in alerts:
            rule = a.get("rule") or {}
            tags = rule.get("tags") or []
            row = {
                "number": a.get("number"),
                "severity": normalize_severity(rule),
                "weaknesses": "|".join(extract_cwes(tags)),
                "tags": "|".join(tags),
                "state": a.get("state"),
                "rule_id": rule.get("id"),
                "tool": (a.get("tool") or {}).get("name"),
                "created_at": a.get("created_at"),
                "url": a.get("html_url"),
            }
            w.writerow(row)

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--owner", required=False, default="mohsystem")
    p.add_argument("--repo", required=False,default="llm-generated-code-python-paper3")
    p.add_argument("--token", required=False,default="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")
    p.add_argument("--out", default="code_scanning_alerts.csv")
    args = p.parse_args()

    alerts = fetch_alerts(args.owner, args.repo, args.token)
    print(f"Fetched {len(alerts)} alerts")
    write_csv(alerts, args.out)
    print(f"Wrote {args.out}")
