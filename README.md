## Introduction
Dataset for evaluating LLMs generated code, developed for "An Empirical Evaluation of LLM-Generated Code Security Across Prompting Methods" research paper.

## Getting Started
To get started with the project, follow these steps:

1. Clone the repository: `git clone https://github.com/mohkharma/paper2.git`
2. Review the prompt description files existing in the repository - dataset/promptDescription-XXX.
3. Review the prompt description schema to understand the dataset instance attributes [directory](dataset/promptDescriptionSchema.json).


## Usage
Here's a brief guide on how to use the project, inside the code_generation directory:

* Start generation of responses from the AI models using main<prompt_method_name>.py]

* Check for generation completeness using [dir_with_file_count.py](code_generation/dir_with_file_count.py) and [extract-missing-files.py](code_generation/extract-missing-files.py)

* Reprocess the missing using [reprocess_ai_response.py](code_generation/reprocess_ai_response.py)

* Fix java class name using [fix_public_class_name_for_java.py](code_generation/fix_public_class_name_for_java.py)


## Generated Code and Unit Tests Repositories:

- [C Language Generated Code](https://github.com/mohsystem/llm-generated-code-c-paper2)
- [Python Generated Code](https://github.com/mohsystem/llm-generated-code-python-paper2)
- [C++ Generated Code](https://github.com/mohsystem/llm-generated-code-cpp-paper2)
- [Java Generated Code](https://github.com/mohsystem/llm-generated-code-java-paper2)


## SonarQube Scanning

`sonar-scanner.bat -D"sonar.projectKey=<projectKey>" -D"sonar.so
urces=." -D"sonar.host.url=http://localhost:9000" -D"sonar.token=<token>"`

# Download sonar-scanner:
https://docs.sonarsource.com/sonarqube/10.6/analyzing-source-code/scanners/sonarscanner/


# Fix elasticsearch issue
https://stackoverflow.com/questions/42111566/elasticsearch-in-windows-docker-image-vm-max-map-count

# Running the CFamily analysis
https://docs.sonarsource.com/sonarqube-server/latest/analyzing-source-code/languages/c-family/running-the-analysis/

## DS Summary and results analysis
[analysis-statistical](analysis-statistical)

[analysis-cwe-severity](analysis-cwe-severity)

https://docs.google.com/spreadsheets/d/16HwGvOLxTP7EBbEPBX9tfmVBFcXTtGTtzLYPcA6K83U/edit?usp=sharing
