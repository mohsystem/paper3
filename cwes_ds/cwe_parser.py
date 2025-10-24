import os
import csv
import json
import openai
import xml.etree.ElementTree as ET
from time import sleep
from openai_integration import OpenAIIntegration
# Path to the CWE XML file. Update this if your file is in another location.
CWE_XML_PATH = "dataset/cwec_v4.16_all_13_12_2024.xml"

# openai.api_key = os.getenv("OPENAI_API_KEY")
# if not openai.api_key:
#     raise ValueError("OpenAI API key not set. Please set OPENAI_API_KEY environment variable.")

def parse_cwe_xml(xml_path):
    """
    Parse the CWE XML file and extract CWE ID, Name, and Description.
    Returns a list of dicts with cwe_id, cwe_name, and cwe_description.
    """
    tree = ET.parse(xml_path)
    root = tree.getroot()

    # The namespace is used in the XML. Let's extract it from the root tag if present.
    # Typically, root.tag looks like '{http://cwe.mitre.org/cwe-7}Weakness_Catalog'
    # We can extract the namespace by splitting the tag.
    ns = ""
    if root.tag.startswith("{"):
        ns = root.tag[1:].split("}")[0]

    weaknesses = []
    # Weaknesses are located at: Weakness_Catalog -> Weaknesses -> Weakness
    # With namespace, it would be something like: {ns}Weaknesses/{ns}Weakness
    weaknesses_path = f"{{{ns}}}Weaknesses/{{{ns}}}Weakness"
    for w in root.findall(weaknesses_path):
        cwe_id = w.attrib.get("ID")
        cwe_name = w.attrib.get("Name", "Unknown CWE")
        # The description is an element: <Description>...</Description>
        desc_element = w.find(f"{{{ns}}}Description")
        cwe_description = desc_element.text if desc_element is not None else "No description available."
        weaknesses.append({
            "cwe_id": cwe_id,
            "cwe_name": cwe_name,
            "cwe_description": cwe_description
        })
    return weaknesses

def query_gpt_for_cwe_details(cwe_id, cwe_name, cwe_description):
    """
    Query ChatGPT for additional details given a CWE entry:
    - Category
    - Category description
    - Mitigation
    - Applicable programming languages
    """
    prompt = f"""
You are a security expert and software engineer. You are given information about a specific CWE (Common Weakness Enumeration).

CWE Information:
- CWE ID: {cwe_id}
- CWE Name: {cwe_name}
- CWE Description: {cwe_description}

Your task:
1. Provide a specific tag  for this CWE falls under. Tag should be from the below list of tags:
    Tag	                                    Tag Description
    C Language specific weakness	        C Language specific weakness.
    CPP Language specific weakness	        CPP Language specific weakness.
    Java Language specific weakness	        Java Language specific weakness.
    Python Language specific weakness	    Python Language specific weakness.
    Language Basics	                        Core language syntax, variables, data types, control flow, Code Quality Issues, Improper Adherence to Coding Standards. This is applicable for all languages and tasks. Consider the code complixity, maintanability and other code quality attributes.
    Memory & Resource Management	        Dynamic allocation, stack vs. heap usage, pointer arithmetic, resource leaks.
    Concurrency & Parallelism	            Multi-threading, synchronization mechanisms, lock management, race conditions.
    Networking & Communication	            Socket programming, client-server architecture, protocols (TCP/UDP), web sockets.
    Data safety and security	            All sensitive data need to be encrypted in transition and at rest, storage, database, files.
    Web Development, API & Web Services	    Backend frameworks, REST APIs, session management, front-end integration. Building/consuming RESTful APIs, microservices, GraphQL.
    Database	                            SQL or NoSQL queries, ORM frameworks, transaction management.
    Input Validation & Sanitization	        Sanitizing user input, type checks, length checks, preventing injections.
    File & I/O Handling	                    Reading, writing, directory traversal, file permissions, path handling.
    Cryptography	                        Encryption/decryption, hashing, key management, SSL/TLS usage.
    Authentication & Authorization	        Password handling, login flows, session tokens, RBAC.
    Error Handling	                        Exception capturing and handling.
    Logging	                                Structured logging, not exposing sensitive info in logs.
    Code Injection	                        OS command injection, script injection.
    Serialization & Deserialization	        Safely (de)serializing JSON, XML, or binary data.
    Configuration Errors & Hardcoding	    Misconfigured environment, debug mode in production, Hardcoding credentials and secret data.

2. Provide a concise mitigation statement: a guideline or approach to mitigate this CWE to the LLM when generate the code.
3. List any applicable programming languages if the CWE is specific to certain languages. If it's language-agnostic, state "Language-agnostic".
4. Provide the reasoning why this CWE is taged with the mentioned tag.
5. If there is additional tags applicable for the CWE, provide them:
Format your response as valid JSON with the following fields:
- "CWE ID"
- "CWE Name"
- "category"
- "category_description"
- "mitigation"
- "applicable_languages"
- "Reason_tag"
- "Additional_tags"
"""

    openai_model = ["OPENAI", "gpt-4o"]
    # Initialize OpenAI GPT Model
    selected_model = openai_model[1]
    openAIIntegration = OpenAIIntegration()
    response = openAIIntegration.get_completion_content(prompt, selected_model)
    # try:
    #     model_response = generated_text.choices[0].message.content
    #     print(generated_text.choices[0].message.content)
    #     logger.info(model_response)
    # except Exception as e:
    #     print(f"An error occurred while generating content: {e}")
    #     logger.error(f"An error occurred while generating content: {e}")
    #     generated_text = ""
    #     model_response = ""



    #
    #
    # response = openai.ChatCompletion.create(
    #     model="gpt-4",  # Use gpt-4 if you have access, otherwise "gpt-3.5-turbo"
    #     messages=[{"role": "user", "content": prompt}],
    #     temperature=0.2
    # )
# generated_text.choices[0].message.content
    content = response.choices[0].message.content.replace('```json', '') .replace('```', '').strip()
    # Attempt to parse JSON
    try:
        json_data = json.loads(content)
    except json.JSONDecodeError:
        # Fallback if JSON not valid
        json_data = {
            "category": "Unknown",
            "category_description": "No valid JSON response.",
            "mitigation": "No mitigation available.",
            "applicable_languages": "Unknown",
            "Reason_tag": "Unknown",
            "Additional_tags": "Unknown"
        }
    return json_data

def enrich_cwe_data(weaknesses, sleep_time=0.5):
    enriched = []
    for i, cwe in enumerate(weaknesses):
        cwe_id = cwe["cwe_id"]
        cwe_name = cwe["cwe_name"]
        cwe_description = cwe["cwe_description"]

        print(f"Processing CWE-{cwe_id}: {cwe_name}")
        gpt_info = query_gpt_for_cwe_details(cwe_id, cwe_name, cwe_description)

        enriched.append({
            "cwe_id": cwe_id,
            "cwe_name": cwe_name,
            "cwe_description": cwe_description,
            "category": gpt_info.get("category", "Unknown"),
            "category_description": gpt_info.get("category_description", "N/A"),
            "mitigation": gpt_info.get("mitigation", "N/A"),
            "applicable_languages": gpt_info.get("applicable_languages", "Unknown"),
            "Reason_tag": gpt_info.get("Reason_tag", "Unknown"),
            "Additional_tags": gpt_info.get("Additional_tags", "Unknown")
        })

        # Sleep to avoid rate limits
        sleep(sleep_time)
    return enriched

def save_to_csv(data, filename="dataset/enriched_cwe_dataset.csv"):
    fieldnames = ["cwe_id", "cwe_name", "cwe_description", "category", "category_description", "mitigation", "applicable_languages","Reason_tag", "Additional_tags"]
    with open(filename, mode="w", newline="", encoding='utf-8') as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        writer.writeheader()
        for row in data:
            writer.writerow(row)
    print(f"CSV saved to {filename}")

def save_to_json(data, filename="dataset/enriched_cwe_dataset.json"):
    with open(filename, mode="w", encoding='utf-8') as file:
        json.dump(data, file, indent=4, ensure_ascii=False)
    print(f"JSON saved to {filename}")

if __name__ == "__main__":
    print("Parsing CWE XML...")
    cwe_list = parse_cwe_xml(CWE_XML_PATH)
    if not cwe_list:
        print("No CWE data found in the XML.")
        exit(1)

    print(f"Found {len(cwe_list)} CWEs.")
    print("Enriching CWE data with GPT responses...")
    enriched = enrich_cwe_data(cwe_list)

    print("Saving results...")
    save_to_csv(enriched)
    save_to_json(enriched)

    print("Done.")
