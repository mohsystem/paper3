import datetime
import os
import json
import logging
import time
import re

from code_generation.llm_integration.claude_integration import ClaudeIntegration
from code_processor import CodeProcessor
from code_generation.llm_integration.gemini_integration import GeminiIntegration
from code_generation.llm_integration.mistral_integration import MistralIntegration
from code_generation.llm_integration.openai_integration import OpenAIIntegration
from code_generation.llm_integration.perplexity_integration import PerplexityIntegration

promptType = "ZeroShot"
# Directory where JSON files are located
# input_directory = 'C:/data/PhD/paper3/dataset/promptDescription-1XX'
input_file = 'C:/data/PhD/paper3/benchmarkDS/LLMSecEval-Prompts_dataset.csv'
# input_directory = 'C:/data/PhD/paper3/dataset/Vanilla/missingcodestral-latest'

openai_model = ["OPENAI", "gpt-5-2025-08-07"]  # https://platform.openai.com/docs/models/gpt-5
gemini_model = ["GEMINI", "gemini-2.5-pro"] # https://ai.google.dev/gemini-api/docs/text-generation
perplexity_model = ["PERPLEXITY", "sonar-reasoning"] # https://docs.perplexity.ai/guides/model-cards --- https://www.perplexity.ai/settings/api
# perplexity_model = ["PERPLEXITY", "llama-3.1-sonar-large-128k-online"] # https://docs.perplexity.ai/guides/model-cards --- https://www.perplexity.ai/settings/api
claude_model = ["CLAUDE", "claude-sonnet-4-5-20250929"] # https://docs.claude.com/en/docs/about-claude/model-deprecations#model-status  https://docs.claude.com/en/docs/about-claude/models/overview
mistral_model = ["MISTRAL", "codestral-latest"]    # https://docs.mistral.ai/getting-started/models/ mistral-large-latest version	24.11

# Set the active integration
# active_integration = openai_model
active_integration = gemini_model
# active_integration = perplexity_model
# active_integration = claude_model
# active_integration = mistral_model

# Get today's date in YYYY-MM-DD format
today = datetime.date.today().strftime('%Y-%m-%d_%H-%M-%S')

# Configure logging to write to a file with date in filename
logging.basicConfig(filename=f"C:/data/PhD/paper3/benchmarkDS/logs/processing_{active_integration[0] + '_' + active_integration[1]}_{today}.log", level=logging.DEBUG, format="%(asctime)s %(levelname)s %(message)s")

logger = logging.getLogger(__name__)

# Directory where output files will be stored
output_directory = f"C:/data/PhD/paper3/benchmarkDS/{promptType}/{active_integration[0]+'_' + active_integration[1]}"

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

# Ensure output directory exists or create it
os.makedirs(output_directory, exist_ok=True)

import pandas as pd

# Load the CSV file
df = pd.read_csv(input_file)
# Limit to the first 10 records
# df_head = df.head(10)
# df_head = df.iloc[20:] #Select the next 10 records (rows 11(index 10)–20)

# Loop over each row and print ID and "Manually-fixed NL Prompt"
# for index, row in df.iterrows():  # read all the file
for index, row in df.iterrows():
    id_value = row.get("ID", None)
    prompt = row.get("Manually-fixed NL Prompt", None)
    print(f"ID: {id_value}\nPrompt: {prompt}\n{'-'*40}")
    # Extract the prompt_description from the JSON data
    prompt_description = prompt
    # prompt_description_id = data['tasks'][0]['task_number']
    task_number = id_value
    # prompt_title = data['tasks'][0]['prompt_title']
    output_filename = "Lse" + str(task_number) # + '_' + active_integration[0] + '_' + active_integration[1]

    # logger.info(output_filename + "_" + instruction_message + prompt_description)

    instruction_message = instruction_messageT + f'6) Give name of the main class in the generated code should be {output_filename}, if any. \n'
    instruction_message = instruction_message + f'7) If applicable, include in the generate code a main method with 5 test cases. \n'
    instruction_message = instruction_message + 'Write a secure code for the following prompt description: \n'

    print(output_filename + "_" + instruction_message + prompt_description)

    # Print the prompt_description
    # print(instruction_message + prompt_description)
    model_response = None
    # Call the AI model integration
    generated_text = None
    if active_integration[0] == gemini_model[0]:
        model_name = gemini_model[1]
        # model_name = 'gemini-1.0-pro-latest'
        # Initialize Gemini Model
        processor = GeminiIntegration(model_name)

        # Generate content using the model
        try:
            generated_text = processor.generate_content(instruction_message + prompt_description)
            model_response = generated_text
            logger.info(generated_text)
            print(generated_text)
        except Exception as e:
            print(f"An error occurred while generating content: {e}")
            logger.error(f"An error occurred while generating content: {e}")
            generated_text = ""
    elif active_integration[0] == mistral_model[0]:
        model_name = mistral_model[1]
        # Initialize Mistral Model
        processor = MistralIntegration(model_name)

        # Generate content using the model
        generated_text = processor.generate_content(instruction_message + prompt_description)
        model_response = generated_text
        logger.info(generated_text)
        print(generated_text)
    elif active_integration[0] == claude_model[0]:
        model_name = claude_model[1]
        # Initialize CLAUDE Model
        processor = ClaudeIntegration(model_name)

        # Generate content using the model

        try:
            generated_text = processor.generate_content(instruction_message, prompt_description)
            model_response = ','.join(map(str, generated_text))
            model_response = re.sub(r"\\'", "'", model_response)
            logger.info(model_response)
            print(model_response)
        except Exception as e:
            print(f"An error occurred while generating content: {e}")
            logger.error(f"An error occurred while generating content: {e}")
            generated_text = ""
            model_response = ""

    elif active_integration[0] == openai_model[0]:
        # Initialize OpenAI GPT Model
        selected_model = openai_model[1]
        openAIIntegration = OpenAIIntegration()
        generated_text = openAIIntegration.get_completion_content(instruction_message + prompt_description, selected_model)
        try:
            model_response = generated_text.choices[0].message.content
            print(generated_text.choices[0].message.content)
            logger.info(model_response)
        except Exception as e:
            print(f"An error occurred while generating content: {e}")
            logger.error(f"An error occurred while generating content: {e}")
            generated_text = ""
            model_response = ""

    elif active_integration[0] == perplexity_model[0]:
        # Initialize PERPLEXITY Model
        selected_model = perplexity_model[1]
        perplexityIntegration = PerplexityIntegration()
        try:
            generated_text = perplexityIntegration.get_completion_content(instruction_message + prompt_description, selected_model)
            model_response = generated_text.choices[0].message.content
            print(generated_text.choices[0].message.content)
            logger.info(model_response)
        except Exception as e:
            print(f"An error occurred while generating content: {e}")
            logger.error(f"An error occurred while generating content: {e}")
            generated_text = ""
            model_response = ""
    else:
        print(f"Unsupported active integration: {active_integration}")
        logger.info(f"Unsupported active integration: {active_integration}")

    # Write the prompt_description to a new file in the output directory
    # + '.' + programming_language
    # output_path = os.path.join(output_directory, output_filename)

    codeProcessor = CodeProcessor(logger, model_response, output_directory, output_filename)

    try:
        codeProcessor.process_sections()
    except Exception as e:
        print(f"An error occurred while codeProcessor.process_sections: {e}")
        logger.error(f"An error occurred while codeProcessor.process_sections: {e}")
    # Sleep for 2 seconds
    time.sleep(3)


# with open(output_path, 'w') as output_file:
        #     output_file.write(completion_content.choices[0].message.content.replace('```java', '') .replace('```', ''))

        # print(f'Created file: {output_path}')

print('All files processed.')
logger.info('All files processed.')
