
import subprocess
import re
import shlex

class Task66:
    ALLOWED_COMMANDS = {'ls', 'pwd', 'date', 'whoami', 'echo'}
    DANGEROUS_PATTERN = re.compile(r'.*[;&|`$(){}\\[\\]<>].*|.*\\.\\..*')
    
    @staticmethod
    def execute_command(command):
        if not command or not command.strip():
            return "Error: Command cannot be empty"
        
        command = command.strip()
        
        # Check for dangerous characters
        if Task66.DANGEROUS_PATTERN.match(command):
            return "Error: Command contains dangerous characters"
        
        # Parse command safely
        try:
            parts = shlex.split(command)
        except ValueError as e:
            return f"Error: Invalid command format - {str(e)}"
        
        if not parts:
            return "Error: Command cannot be empty"
        
        base_command = parts[0]
        
        # Validate against whitelist
        if base_command not in Task66.ALLOWED_COMMANDS:
            return f"Error: Command not allowed. Allowed commands: {Task66.ALLOWED_COMMANDS}"
        
        # Validate arguments
        for arg in parts[1:]:
            if '/' in arg or '\\\\' in arg:\n                return "Error: Path traversal not allowed in arguments"\n        \n        # Execute command\n        try:\n            result = subprocess.run(\n                parts,\n                capture_output=True,\n                text=True,\n                timeout=5,\n                check=False\n            )\n            \n            if result.returncode != 0:\n                return f"Error: Command exited with code {result.returncode}\\n{result.stdout}{result.stderr}"\n            \n            return result.stdout.strip()\n            \n        except subprocess.TimeoutExpired:\n            return "Error: Command execution timed out"\n        except Exception as e:\n            return f"Error executing command: {str(e)}"\n\ndef main():\n    print("=== Test Case 1: Valid command 'pwd' ===")\n    print(Task66.execute_command("pwd"))\n    print()\n    \n    print("=== Test Case 2: Valid command 'date' ===")\n    print(Task66.execute_command("date"))\n    print()\n    \n    print("=== Test Case 3: Valid command 'echo' with arguments ===")\n    print(Task66.execute_command("echo Hello World"))\n    print()\n    \n    print("=== Test Case 4: Invalid command with dangerous characters ===")\n    print(Task66.execute_command("ls; rm -rf /"))\n    print()\n    \n    print("=== Test Case 5: Unauthorized command ===")\n    print(Task66.execute_command("rm testfile.txt"))\n    print()\n\nif __name__ == "__main__":
    main()
