import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Patch
from matplotlib.ticker import FixedLocator

# -----------------------------
# Inputs
# -----------------------------
PRIMARY_XLSX   = "summary-our-ds.xlsx"
BENCHMARK_XLSX = "summary-LLMSecEval-ds.xlsx"

OUT_PDF = "Fig_Severity_All_Datasets.pdf"
OUT_PNG = "Fig_Severity_All_Datasets.png"

# Sheet names in your Excel files
languages = ["C", "Java", "Python"]

# Order
llm_order = ["gpt-5", "claude-4.5", "gemini-2.5"]
prompt_order = ["Vanilla", "ZeroShot", "CoT", "ourMethod"]

# Abbreviations
prompt_abbr = {"Vanilla": "V", "ZeroShot": "ZS", "CoT": "CoT", "ourMethod": "MA"}

# Severity columns (Major excluded)
severities = [
    ("Blocker",  "blocker_count",  "#8B0000", ""),
    ("Critical", "critical_count", "#D97A00", ""),
    ("Major", "major_count", "#d62728", ""),
    ("Minor",    "minor_count",    "#6E6E6E", ""),
]

# -----------------------------
# Visual tuning
# -----------------------------
# Piecewise y scaling: expand 0–10 by 2x
LOW_RANGE_MAX = 10
LOW_RANGE_GAIN = 2.0

# Reduce spacing around bars/groups (per your request)
BAR_W = 0.0005        # wider bars
METHOD_GAP = 0.0002  # smaller gap between methods
GROUP_GAP = 0.0005    # smaller gap between LLM groups

# -----------------------------
# Piecewise scale functions
# -----------------------------
def y_forward(y):
    """
    Map original y -> display y.
    For y <= 10: expand by 2x.
    For y > 10: keep same slope, just shift up to keep continuity.
    """
    y = np.asarray(y, dtype=float)
    return np.where(
        y <= LOW_RANGE_MAX,
        y * LOW_RANGE_GAIN,
        (LOW_RANGE_MAX * LOW_RANGE_GAIN) + (y - LOW_RANGE_MAX)
    )

def y_inverse(yt):
    """Inverse mapping for ticks/limits."""
    yt = np.asarray(yt, dtype=float)
    return np.where(
        yt <= (LOW_RANGE_MAX * LOW_RANGE_GAIN),
        yt / LOW_RANGE_GAIN,
        LOW_RANGE_MAX + (yt - (LOW_RANGE_MAX * LOW_RANGE_GAIN))
    )

# -----------------------------
# Helpers
# -----------------------------
def load_dataset(path: str) -> dict[str, pd.DataFrame]:
    xls = pd.ExcelFile(path)
    return {sh: pd.read_excel(path, sheet_name=sh) for sh in xls.sheet_names}

def aggregate_counts(df: pd.DataFrame) -> pd.DataFrame:
    df = df.copy()
    df["LLM"] = df["LLM name"]
    df["Prompt"] = df["prompt method"]
    df = df[df["Prompt"].isin(prompt_order)]

    idx = pd.MultiIndex.from_product([llm_order, prompt_order], names=["LLM", "Prompt"])
    cols = [col for _, col, _, _ in severities]

    out = (
        df.set_index(["LLM", "Prompt"])[cols]
        .reindex(idx)
        .fillna(0)
        .astype(int)
    )
    return out

def panel_max_height(dataset_dict: dict[str, pd.DataFrame]) -> int:
    m = 0
    cols = [col for _, col, _, _ in severities]
    for lang in languages:
        agg = aggregate_counts(dataset_dict[lang])
        heights = agg[cols].sum(axis=1)
        m = max(m, int(heights.max()))
    return m

def _bar_geometry():
    x_positions, method_ticks, group_centers = [], [], []
    x = 0.0
    for llm in llm_order:
        start = x
        for p in prompt_order:
            x_positions.append(x)
            method_ticks.append(prompt_abbr[p])
            x += BAR_W + METHOD_GAP
        end = x - (BAR_W + METHOD_GAP)
        group_centers.append((start + end) / 2)
        x += GROUP_GAP
    return x_positions, method_ticks, group_centers

def _vals_in_plot_order(agg: pd.DataFrame) -> np.ndarray:
    vals = []
    for llm in llm_order:
        for p in prompt_order:
            vals.append(agg.loc[(llm, p)].values)
    return np.array(vals)

def _set_piecewise_yscale(ax):
    ax.set_yscale("function", functions=(y_forward, y_inverse))

def _set_ticks(ax, y_max, hide_y_numbers=False):
    """
    Make 0–10 readable by dense ticks there, and coarser above.
    Ticks are set in ORIGINAL y units.
    """
    ticks = [0, 2, 4, 6, 8, 10]

    # Add coarser ticks above 10 (keep readable; adjust if you want denser)
    step_above = 10
    t = 20
    while t <= y_max:
        ticks.append(t)
        t += step_above

    ax.yaxis.set_major_locator(FixedLocator(ticks))
    ax.tick_params(axis="y", which="major", labelsize=9, length=4)

    if hide_y_numbers:
        ax.tick_params(axis="y", labelleft=False)

def plot_language_panel(
        ax,
        agg: pd.DataFrame,
        language: str,
        dataset_name: str,
        show_ylabel: bool,
        y_max: int,
        show_llm_labels: bool,
        show_language_title: bool,
        hide_y_numbers: bool,
):
    x_positions, method_ticks, group_centers = _bar_geometry()
    vals = _vals_in_plot_order(agg)

    # Piecewise y scaling (single axis, no inset)
    _set_piecewise_yscale(ax)

    bottoms = np.zeros(len(x_positions))

    # Bars: no outlines
    containers = []
    for j, (sev_name, _, color, hatch) in enumerate(severities):
        bars = ax.bar(
            x_positions,
            vals[:, j],
            width=BAR_W,
            bottom=bottoms,
            color=color,
            edgecolor="none",
            linewidth=0,
            hatch=hatch,
            label=sev_name,
            zorder=2
        )
        containers.append(bars)
        bottoms += vals[:, j]

    # Bold white labels
    for j, bars in enumerate(containers):
        labels = [str(v) if v > 0 else "" for v in vals[:, j]]
        ax.bar_label(
            bars,
            labels=labels,
            label_type="center",
            fontsize=9,
            # fontweight="bold",
            color="white",
        )

    # X ticks
    ax.set_xticks(x_positions)
    ax.set_xticklabels(method_ticks, fontsize=9)
    ax.tick_params(axis="x", length=0)

    # Unified y scale (original units)
    ax.set_ylim(0, y_max)

    # Make 0–10 more visible via piecewise + explicit ticks
    _set_ticks(ax, y_max=y_max, hide_y_numbers=hide_y_numbers)

    # LLM labels (only first row)
    if show_llm_labels:
        for llm, xc in zip(llm_order, group_centers):
            ax.text(
                xc, y_max * 0.99,
                llm,
                ha="center", va="top",
                fontsize=9,
                fontweight="bold"
            )

    # Group separators
    n_methods = len(prompt_order)
    for g in range(len(llm_order) - 1):
        last_idx = (g + 1) * n_methods - 1
        next_idx = last_idx + 1
        boundary = (x_positions[last_idx] + x_positions[next_idx]) / 2
        ax.axvline(boundary, color="black", linewidth=0.6, alpha=0.20)

    # Titles and y-labels
    ax.set_title(language if show_language_title else "", fontweight="bold",fontsize=11)
    if show_ylabel:
        ax.set_ylabel(f"{dataset_name} Vulnerability count",fontweight="bold", fontsize=9)

    ax.grid(axis="y", linestyle=":", linewidth=0.6, alpha=0.7)
    ax.margins(x=0.01)  # reduce left/right padding inside each subplot

# -----------------------------
# Main
# -----------------------------
primary = load_dataset(PRIMARY_XLSX)
bench   = load_dataset(BENCHMARK_XLSX)

# Keep unified scale as you had before (same approach)
global_ymax = max(panel_max_height(primary), panel_max_height(bench))
global_ymax = int(np.ceil(global_ymax * 1.15))

fig, axes = plt.subplots(
    nrows=2, ncols=3,
    figsize=(13.5, 6.4),
    dpi=400,
    constrained_layout=False
)

datasets = [("Primary", primary), ("LLMSecEval", bench)]

for r, (dname, ds) in enumerate(datasets):
    for c, lang in enumerate(languages):
        ax = axes[r, c]
        agg = aggregate_counts(ds[lang])

        plot_language_panel(
            ax=ax,
            agg=agg,
            language=lang,
            dataset_name=dname,
            show_ylabel=(c == 0),
            y_max=global_ymax,
            show_llm_labels=(r == 0),      # remove LLM names from 2nd row
            show_language_title=(r == 0),  # remove language titles from 2nd row
            hide_y_numbers=(c != 0),       # remove y numbering from cols 2 and 3
        )

# Smaller legend at top-right
severity_handles = [
    Patch(facecolor=color, edgecolor="none", hatch=hatch, label=sev_name)
    for (sev_name, _, color, hatch) in severities
]

fig.legend(
    handles=severity_handles,
    loc="upper right",
    bbox_to_anchor=(0.045, 0.955),
    frameon=True,
    fontsize=9,
    title="Severity",
    title_fontsize=9,
    borderpad=0.2,
    labelspacing=0.2,
    handlelength=0.95,
    handletextpad=0.09,
)

# Layout: reserve space for legend
plt.tight_layout(rect=[0.02, 0.02, 0.92, 0.94])

fig.savefig(OUT_PDF, bbox_inches="tight")
fig.savefig(OUT_PNG, bbox_inches="tight")
plt.close(fig)

print(f"Saved: {OUT_PDF}")
print(f"Saved: {OUT_PNG}")
