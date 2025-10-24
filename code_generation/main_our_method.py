import datetime
import json
import logging
import os
import re
import time

from .ds_manipulation import get_related_cwe_list
from .code_processor import CodeProcessor
from .llm_integration.ModelIntegrationHandler import ModelIntegrationHandler  # <-- Import the new class

# ------------------------------
# Global Configuration
# ------------------------------

# promptType = "Vanilla"
promptType = "ourMethod"
# promptType = "CoT"
# promptType = "ZeroShot"

# Directory where JSON files are located
# input_directory = 'C:/data/PhD/paper3/dataset/promptDescription-0XX'
# input_directory = 'C:/data/PhD/paper3/dataset/promptDescription-1XX'
# input_directory = f'C:/data/PhD/paper3/dataset/{promptType}/missinggpt-4o'
# input_directory = f'C:/data/PhD/paper3/dataset/{promptType}/missinggemini-1.5-pro-002'
# input_directory = f'C:/data/PhD/paper3/dataset/{promptType}/missingllama-3.1-sonar-large-128k-online'
input_directory = f'./dataset/{promptType}'
# input_directory = f'C:/data/PhD/paper3/dataset/{promptType}/missingcodestral-latest'

openai_model = ["OPENAI", "gpt-4o"]  # https://platform.openai.com/docs/models
gemini_model = ["GEMINI", "gemini-1.5-pro-002"] # gemini-1.5-pro-001 gemini-1.0-pro-001 gemini-1.5-flash-001 https://console.cloud.google.com/vertex-ai/generative/language/create/text?createType=code&project=gen-lang-client-0375481745
perplexity_model = ["PERPLEXITY", "llama-3.1-sonar-large-128k-online"] # https://docs.perplexity.ai/guides/model-cards --- https://www.perplexity.ai/settings/api
claude_model = ["CLAUDE", "claude-3-5-sonnet-20241022"] # https://docs.anthropic.com/en/docs/about-claude/models xxxxx
# claude_model = ["CLAUDE", "claude-3-5-sonnet-20240620"] # https://docs.anthropic.com/en/docs/about-claude/models
mistral_model = ["MISTRAL", "codestral-latest"]    # https://docs.mistral.ai/getting-started/models/ mistral-large-latest version	24.11

# Set the active integration
# active_integration = openai_model
# active_integration = gemini_model
# active_integration = perplexity_model
active_integration = claude_model # failed, todo need to check
# active_integration = mistral_model

# Configure logging
today = datetime.date.today().strftime('%Y-%m-%d_%H-%M-%S')
logging.basicConfig(
    filename=f"./logs/processing_{active_integration[0]}_{active_integration[1]}_{today}.log",
    level=logging.DEBUG,
    format="%(asctime)s %(levelname)s %(message)s"
)
logger = logging.getLogger(__name__)

# Define output directory
output_directory = f"./dataset/{promptType}/{active_integration[0]}_{active_integration[1]}"
os.makedirs(output_directory, exist_ok=True)

# Instruction message
instruction_messageT = ('Instructions:\n'
                        '1) The output should include only the code, do not include any other output or descriptions.\n'
                        '2) Write the code in the following four programming languages: Java, Python, CPP, and C. all function should accept the input as parameters and return the output if any.\n'
                        '3) Put the code like the below template/example:\n'
                        '```java  \n'
                        'code here:\n'
                        '```\n'
                        '```python  \n'
                        'code here:\n'
                        '```\n'
                        '```cpp  \n'
                        'code here:\n'
                        '```\n'
                        '```c  \n'
                        'code here:\n'
                        '```\n'
                        '4) If you cannot provide the code for any of these programming languages, just put a comment inside the above code block template.\n'
                        '5) we need the output program run as one source code file. All code needs to be in one file.\n')

def sorted_directory_listing_with_os_listdir(directory):
    items = os.listdir(directory)
    return sorted(items)

if __name__ == "__main__":
    # Initialize our new ModelIntegrationHandler
    model_handler = ModelIntegrationHandler(logger)
    print(f"active_integration ------------------ {active_integration}")
    logger.info(f"active_integration ------------------ {active_integration}")
    # Iterate over each file in the input directory
    for filename in sorted_directory_listing_with_os_listdir(input_directory):
        if filename.endswith('.json'):
            file_path = os.path.join(input_directory, filename)

            # Read JSON data from the file
            with open(file_path, mode='r', encoding="utf8") as file:
                data = json.load(file)

            # Extract the prompt_description from the JSON data
            prompt_description = data['tasks'][0]['prompt_description']
            prompt_description_id = data['tasks'][0]['task_number']
            task_number = data['tasks'][0]['task_number']
            prompt_title = data['tasks'][0]['prompt_title']
            output_filename = "Task" + str(task_number) # + '_' + active_integration[0] + '_' + active_integration[1]

            # Phase 1 - Prompt description tagging
            # Construct the instruction message
            taggin_instruction_message = fr"""
            You are a secure code classification assistant. Your task is to examine the given prompt description 
            (i.e., a programming task), then identify which of the following predefined tags are most relevant 
            based on the problem statement. 
            
            Note: The assigned tags will be used to link this prompt to corresponding 
            vulnerabilities or weaknesses (CWE entries) that might arise in the final code. 
            Therefore, it is crucial that you only select the tags that accurately reflect the 
            prompt's requirements or concerns.
            
            Predefined Tags:
            1. Memory & Resource Management
               - Dynamic allocation, stack vs. heap usage, pointer arithmetic, resource leaks.
            2. Concurrency & Parallelism
               - Multi-threading, synchronization mechanisms, lock management, race conditions.
            3. Networking & Communication
               - Socket programming, client-server architecture, protocols (TCP/UDP), web sockets.
            4. Data safety and security
               - All sensitive data need to be encrypted in transition and at rest, storage, database, files.
            5. Web Development, API & Web Services
               - Back-end frameworks, REST APIs, session management, front-end integration. 
                 Building/consuming RESTful APIs, microservices, GraphQL.
            6. Database
               - SQL or No-SQL queries, ORM frameworks, transaction management.
            7. Input Validation & Sanitization
               - Sanitizing user input, type checks, length checks, preventing injections.
            8. File & I/O Handling
               - Reading, writing, directory traversal, file permissions, path handling.
            9. Cryptography
               - Encryption/decryption, hashing, key management, SSL/TLS usage.
            10. Authentication & Authorization
                - Password handling, login flows, session tokens, RBAC.
            11. Error Handling
                - Exception capturing.
            12. Code Injection
                - OS command injection, script injection.
            13. Serialization & Deserialization
                - Safely (de)serializing JSON, XML, or binary data.
            14. Hardcoding
                - Hardcoding credentials and secret data.
            15. Language Basics
                - Language Basics always applicable for all languages and tasks.

            Instructions:
            1. Read the `prompt_description` below.
            2. From the Predefined Tags list above, pick ONLY the tags that are relevant to the prompt.
            3. Return these tags in a JSON array (of strings).
            4. Do not include any explanations or additional commentary, and do not include tags that do not apply.
            5. If multiple tags apply, list them all, e.g. ["Language Basics", "Concurrency & Parallelism", "File & I/O Handling"].
            6. If no tags apply, return the default array ["Language Basics"].
            
            Remember: These chosen tags will be used for further mapping to potential CWE vulnerabilities.
            
            Prompt Description:
            {prompt_description}
            
            Required Output Format:
            ["TAG_NAME_1", "TAG_NAME_2", ...]
            (Only the array of relevant tags, nothing else.)
            """

            logger.info(f"tagging_instruction_message -- {taggin_instruction_message}")

            # ------------------------------
            # Use the new ModelIntegrationHandler
            # ------------------------------
            if active_integration[0] == claude_model[0]:
                tag_list_response = model_handler.generate_model_response(
                    active_integration=active_integration,
                    gemini_model=gemini_model,
                    mistral_model=mistral_model,
                    claude_model=claude_model,
                    openai_model=openai_model,
                    perplexity_model=perplexity_model,
                    instruction_message="Classify the prompt",
                    prompt_description=taggin_instruction_message,
                )
            else:
                tag_list_response = model_handler.generate_model_response(
                    active_integration=active_integration,
                    gemini_model=gemini_model,
                    mistral_model=mistral_model,
                    claude_model=claude_model,
                    openai_model=openai_model,
                    perplexity_model=perplexity_model,
                    instruction_message=taggin_instruction_message,
                    prompt_description="",
                )

            print(f"Assigned tags -->>>>{tag_list_response}")
            logger.info(f"Assigned tags -->>>>{tag_list_response}")
            # # Convert the JSON-formatted string into a Python list
            # tags_list = json.loads(model_response)
            #
            # # Iterate over each item in the list and print it
            # for tag in tags_list:
            #     print(tag)
            related_cwe_list = get_related_cwe_list(tag_list_response)
            # print(get_related_cwe_list(tag_list_response))
            time.sleep(2)

            # Phase 2 - code generation

            # Construct the instruction message
            instruction_message = instruction_messageT + f'6) Give name of the main class in the generated code should be {output_filename}, if any. \n'
            instruction_message = instruction_message + f'7) If applicable, include in the generate code a main method with 5 test cases. \n'
            instruction_message = (instruction_message +
                                   (f'Write the code of the following prompt description based on the below Chain-of-Through process:\n {prompt_description}\n'
                                    f'\n Chain-of-Through process in code generation:'+
                                    "\n1.   	Problem understanding: clearly define the purpose of the code, what functionality or logic does the code need to implement, and identify any potential inputs, outputs, and operations involved."+
                                    f"\n2.   	Security requirements: ensure that code generation is free of vulnerability and security weaknesses. List of possible weaknesses to avoid as following: \n {related_cwe_list}"+
                                    "\n3.   	Secure coding generation: write the code step-by-step, ensuring the code is preventing all security weaknesses."+
                                    "\n4.   	Code review: conduct a review for the generated code line by line, to ensure there is no security weaknesses."+
                                    "\n5.   	Secure code output: produce the final code based on fixing the reviewed code, to ensure all vulnerabilities identified earlier are mitigated. \n"
                                    ))


            print(f"{output_filename}_{instruction_message}")
            logger.info(f"{output_filename}_{instruction_message}")

            # ------------------------------
            # Use the new ModelIntegrationHandler
            # ------------------------------
            model_response = model_handler.generate_model_response(
                active_integration=active_integration,
                gemini_model=gemini_model,
                mistral_model=mistral_model,
                claude_model=claude_model,
                openai_model=openai_model,
                perplexity_model=perplexity_model,
                instruction_message=instruction_message,
                prompt_description=prompt_description,
            )

            # ------------------------------
            # Process Generated Code
            # ------------------------------
            codeProcessor = CodeProcessor(logger, model_response, output_directory, output_filename)
            try:
                codeProcessor.process_sections()
            except Exception as e:
                print(f"An error occurred while codeProcessor.process_sections: {e}")
                logger.error(f"An error occurred while codeProcessor.process_sections: {e}")

            time.sleep(2)  # Optional: Sleep to avoid hitting rate limits

    print("All files processed.")
    logger.info("All files processed.")
