import pandas as pd
from pathlib import Path

# -----------------------------
# Inputs
# -----------------------------
primary_path = Path("summary-our-ds.xlsx")
bench_path   = Path("summary-LLMSecEval-ds.xlsx")

out_path     = Path("combined_severity_summary.xlsx")

# -----------------------------
# Load workbooks
# -----------------------------
primary_xls = pd.ExcelFile(primary_path)
bench_xls   = pd.ExcelFile(bench_path)

primary_sheets = primary_xls.sheet_names
bench_sheets   = bench_xls.sheet_names

def load_dataset(xls: pd.ExcelFile, dataset_name: str):
    """Load all sheets and append Dataset + Language columns."""
    frames = []
    for sh in xls.sheet_names:
        df = pd.read_excel(xls, sheet_name=sh)
        df.columns = [c.strip() if isinstance(c, str) else c for c in df.columns]
        df["Dataset"] = dataset_name
        df["Language"] = sh
        frames.append(df)
    return frames

# Combine all rows from both datasets
combined_df = pd.concat(
    load_dataset(primary_xls, "Primary") + load_dataset(bench_xls, "LLMSecEval"),
    ignore_index=True
)

# -----------------------------
# Build summary
# -----------------------------
llm_col = "LLM name"
prompt_col = "prompt method"

if llm_col not in combined_df.columns or prompt_col not in combined_df.columns:
    raise ValueError("Expected columns 'LLM name' and 'prompt method' were not found.")

severity_cols = [c for c in ["blocker_count", "critical_count", "major_count", "minor_count"]
                 if c in combined_df.columns]
if not severity_cols:
    raise ValueError("No severity count columns were found.")

summary = (
    combined_df
    .groupby(["Dataset", "Language", llm_col, prompt_col], dropna=False)[severity_cols]
    .sum()
    .reset_index()
    .rename(columns={llm_col: "LLM", prompt_col: "Prompt"})
)

# Convenience: high-severity = Blocker + Critical
if "blocker_count" in summary.columns and "critical_count" in summary.columns:
    summary["high_severity_blocker_plus_critical"] = (
            summary["blocker_count"] + summary["critical_count"]
    )

# Excel sheet name must be <= 31 chars
def safe_sheet_name(name: str) -> str:
    name = name.replace("/", "_").replace("\\", "_")
    return name[:31]

# -----------------------------
# Write output workbook
# -----------------------------
with pd.ExcelWriter(out_path, engine="openpyxl") as writer:
    combined_df.to_excel(writer, sheet_name="Combined_Raw", index=False)
    summary.to_excel(writer, sheet_name="Summary_By_Config", index=False)

    # Also keep original sheets for traceability
    for sh in primary_sheets:
        pd.read_excel(primary_xls, sheet_name=sh).to_excel(
            writer, sheet_name=safe_sheet_name(f"P_{sh}"), index=False
        )
    for sh in bench_sheets:
        pd.read_excel(bench_xls, sheet_name=sh).to_excel(
            writer, sheet_name=safe_sheet_name(f"B_{sh}"), index=False
        )

print(f"Saved: {out_path}")
