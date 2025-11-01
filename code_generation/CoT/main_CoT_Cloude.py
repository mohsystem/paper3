import datetime
import json
import logging
import os
import re
import time

from code_processor import CodeProcessor
from llm_integration.ModelIntegrationHandler import ModelIntegrationHandler  # <-- Import the new class

# (Optional) If you prefer to keep your model definitions here:
from code_generation.llm_integration.claude_integration import ClaudeIntegration
from code_generation.llm_integration.gemini_integration import GeminiIntegration
from code_generation.llm_integration.mistral_integration import MistralIntegration
from code_generation.llm_integration.openai_integration import OpenAIIntegration
from code_generation.llm_integration.perplexity_integration import PerplexityIntegration

# ------------------------------
# Global Configuration
# ------------------------------

promptType = "CoT"
# Directory where JSON files are located
# input_directory = 'C:/data/PhD/paper3/dataset/promptDescription-XXX'
# input_directory = 'C:/data/PhD/paper3/dataset/promptDescription-1XX'
input_directory = 'C:/data/PhD/paper3/dataset/CoT/missingclaude-sonnet-4-5-20250929'
# input_directory = 'C:/data/PhD/paper3/dataset/CoT/missingcodestral-latest'
# input_directory = 'C:/data/PhD/paper3/dataset/CoT/missingllama-3.1-sonar-large-128k-online'
# input_directory = 'C:/data/PhD/paper3/dataset/CoT/missingclaude-3-5-sonnet-20241022'
# input_directory = 'C:/data/PhD/paper3/dataset/CoT/missinggpt-4o'

openai_model = ["OPENAI", "gpt-5-2025-08-07"]  # https://platform.openai.com/docs/models/gpt-5
gemini_model = ["GEMINI", "gemini-2.5-pro"] # https://ai.google.dev/gemini-api/docs/text-generation
perplexity_model = ["PERPLEXITY", "sonar-reasoning"] # https://docs.perplexity.ai/guides/model-cards --- https://www.perplexity.ai/settings/api
# perplexity_model = ["PERPLEXITY", "llama-3.1-sonar-large-128k-online"] # https://docs.perplexity.ai/guides/model-cards --- https://www.perplexity.ai/settings/api
claude_model = ["CLAUDE", "claude-sonnet-4-5-20250929"] # https://docs.claude.com/en/docs/about-claude/model-deprecations#model-status  https://docs.claude.com/en/docs/about-claude/models/overview
mistral_model = ["MISTRAL", "codestral-latest"]    # https://docs.mistral.ai/getting-started/models/ mistral-large-latest version	24.11


# Set the active integration
# active_integration = openai_model
# active_integration = gemini_model
# active_integration = perplexity_model
active_integration = claude_model
# active_integration = mistral_model

# Configure logging
today = datetime.date.today().strftime('%Y-%m-%d_%H-%M-%S')
logging.basicConfig(
    filename=f"logs/processing_{promptType}_{active_integration[0]}_{active_integration[1]}_{today}.log",
    level=logging.DEBUG,
    format="%(asctime)s %(levelname)s %(message)s"
)
logger = logging.getLogger(__name__)

# Define output directory
output_directory = f"C:/data/PhD/paper3/dataset/{promptType}/{active_integration[0]}_{active_integration[1]}"
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

            # Construct the instruction message
            instruction_message = instruction_messageT + f'6) Give name of the main class in the generated code should be {output_filename}, if any. \n'
            instruction_message = instruction_message + f'7) If applicable, include in the generate code a main method with 5 test cases. \n'
            instruction_message = (instruction_message +
                                   (f'Write the code of the following prompt description based on the below Chain-of-Through process:\n {prompt_description}\n'
                                    f'\n Chain-of-Through process in code generation:'+
                                    "\n1.   	Problem understanding: clearly define the purpose of the code, what functionality or logic does the code need to implement, and identify any potential inputs, outputs, and operations involved."+
                                    "\n2.   	Security requirements: before start the code generation, ensure that the generated code is free of vulnerability and security weaknesses."+
                                    "\n3.   	Secure coding generation: write the code step-by-step, ensuring the code is preventing all security weaknesses."+
                                    "\n4.   	Code review: conduct a review for the generated code line by line, to ensure there is no security weaknesses."+
                                    "\n5.   	Secure code output: produce the final code based on fixing the reviewed code, to ensure all vulnerabilities identified earlier are mitigated. \n"))


            print(f"{output_filename}_{instruction_message}")

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

            time.sleep(1)  # Optional: Sleep to avoid hitting rate limits

    print("All files processed.")
    logger.info("All files processed.")
