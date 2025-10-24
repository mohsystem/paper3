import pandas as pd
import re

excel_path = "D:\\sourceCode\\PhD\\paper3\\analysis\\c-data.csv"
# excel_path = "D:/GoogleDrive/edu/PhD/Dissertation/RQ2/generated-code-python-detailed-sample.xlsx"


xl = pd.read_csv(excel_path)
print(xl.dtypes)
print(xl.groupby("PromptMethod")["SecurityVulns"].median(numeric_only=True))
df= xl
# import pandas as pd
# import matplotlib.pyplot as plt
# import matplotlib
# matplotlib.use("Agg")
# import matplotlib.pyplot as plt
# # Suppose you've already loaded your DataFrame
# # df = pd.read_csv("your_data.csv")
#
# # 1. Compute mean and standard deviation for SecurityVulns per PromptMethod
# group_stats = df.groupby("PromptMethod")["SecurityVulns"].agg(["mean", "std"])
# # This gives a DataFrame with two columns: "mean" and "std"
#
# # 2. Plot bar chart of means with error bars = std
# fig, ax = plt.subplots(figsize=(6, 4))
#
# # yerr expects the standard deviation
# ax.bar(group_stats.index, group_stats["mean"], yerr=group_stats["std"], capsize=4)
#
# ax.set_xlabel("Prompt Method")
# ax.set_ylabel("Security Vulnerabilities (mean ± std)")
# ax.set_title("Mean SecurityVulns by PromptMethod with Standard Deviation")
#
# plt.tight_layout()
# # plt.show()
# fig.savefig("my_plot.png")

#
#
# # 9. Print out to verify
# print("Columns:", xl.columns)
# # print("Columns:", combined_df.groupby(""))
# print("First 5 rows:\n", xl.head())
# print("Number of rows:", len(xl))

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# Normal Distribution #########################################

# 1. Load or create your DataFrame
# df = pd.read_csv("your_data.csv")

# 2. Decide which column to group by, and which numeric column to plot
group_col = "PromptMethod"       # e.g., grouping factor
value_col = "SecurityVulns"      # e.g., numeric measure

# 3. Group by your 'group_col'
grouped = df.groupby(group_col)

# 4. Create subplots, one per group
#   We'll have as many rows as there are groups.
n_groups = len(grouped)
fig, axes = plt.subplots(nrows=2, ncols=2,figsize=(9, 1 * n_groups), sharex=False)
axes = axes.ravel()

# If there's only 1 group, axes won't be a list, so handle that case
if n_groups == 1:
    axes = [axes]

# 5. Loop over each group, compute mean & std, plot the normal PDF
for ax, (g_name, subdf) in zip(axes, grouped):
    # a) Calculate mean & std
    mu = subdf[value_col].mean()
    sigma = subdf[value_col].std()
    # If sigma=0 or subdf has <2 rows, handle gracefully
    if subdf[value_col].count() < 2 or sigma == 0:
        ax.text(0.5, 0.5, "Not enough data or zero std", ha='center', va='center')
        ax.set_title(f"{g_name} (mean={mu:.2f}, std={sigma:.2f})")
        continue

    # b) Generate x-values around mu ± 4*sigma
    x = np.linspace(mu - 4*sigma, mu + 4*sigma, 200)
    # c) Normal PDF formula
    y = (1 / (sigma * np.sqrt(2 * np.pi))) * np.exp(-0.5 * ((x - mu) / sigma)**2)

    # d) Plot the curve
    ax.plot(x, y, label=f"{g_name}")
    ax.set_title(f"Group: {g_name} | mean={mu:.2f}, std={sigma:.2f}")
    ax.set_xlabel(value_col)
    ax.set_ylabel("Probability Density")

plt.tight_layout()

plt.savefig("Normal Distribution")




import pandas as pd
import pingouin as pg

# Suppose you've already loaded your DataFrame
# df = pd.read_csv("your_data.csv")

# We assume 'PromptMethod' is a categorical column with methods,
# and 'SecurityVulns' is the numeric outcome measure.
# llama31  gpt4o   gemini  codestral   claude
res = pg.anova(
    dv="SecurityVulns",          # the dependent variable (numeric)
    between="PromptMethod",      # the factor/grouping variable
    data=df[df["LLM"]=="claude"],
    detailed=True                # provides a more detailed ANOVA table
)

print(res)

filterBy = "gpt4o"
import pandas as pd
import matplotlib.pyplot as plt
df = df[df["LLM"]==filterBy]
# 1. Suppose your DataFrame has columns like "Month" and "Vulnerability"
#    We'll filter out rows where "Vulnerability" == 0, retaining only >0
df_nonzero = df[df["SecurityVulns"] != 0]

# 2. Group by "Month" and count how many rows we have for each month
counts = df_nonzero.groupby("PromptMethod")["SecurityVulns"].count()

# 3. Create a bar plot of these counts
fig, ax = plt.subplots(figsize=(8, 4))
counts.plot(kind="bar", ax=ax)

ax.set_title("Frequency of Nonzero Vulnerabilities by Prompt Method")
ax.set_xlabel("SecurityVulns")
ax.set_ylabel("Count of Rows (Nonzero Vulnerabilities)")

plt.tight_layout()
plt.savefig("Frequency of Nonzero Vulnerabilities by Prompt Method")
