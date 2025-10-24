import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import matplotlib as mpl
# ----------------- Data (from SonarQube export) -----------------
data = {
    "Prompt": ["Zero-shot CoT"]*5 + ["WA-0CoT"]*5 + ["Vanilla"]*5 + ["Zero-shot"]*5,
    "LLM": ["claude-3.5","codestral","gemini-1.5","GPT-4o","llama-3.1"]*4,
    "Python_Total":[7,7,4,7,7, 5,12,4,6,13, 4,4,4,4,5, 9,4,6,3,6],
    "Python_Blocker":[0,3,1,0,2, 0,3,0,0,4, 0,1,1,0,1, 0,1,1,0,0],
    "Python_High":[7,4,3,7,5, 5,9,4,6,9, 4,3,3,4,4, 9,3,5,3,6],
    "Java_Total":[22,20,13,13,11, 22,18,15,13,15, 16,9,11,12,11, 30,17,19,14,12],
    "Java_Blocker":[1,2,2,2,2, 1,3,3,0,3, 2,2,2,2,2, 1,1,2,1,2],
    "Java_High":[21,18,11,11,9, 21,15,12,13,12, 14,7,9,10,9, 29,16,17,13,10],
    "Cpp_Total":[15,11,4,13,8, 13,10,6,17,6, 12,9,7,12,8, 18,6,6,11,7],
    "Cpp_Blocker":[0,0,0,0,0, 0,0,0,1,0, 0,0,0,0,0, 0,0,0,0,0],
    "Cpp_High":[15,11,4,13,8, 13,10,6,16,6, 12,9,7,12,8, 18,6,6,11,7],
    "C_Total":[18,15,13,13,7, 22,13,15,23,10, 19,17,13,14,11, 22,6,16,12,16],
    "C_Blocker":[3,1,0,1,0, 3,2,1,3,1, 1,4,2,2,2, 4,2,0,1,2],
    "C_High":[15,14,13,12,7, 19,11,14,20,9, 18,13,11,12,9, 18,4,16,12,14],
}
df = pd.DataFrame(data)

prompt_order = ["Vanilla","Zero-shot","Zero-shot CoT","WA-0CoT"]
llm_order = ["llama-3.1","GPT-4o","gemini-1.5","codestral","claude-3.5"]

df["LLM"] = pd.Categorical(df["LLM"], categories=llm_order, ordered=True)
df["Prompt"] = pd.Categorical(df["Prompt"], categories=prompt_order, ordered=True)
df = df.sort_values(["LLM","Prompt"])

COLORS = {"High":"#f9cb9c", "Blocker":"#d9ead3"}
WIDTH = 0.18
mpl.rcParams['pdf.fonttype'] = 42
mpl.rcParams['ps.fonttype'] = 42
def stacked_llm_language(lang_label, file_out):
    prefix = "Cpp" if lang_label == "C++" else lang_label
    sub = df[["LLM","Prompt",f"{prefix}_High",f"{prefix}_Blocker"]].copy()
    sub = sub.sort_values(["LLM","Prompt"])
    x = np.arange(len(llm_order))

    fig, ax = plt.subplots(figsize=(9,5))
    for i, p in enumerate(prompt_order):
        pick = sub[sub["Prompt"] == p]
        highs = pick[f"{prefix}_High"].to_numpy()
        blocks = pick[f"{prefix}_Blocker"].to_numpy()

        xpos = x + (i - 1.5)*WIDTH
        ax.bar(xpos, highs, WIDTH, color=COLORS["High"], edgecolor="#555", linewidth=0.5)
        ax.bar(xpos, blocks, WIDTH, bottom=highs, color=COLORS["Blocker"], edgecolor="#555", linewidth=0.5)

        # annotate totals
        for j in range(len(x)):
            total = int(highs[j] + blocks[j])
            ax.text(xpos[j], highs[j] + blocks[j] + 0.2, f"{total}", ha="center", va="bottom", fontsize=7, rotation=90)

    ax.set_xticks(x)
    ax.set_xticklabels(llm_order, rotation=25, ha="right")
    ax.set_ylabel("Vulnerabilities (High + Blocker)")
    # ax.set_title(f"{lang_label} — totals per LLM by prompt")
    # dual legend: prompt methods + severity color key
    from matplotlib.patches import Patch
    prompt_patches = [Patch(facecolor="#ccc", edgecolor="#555", label=p) for p in prompt_order]
    sev_patches = [Patch(facecolor=COLORS["High"], edgecolor="#555", label="High"),
                   Patch(facecolor=COLORS["Blocker"], edgecolor="#555", label="Blocker")]
    leg1 = ax.legend(handles=prompt_patches, title="Prompt method", loc="upper left", frameon=False, ncol=2)
    ax.add_artist(leg1)
    ax.legend(handles=sev_patches, title="Severity", loc="upper center", frameon=False)
    ax.grid(axis="y", alpha=0.3, linestyle="--")
    fig.tight_layout()
    fig.savefig(file_out, format="pdf", bbox_inches="tight")
    plt.close(fig)

for lang, outpng in [("Python","stacked_llm_python.pdf"),
                     ("Java","stacked_llm_java.pdf"),
                     ("C++","stacked_llm_cpp.pdf"),
                     ("C","stacked_llm_c.pdf")]:
    stacked_llm_language(lang, outpng)
