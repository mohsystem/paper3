import json

# 1. Read the dataset from a JSON file.
file_path = "../cwes_ds/dataset/enriched_cwe_dataset_v3.json"
with open(file_path, "r", encoding="utf-8") as f:
    cwe_data = json.load(f)

# 2. Build dictionaries for fast lookups.
category_map = {}
tags_map = {}

for item in cwe_data:
    cat = item["category"]
    if cat not in category_map:
        category_map[cat] = []
    category_map[cat].append(item)

    for tag in item["Additional_tags"]:
        # If tag is a dictionary, convert it into a string or extract a field.
        if isinstance(tag, dict):
            # Option 1: Convert to string
            tag_value = str(tag)
            # Option 2: Extract a specific key, if relevant:
            # tag_value = tag.get("tagName", "unknown_tag")
        else:
            tag_value = tag

        if tag_value not in tags_map:
            tags_map[tag_value] = []
        tags_map[tag_value].append(item)

# 3. Define helper functions to retrieve data.

def get_records_by_category(category_name):
    """
    Returns all CWE records that match the specified category.
    """
    return category_map.get(category_name, [])

def get_records_by_tag(tag_value):
    """
    Returns all CWE records that have the specified tag in their Additional_tags list.
    """
    return tags_map.get(tag_value, [])

# Example usage:
def get_related_cwe_list(tags_string):
    # tags_string = '["Data safety and security","Web Development, API & Web Services"]'
    # tags_string = '["Language Basics", "Concurrency & Parallelism", "File & I/O Handling"]'
    # Convert the JSON-formatted string into a Python list
    # if not tags_string:
    cleaned = (
        tags_string
        .replace("TextBlock(citations=None, text=\'```json\\n", "")
        .replace("\\n```', type='text'", "")
        .replace("```json", "")
        .replace("\\n", "")
        .replace("```", "")
        .replace("', type='text')", "")
        .replace("TextBlock(text='", "")
        .replace("citations=None,", "")
        .replace(")", "")
        .replace("text='", "")
        .replace("TextBlock( ", "")
    )
    import re
    print(cleaned)
    # Remove any text between <think>...</think> (including multiline)
    # cleaned = re.sub(r"<think>.*?</think>", "", cleaned, flags=re.DOTALL)
    # lines = [line.strip() for line in cleaned.strip().splitlines() if line.strip()]
    # last_line = lines[-1] if lines else ""
    tags_list = json.loads(cleaned)
    # else:
    #     tags_list=["Language Basics"]
    tags_list+=["Language Basics"]
    web_dev_records=[]
    # Iterate over each item in the list and print it
    for tag in tags_list:
        print(tag)
        web_dev_records += get_records_by_category(tag)
        web_dev_records += get_records_by_tag(tag)

# Deduplicate while preserving order
    seen = set()
    unique_records = []
    for rec in web_dev_records:
        sig = signature(rec)
        if sig not in seen:
            seen.add(sig)
            unique_records.append(rec)

    web_dev_records = unique_records

    cwe_list=""
    # Iterate over each record and print desired fields
    counter = 0  # Initialize the counter

    for record in web_dev_records:
        counter += 1  # Increment for each record
        cwe_id = record["cwe_id"]
        cwe_name = record["cwe_name"]
        cwe_description:str = record["cwe_description"]
        mitigation:str = record["mitigation"]
        cwe_list += f"[\"Rules#{counter}: {mitigation}\"],\n"
        # cwe_list += f"[\"CWE_ID\": \"{cwe_id}\", \"\": \"{mitigation}\"],\n"
        # cwe_list += f"[\"CWE_ID\": \"{cwe_id}\", \"CWE_Name\": \"{cwe_name}\", \"CWE_Description\": \"{cwe_description.replace("\n", " ")}\"],\n"

    return cwe_list

def signature(obj):
    """Create a stable, hashable signature for any JSON-like object."""
    try:
        # Fast path: canonical JSON with sorted keys
        return json.dumps(obj, sort_keys=True, separators=(",", ":"), ensure_ascii=False, default=repr)
    except TypeError:
        # Fallback: recursive freeze for odd types
        return _freeze(obj)

def _freeze(obj):
    if isinstance(obj, dict):
        return tuple((k, _freeze(v)) for k, v in sorted(obj.items()))
    if isinstance(obj, list) or isinstance(obj, tuple):
        return tuple(_freeze(x) for x in obj)
    if isinstance(obj, set):
        return tuple(sorted(_freeze(x) for x in obj))
    # Last resort: use repr for anything else
    return obj if isinstance(obj, (str, int, float, bool, type(None))) else repr(obj)

# # Example usage:
# if __name__ == "__main__":
#     tags_string = '["Data safety and security","Web Development, API & Web Services"]'
#     # tags_string = '["Language Basics", "Concurrency & Parallelism", "File & I/O Handling"]'
#     # Convert the JSON-formatted string into a Python list
#     tags_list = json.loads(tags_string)
#
#     web_dev_records=[]
#     # Iterate over each item in the list and print it
#     for tag in tags_list:
#         print(tag)
#         web_dev_records += get_records_by_category(tag)
#         web_dev_records += get_records_by_tag(tag)
#
#     # Iterate over each record and print desired fields
#     for record in web_dev_records:
#         cwe_id = record["cwe_id"]
#         cwe_name = record["cwe_name"]
#         cwe_description = record["cwe_description"]
#
#         print(f"[\"CWE_ID\": \"{cwe_id}\", \"CWE_Name\": \"{cwe_name}\", \"CWE_Description\": \"{cwe_description}\"]")
#         # print(f"CWE Name: {cwe_name}")
#         # print(f"CWE Description: {cwe_description}")
#         print("-" * 5)  # Just a separator for clarity
#     # # Fetch by tag
#
#     print("-" * 50)  # Just a separator for clarity

    # data_security_records = get_records_by_tag("Data safety and security")
    # for record in data_security_records:
    #     cwe_id = record["cwe_id"]
    #     cwe_name = record["cwe_name"]
    #     cwe_description = record["cwe_description"]
    #
    #     print(f"[\"CWE_ID\": \"{cwe_id}\", \"CWE_Name\": \"{cwe_name}\", \"CWE_Description\": \"{cwe_description}\"]")
    #     # print(f"CWE Name: {cwe_name}")
    #     # print(f"CWE Description: {cwe_description}")
    #     print("-" * 5)  # Just a separator for clarity
    # # print(f"Records with 'Data safety and security' in Additional_tags:\n{data_security_records}\n")
