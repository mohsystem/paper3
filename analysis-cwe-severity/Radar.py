import matplotlib.pyplot as plt
import matplotlib as mpl

import numpy as np
import pandas as pd

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
prompts = ["Vanilla","Zero-shot","Zero-shot CoT","WA-0CoT"]
metrics = ["Total","Blocker","High"]
PASTELS = ["#cfe2f3", "#ffe599", "#f9cb9c", "#d9ead3"]  # light blue, light yellow, light orange, light green

def radar_for_language(lang_label, file_out):
    prefix = "Cpp" if lang_label == "C++" else lang_label
    values = []
    for p in prompts:
        row = df[df["Prompt"] == p][[f"{prefix}_{m}" for m in metrics]].mean().values
        values.append(row.tolist())

    N = len(metrics)
    angles = np.linspace(0, 2*np.pi, N, endpoint=False).tolist()
    angles += angles[:1]

    fig, ax = plt.subplots(figsize=(6,6), subplot_kw=dict(polar=True))
    # for i, p in enumerate(prompts):
    #     v = values[i] + values[i][:1]
    #     ax.plot(angles, v, color=PASTELS[i], linewidth=2, label=p)
    #     ax.fill(angles, v, color=PASTELS[i], alpha=0.35)
    #
    # ax.set_xticks(angles[:-1])
    # ax.set_xticklabels(metrics)
    # ax.set_title(f"{lang_label}", size=14)
    # ax.legend(loc="upper right", bbox_to_anchor=(1.22, 1.02), frameon=False)
    # fig.tight_layout()
    # fig.savefig(file_out, dpi=400)
    # plt.close(fig)

    mpl.rcParams['pdf.fonttype'] = 42
    mpl.rcParams['ps.fonttype'] = 42

    for i, p in enumerate(prompts):
        v = values[i] + values[i][:1]
        ax.plot(angles, v, color=PASTELS[i], linewidth=2, label=p)
        ax.fill(angles, v, color=PASTELS[i], alpha=0.35)

    ax.set_xticks(angles[:-1])
    ax.set_xticklabels(metrics)
    # ax.set_title(f"", size=14)
    ax.legend(loc="upper right", bbox_to_anchor=(1.22, 1.02), frameon=False)
    fig.tight_layout()
    fig.savefig(file_out, format="pdf", bbox_inches="tight")
    plt.close(fig)
for lang, outpng in [("Python","radar_python.pdf"),
                     ("Java","radar_java.pdf"),
                     ("C++","radar_cpp.pdf"),
                     ("C","radar_c.pdf")]:
    radar_for_language(lang, outpng)
