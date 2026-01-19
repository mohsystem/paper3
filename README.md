# Mitigation-Aware Prompt Engineering for Secure Code Generation (MA-CoT)

This repository provides the dataset and tooling used in our study on **secure LLM code generation** with **Mitigation-Aware Chain-of-Thought (MA-CoT)** prompting. MA-CoT is an inference-time prompt framework that injects **task-specific CWE mitigation cues**, a **baseline secure-coding ruleset**, and **language-aware safeguards** to reduce recurring vulnerabilities in LLM-generated code.

The benchmark consists of self-contained tasks at the function and file level. Tasks are designed to capture common building blocks used in real applications while remaining small enough for systematic, scalable evaluation. The repository also includes SonarQube scanning support and standardized reports that map findings to **language-core vs. language-stack** drivers (runtime, ecosystem libraries/frameworks, OS/platform APIs, and toolchains).

## Getting Started

1. Clone the repository:
   ```bash
   git clone https://github.com/mohsystem/paper3.git
   cd paper3
   ```

2. Review the prompt description files:
   - `dataset/promptDescription-XXX`

3. Review the prompt description schema:
   - `dataset/promptDescriptionSchema.json`

## Usage

A brief guide for running the pipeline from the `code_generation/` directory:

- Start generating responses using the prompt-method entrypoints (naming pattern):
  - `main_<prompt_method_name>.py`

- Check generation completeness:
  - `code_generation/dir_with_file_count.py`
  - `code_generation/extract-missing-files.py`

- Reprocess missing outputs:
  - `code_generation/reprocess_ai_response.py`

- Fix Java public class name mismatches:
  - `code_generation/fix_public_class_name_for_java.py`

## Generated Code and Unit Tests Repositories

- Python Generated Code: https://github.com/mohsystem/llm-generated-code-python-paper3  
- Java Generated Code: https://github.com/mohsystem/llm-generated-code-java-paper3  
- C Language Generated Code: https://github.com/mohsystem/llm-generated-code-c-paper3  

## Vulnerability Attribution (Excel Reports)

Standardized, language-specific vulnerability attribution reports (SonarQube findings mapped to attribution layers and mechanisms):

- `sast-analysis/c_vulnerability_analysis_standardized.xlsx`
- `sast-analysis/Java-vulnerability_analysis_standardized.xlsx`
- `sast-analysis/Python - vulnerability_analysis_standardized.xlsx`

## SonarQube Scanning

Example scan command (adjust values for your environment):

```bash
sonar-scanner   -D"sonar.projectKey=<projectKey>"   -D"sonar.sources=."   -D"sonar.host.url=http://localhost:9000"   -D"sonar.token=<token>"
```

### Download SonarScanner
https://docs.sonarsource.com/sonarqube/10.6/analyzing-source-code/scanners/sonarscanner/

### Fix Elasticsearch issue (common in Docker/Linux)
https://stackoverflow.com/questions/42111566/elasticsearch-in-windows-docker-image-vm-max-map-count

### Running the CFamily analysis
https://docs.sonarsource.com/sonarqube-server/latest/analyzing-source-code/languages/c-family/running-the-analysis/

## Results and Analysis

- SAST outputs and standardized attribution reports: `sast-analysis/`
- CWE and severity aggregation scripts/results: `analysis-cwe-severity/`

Reference spreadsheet (overview):
https://docs.google.com/spreadsheets/d/16HwGvOLxTP7EBbEPBX9tfmVBFcXTtGTtzLYPcA6K83U/edit?usp=sharing
