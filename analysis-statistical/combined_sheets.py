import pandas as pd
import re

excel_path = "D:/GoogleDrive/edu/PhD/Dissertation/RQ2/results/generated-code-java-detailed.xlsx"
# excel_path = "D:/GoogleDrive/edu/PhD/Dissertation/RQ2/generated-code-python-detailed-sample.xlsx"


# 1. Load the workbook
xl = pd.ExcelFile(excel_path)

# 2. Create a list to store DataFrames from each sheet
all_dfs = []

for sheet_name in xl.sheet_names:
    # 3. Read each sheet into a DataFrame
    # header=0 assumes the first row in each sheet is the column header row
    df = pd.read_excel(excel_path, sheet_name=sheet_name, header=0)

    # 4. Extract prompt method and LLM from the sheet name
    # Example naming scheme: "CoT_template_LLM_claude"
    # You might have something like "CoT_template_LLM_gpt4o" or "Vanilla_template_LLM_codestral"
    # Let's do a simple approach: split on underscores
    # if your actual naming scheme differs, adjust the splitting logic

    # Attempt to identify words like "CoT", "template", "LLM", "claude" from the sheet name
    # e.g. sheet_name.split("_") => ["CoT", "template", "LLM", "claude"]

    parts = sheet_name.split("_")

    # We'll guess the last part is the LLM, and the first part is the prompt method
    # You might have multiple underscores, so adapt as needed
    if len(parts) >= 4:
        # e.g. ["CoT", "template", "LLM", "claude"]
        prompt_method = parts[0]       # "CoT"
        llm = parts[-1]               # "claude"
    else:
        # If the sheet name doesn't match your pattern, define a fallback
        prompt_method = parts[0] if len(parts) > 0 else "UnknownMethod"
        llm = parts[-1] if len(parts) > 1 else "UnknownLLM"

    # 5. Add columns for LLM and PromptMethod
    df["LLM"] = llm
    df["PromptMethod"] = prompt_method

    # 6. Append this DataFrame to our list
    all_dfs.append(df)

# 7. Concatenate all sheets into a single DataFrame
combined_df = pd.concat(all_dfs, ignore_index=True)
pd.set_option("display.max_columns",None)
# 8. Optionally rename columns for consistency
combined_df.rename(columns={
    "Generated code file name": "CodeFile",
    "Lines of Code": "LoC",
    "Security": "SecurityVulns",
    "Reliability": "ReliabilityIssues",
    "Maintainability": "MaintainabilityScore",
    "Security Hotspots": "SecHotspots"
}, inplace=True)

# 9. Print out to verify
print("Columns:", combined_df.columns)
# print("Columns:", combined_df.groupby(""))
print("First 5 rows:\n", combined_df.head())
print("Number of rows:", len(combined_df))
combined_df.to_csv("D:/GoogleDrive/edu/PhD/Dissertation/RQ2/results/java-all.csv")


