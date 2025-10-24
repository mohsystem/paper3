import os
from collections import defaultdict

import os
import shutil
import os
import re
# This program is to extract the failed to generated prompts
# openAIIntegration = OpenAIIntegration()
openai_model = ["OPENAI", "gpt-5-2025-08-07"]  # https://platform.openai.com/docs/models/gpt-5
gemini_model = ["GEMINI", "gemini-2.5-pro"] # https://ai.google.dev/gemini-api/docs/text-generation
perplexity_model = ["PERPLEXITY", "sonar-reasoning"] # https://docs.perplexity.ai/guides/model-cards --- https://www.perplexity.ai/settings/api
# perplexity_model = ["PERPLEXITY", "llama-3.1-sonar-large-128k-online"] # https://docs.perplexity.ai/guides/model-cards --- https://www.perplexity.ai/settings/api
claude_model = ["CLAUDE", "claude-sonnet-4-5-20250929"] # https://docs.claude.com/en/docs/about-claude/model-deprecations#model-status  https://docs.claude.com/en/docs/about-claude/models/overview
mistral_model = ["MISTRAL", "codestral-latest"]    # https://docs.mistral.ai/getting-started/models/ mistral-large-latest version	24.11

# todo change this based on your case.
basePath = 'C:/data/PhD/paper3/dataset/'
# promptType = "Vanilla"
promptType = "ourMethod"
# promptType = "CoT"
# promptType = "ZeroShot"

# Function to extract task numbers from filenames
def extract_task_numbers(directory):
    task_numbers = []
    pattern = re.compile(r'Task(\d+)')

    for file_name in os.listdir(directory):
        match = pattern.search(file_name)
        if match:
            task_number = int(match.group(1))
            task_numbers.append(task_number)

    return sorted(task_numbers)

# Function to find missing numbers in a sequence
from collections import Counter

def find_missing_numbers(task_numbers):
    if not task_numbers:
        return []

    # Get the full range of numbers from the minimum to the maximum in the input
    full_range = set(range(task_numbers[0], task_numbers[-1] + 1))

    # Count occurrences of each number in the input
    counts = Counter(task_numbers)

    # Find numbers missing entirely or not appearing exactly 4 times
    missing_numbers = [
        num for num in full_range
        if counts.get(num, 0) != 4  # Either not present or not appearing 4 times
    ]

    return sorted(missing_numbers)

# Dictionary to store the models
models = {
    "openai_model": openai_model,
    # "gemini_model": gemini_model,
    # "perplexity_model": perplexity_model
    # "claude_model": claude_model,
    # "mistral_model": mistral_model
}

# Loop to iterate through models and execute them once
for model_name, model_values in models.items():
    active_integration = model_values
    print(f"active_integration = {active_integration}")
    print(f"Model Details: Name = {model_values[0]}, model = {model_values[1]}")

    destination_directory = f'{basePath}{promptType}/missing{active_integration[1]}'
    os.makedirs(destination_directory, exist_ok=True)

    # Directory containing the files
    directory = f"{basePath}{promptType}/{active_integration[0] + '_' + active_integration[1]}"

    # Extract task numbers from filenames
    task_numbers = extract_task_numbers(directory)
    # print(f"Task Numbers: {task_numbers}")

    # Find missing task numbers
    missing_numbers = find_missing_numbers(task_numbers)
    print(f"Missing Task Numbers: {missing_numbers}")

    for file_name in missing_numbers:
        print(f"{file_name}")
        formatted_task_number = f"{int(file_name):03d}"

        # Source and destination directories
        source_directory = f'{basePath}/promptDescription-0XX'  # Update this with the actual path


        # File name to copy
        file_name = f"promptDescription-{formatted_task_number}.json"
        # Construct full file paths
        source_file_path = os.path.join(source_directory, file_name)
        destination_file_path = os.path.join(destination_directory, file_name)

        # Check if the source file exists
        if os.path.exists(source_file_path):
            # Copy the file to the destination directory
            shutil.copy(source_file_path, destination_file_path)
            print(f"File copied to: {destination_file_path}")
        else:
            print(f"Source file does not exist: {source_file_path}")
            source_directory = f'{basePath}promptDescription-1XX'  # Update this with the actual path
            source_file_path = os.path.join(source_directory, file_name)

            # Copy the file to the destination directory
            shutil.copy(source_file_path, destination_file_path)
            print(f"File copied to: {destination_file_path}")

