
import subprocess
import shlex
import signal
from typing import Optional

class Task66:
    ALLOWED_COMMANDS = {'ls', 'pwd', 'date', 'whoami', 'echo'}
    TIMEOUT_SECONDS = 5
    MAX_OUTPUT_LENGTH = 10000
    
    @staticmethod
    def execute_command(user_input: str) -> str:
        if not user_input or not user_input.strip():
            return "Error: Command cannot be empty"
        
        user_input = user_input.strip()
        
        # Validate: no dangerous characters
        forbidden_chars = [';', '&', '|', '>', '<', '`', '$', '\\\\', '\
', '\\r']\n        if any(char in user_input for char in forbidden_chars):\n            return "Error: Command contains forbidden characters"\n        \n        # Parse command and arguments safely\n        try:\n            parts = shlex.split(user_input)\n        except ValueError as e:\n            return f"Error: Invalid command format - {str(e)}"\n        \n        if not parts:\n            return "Error: No command provided"\n        \n        command = parts[0]\n        \n        # Whitelist validation\n        if command not in Task66.ALLOWED_COMMANDS:\n            return f"Error: Command '{command}' is not in the allowed list"\n        \n        try:\n            # Execute with timeout and capture output\n            result = subprocess.run(\n                parts,\n                capture_output=True,\n                text=True,\n                timeout=Task66.TIMEOUT_SECONDS,\n                shell=False  # Critical: Never use shell=True\n            )\n            \n            output = result.stdout + result.stderr\n            \n            # Limit output length\n            if len(output) > Task66.MAX_OUTPUT_LENGTH:\n                output = output[:Task66.MAX_OUTPUT_LENGTH] + "\\n[Output truncated]"\n            \n            if not output:\n                return "Command executed successfully (no output)"\n            \n            return output\n            \n        except subprocess.TimeoutExpired:\n            return "Error: Command execution timeout"\n        except FileNotFoundError:\n            return f"Error: Command '{command}' not found"\n        except Exception as e:\n            return f"Error: {str(e)}"\n    \n    @staticmethod\n    def main():\n        print("=== Secure Command Executor Test Cases ===\
")\n        \n        test_cases = [\n            "ls",\n            "pwd",\n            "echo Hello World",\n            "ls; rm -rf /",  # Malicious attempt\n            "cat /etc/passwd"  # Unauthorized command\n        ]\n        \n        for i, test in enumerate(test_cases, 1):\n            print(f"Test Case {i}: {test}")\n            print(f"Result: {Task66.execute_command(test)}")\n            print("---")\n\nif __name__ == "__main__":
    Task66.main()
