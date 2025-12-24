## Introduction
Dataset for evaluating LLMs generated code, developed for "Mitigation-Aware Prompt Engineering for Secure Code Generation" research paper.
The current benchmark focuses on self-contained tasks at the function and file level. These tasks are designed to isolate core building blocks that frequently appear in real applications, while still being small enough to analyze systematically and at scale.


## Getting Started
To get started with the project, follow these steps:

1. Clone the repository: `git clone https://github.com/mohsystem/paper3.git`
2. Review the prompt description files existing in the repository - dataset/promptDescription-XXX.
3. Review the prompt description schema to understand the dataset instance attributes [directory](dataset/promptDescriptionSchema.json).

 
## Usage
Here's a brief guide on how to use the project, inside the code_generation directory:

* Start generation of responses from the AI models using main<prompt_method_name>.py]

* Check for generation completeness using [dir_with_file_count.py](code_generation/dir_with_file_count.py) and [extract-missing-files.py](code_generation/extract-missing-files.py)

* Reprocess the missing using [reprocess_ai_response.py](code_generation/reprocess_ai_response.py)

* Fix java class name using [fix_public_class_name_for_java.py](code_generation/fix_public_class_name_for_java.py)


## Generated Code and Unit Tests Repositories:

- [Python Generated Code](https://github.com/mohsystem/llm-generated-code-python-paper3)
- [Java Generated Code](https://github.com/mohsystem/llm-generated-code-java-paper3)
- [C Language Generated Code](https://github.com/mohsystem/llm-generated-code-c-paper3)


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
[sast-analysis](sast-analysis)
[analysis-cwe-severity](analysis-cwe-severity)

https://docs.google.com/spreadsheets/d/16HwGvOLxTP7EBbEPBX9tfmVBFcXTtGTtzLYPcA6K83U/edit?usp=sharing
