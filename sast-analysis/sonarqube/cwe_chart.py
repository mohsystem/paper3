import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
from matplotlib.patches import Patch

PRIMARY_XLSX = Path("summary-our-ds.xlsx")
BENCH_XLSX   = Path("summary-LLMSecEval-ds.xlsx")

OUT_PDF = Path("Fig_CWE_Stacked_Top2_PerBar.pdf")
OUT_PNG = Path("Fig_CWE_Stacked_Top2_PerBar.png")

# IMPORTANT: Excel sheet names are often "Java" not "Java"
languages = ["C", "Java", "Python"]
lang_titles = {"C": "C", "Java": "Java", "Python": "Python"}

llm_order = ["gpt-5", "claude-4.5", "gemini-2.5"]
prompt_order = ["Vanilla", "ZeroShot", "CoT", "ourMethod"]
prompt_labels = {"Vanilla": "V", "ZeroShot": "ZS", "CoT": "CoT", "ourMethod": "MA"}

# -----------------------------
# Visual tuning
# -----------------------------
BAR_W = 0.20
METHOD_GAP = 0.012
GROUP_GAP = 0.07

# Piecewise y scaling (expand low range without inset)
LOW_RANGE_MAX = 2
LOW_RANGE_GAIN = 5.0   # <- use 2.0 (7.0 is usually too extreme / distorts perception)

def forward(y):
    y = np.asarray(y)
    out = y.astype(float).copy()
    mask = y <= LOW_RANGE_MAX
    out[mask] = y[mask] * LOW_RANGE_GAIN
    out[~mask] = LOW_RANGE_MAX * LOW_RANGE_GAIN + (y[~mask] - LOW_RANGE_MAX)
    return out

def inverse(y):
    y = np.asarray(y)
    out = y.astype(float).copy()
    cutoff = LOW_RANGE_MAX * LOW_RANGE_GAIN
    mask = y <= cutoff
    out[mask] = y[mask] / LOW_RANGE_GAIN
    out[~mask] = LOW_RANGE_MAX + (y[~mask] - cutoff)
    return out

def load_book(path: Path):
    if not path.exists():
        raise FileNotFoundError(f"Missing file: {path}")
    xls = pd.ExcelFile(path)
    return {sh: pd.read_excel(path, sheet_name=sh) for sh in xls.sheet_names}

def find_cwe_total_cols(df: pd.DataFrame):
    return [c for c in df.columns if isinstance(c, str) and c.startswith("CWE-") and c.endswith("_total")]

def bar_positions():
    x_positions, xtick_labels, group_centers = [], [], []
    x = 0.0
    for llm in llm_order:
        start = x
        for p in prompt_order:
            x_positions.append(x)
            xtick_labels.append(prompt_labels[p])
            x += BAR_W + METHOD_GAP
        end = x - (BAR_W + METHOD_GAP)
        group_centers.append((start + end) / 2)
        x += GROUP_GAP
    return x_positions, xtick_labels, group_centers

def top2_for_bar(series: pd.Series):
    s = series.copy()
    s = s[s > 0]
    if s.empty:
        return []
    df = s.reset_index()
    df.columns = ["cwe", "count"]
    df["cwe_sort"] = df["cwe"].astype(str)
    df = df.sort_values(by=["count", "cwe_sort"], ascending=[False, True])
    pairs = list(zip(df["cwe"].tolist(), df["count"].astype(int).tolist()))
    return pairs[:2]

def build_counts_per_panel(df: pd.DataFrame):
    df = df.copy()
    df.columns = [c.strip() if isinstance(c, str) else c for c in df.columns]
    cwe_cols = find_cwe_total_cols(df)
    if not cwe_cols:
        raise ValueError("No CWE columns found (expected columns like 'CWE-295_total').")

    df["LLM"] = df["LLM name"]
    df["Prompt"] = df["prompt method"]

    out = {}
    for llm in llm_order:
        for p in prompt_order:
            sub = df[(df["LLM"] == llm) & (df["Prompt"] == p)]
            if sub.empty:
                s = pd.Series(0, index=cwe_cols, dtype=int)
            else:
                s = sub[cwe_cols].sum(numeric_only=True).astype(int)

            s.index = [i.replace("_total", "") for i in s.index]

            top2 = top2_for_bar(s)
            if len(top2) >= 1:
                cwe1, v1 = top2[0]
            else:
                cwe1, v1 = ("", 0)

            if len(top2) >= 2:
                cwe2, v2 = top2[1]
            else:
                cwe2, v2 = ("", 0)

            out[(llm, p)] = {
                "cwe1": cwe1, "v1": int(v1),
                "cwe2": cwe2, "v2": int(v2),
                "top2_total": int(v1 + v2)
            }
    return out

# -----------------------------
# Load datasets
# -----------------------------
primary = load_book(PRIMARY_XLSX)
bench   = load_book(BENCH_XLSX)
datasets = [("Primary", primary), ("LLMSecEval", bench)]

# Build panel stats and GLOBAL y_max based on plotted stack height (Top-2)
panel_stats = {}
global_max_top2 = 0
for dname, book in datasets:
    for lang in languages:
        stats = build_counts_per_panel(book[lang])
        panel_stats[(dname, lang)] = stats
        for k in stats:
            global_max_top2 = max(global_max_top2, stats[k]["top2_total"])

# y-axis max should follow the plotted values (Top-2)
y_max = int(np.ceil(global_max_top2 * 1.15)) if global_max_top2 > 0 else 1

# y ticks: dense in low range, then coarse
base_ticks = list(range(0, LOW_RANGE_MAX + 1, 2))
high_start = max(LOW_RANGE_MAX + 2, 14)
high_ticks = list(range(high_start, int(np.ceil(y_max / 10.0)) * 10 + 1, 10))
yticks = sorted(set([t for t in base_ticks + high_ticks if t <= y_max]))

x_positions, xtick_labels, group_centers = bar_positions()
n_bars = len(x_positions)

# Colors
cmap = plt.get_cmap("tab10")
color_top1 = cmap(0)
color_top2 = cmap(1)

fig, axes = plt.subplots(2, 3, figsize=(13.5, 6.6), dpi=350)

for r, (dname, _) in enumerate(datasets):
    for c, lang in enumerate(languages):
        ax = axes[r, c]
        stats = panel_stats[(dname, lang)]

        v1 = np.zeros(n_bars, dtype=int)
        v2 = np.zeros(n_bars, dtype=int)
        id1 = [""] * n_bars
        id2 = [""] * n_bars

        idx = 0
        for llm in llm_order:
            for p in prompt_order:
                d = stats[(llm, p)]
                v1[idx] = d["v1"]
                v2[idx] = d["v2"]
                id1[idx] = d["cwe1"].replace("CWE-", "")  # remove CWE- prefix
                id2[idx] = d["cwe2"].replace("CWE-", "")
                idx += 1

        b1 = ax.bar(x_positions, v1, width=BAR_W, color=color_top1, edgecolor="none", linewidth=0)
        b2 = ax.bar(x_positions, v2, width=BAR_W, bottom=v1, color=color_top2, edgecolor="none", linewidth=0)

        # Label policy: always show Top-1/Top-2; if too small, put above
        MIN_INSIDE = 2
        for i, (bar, val) in enumerate(zip(b1, v1)):
            if val > 0 and id1[i]:
                x = bar.get_x() + bar.get_width()/2
                if val >= MIN_INSIDE:
                    ax.text(x, val/2-0.2, f"{id1[i]}\n{val}", ha="center", va="center",
                            fontsize=7, fontweight="bold", color="black")
                else:
                    if val > 3:
                        ax.text(x, val - 2, f"{id1[i]}\n{val}", ha="center", va="bottom",
                                fontsize=7, fontweight="bold", color="black")
                    else:
                        ax.text(x, val - 1, f"{id1[i]}\n{val}", ha="center", va="bottom",
                                fontsize=7, fontweight="bold", color="black")
        MIN_INSIDE = 5
        for i, (bar, val) in enumerate(zip(b2, v2)):
            if val > 0 and id2[i]:
                x = bar.get_x() + bar.get_width()/2
                base = v1[i]
                if val >= MIN_INSIDE:
                    ax.text(x, base + val/2 + 0.5, f"{id2[i]}\n{val}", ha="center", va="center",
                            fontsize=7, fontweight="bold", color="black")
                else:
                    ax.text(x, base + val + 0.7, f"{id2[i]}\n{val}", ha="center", va="bottom",
                            fontsize=7, fontweight="bold", color="black")

        # Apply piecewise y-scale and use the correct y_max (based on plotted values)
        ax.set_yscale("function", functions=(forward, inverse))
        ax.set_ylim(0, y_max)
        ax.set_yticks(yticks)
        ax.tick_params(axis="y", labelsize=9)

        ax.set_xticks(x_positions)
        ax.set_xticklabels(xtick_labels, fontsize=8)
        ax.tick_params(axis="x", length=0)
        ax.grid(axis="y", linestyle=":", linewidth=0.6, alpha=0.7)

        if r == 0:
            ax.set_title(lang_titles[lang], fontsize=11)
        if c == 0:
            ax.set_ylabel(f"{dname}\nCWE count (Top-2)", fontsize=10)
        if c != 0:
            ax.tick_params(axis="y", labelleft=False)

        if r == 0:
            for llm, xc in zip(llm_order, group_centers):
                ax.text(xc, y_max * 0.99, llm, ha="center", va="top",
                        fontsize=9, fontweight="bold")

        ax.margins(x=0.01)

# Bottom legend (one line)
handles = [
    Patch(facecolor=color_top1, edgecolor="none", label="Top-1 CWE (label shows CWE-ID and count)"),
    Patch(facecolor=color_top2, edgecolor="none", label="Top-2 CWE (label shows CWE-ID and count)"),
]

fig.legend(
    handles=handles,
    loc="lower center",
    ncol=len(handles),
    frameon=True,
    fontsize=8.6,
    title="Stack interpretation",
    title_fontsize=9.5,
    bbox_to_anchor=(0.5, 0.01),
    borderpad=0.25,
    labelspacing=0.25,
    handlelength=1.1,
    handletextpad=0.5,
    columnspacing=1.0
)

plt.tight_layout(rect=[0.02, 0.07, 0.98, 0.95])
fig.savefig(OUT_PDF, bbox_inches="tight")
fig.savefig(OUT_PNG, bbox_inches="tight")
plt.close(fig)

print(f"Saved: {OUT_PDF}")
print(f"Saved: {OUT_PNG}")
print(f"y_max (Top-2 stacks): {y_max}")
