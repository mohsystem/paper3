
import java.io.*;
import java.util.*;
import java.util.concurrent.*;

public class Task66 {
    private static final Set<String> ALLOWED_COMMANDS = new HashSet<>(Arrays.asList(
        "ls", "pwd", "date", "whoami", "echo"
    ));
    
    private static final int TIMEOUT_SECONDS = 5;
    private static final int MAX_OUTPUT_LENGTH = 10000;
    
    public static String executeCommand(String userInput) {
        if (userInput == null || userInput.trim().isEmpty()) {
            return "Error: Command cannot be empty";
        }
        
        userInput = userInput.trim();
        
        // Validate: no dangerous characters
        if (userInput.contains(";") || userInput.contains("&") || 
            userInput.contains("|") || userInput.contains(">") || 
            userInput.contains("<") || userInput.contains("`") ||
            userInput.contains("$") || userInput.contains("\\\\") ||\n            userInput.contains("\
") || userInput.contains("\\r")) {\n            return "Error: Command contains forbidden characters";\n        }\n        \n        // Parse command and arguments\n        String[] parts = userInput.split("\\\\s+");\n        String command = parts[0];\n        \n        // Whitelist validation\n        if (!ALLOWED_COMMANDS.contains(command)) {\n            return "Error: Command '" + command + "' is not in the allowed list";\n        }\n        \n        try {\n            ProcessBuilder pb = new ProcessBuilder();\n            List<String> cmdList = new ArrayList<>();\n            cmdList.addAll(Arrays.asList(parts));\n            pb.command(cmdList);\n            pb.redirectErrorStream(true);\n            \n            Process process = pb.start();\n            \n            // Execute with timeout\n            ExecutorService executor = Executors.newSingleThreadExecutor();\n            Future<String> future = executor.submit(() -> {\n                StringBuilder output = new StringBuilder();\n                try (BufferedReader reader = new BufferedReader(\n                        new InputStreamReader(process.getInputStream()))) {\n                    String line;\n                    while ((line = reader.readLine()) != null) {\n                        if (output.length() + line.length() > MAX_OUTPUT_LENGTH) {\n                            return output.toString() + "\
[Output truncated]";\n                        }\n                        output.append(line).append("\
");\n                    }\n                }\n                return output.toString();\n            });\n            \n            String result;\n            try {\n                result = future.get(TIMEOUT_SECONDS, TimeUnit.SECONDS);\n                process.waitFor(TIMEOUT_SECONDS, TimeUnit.SECONDS);\n            } catch (TimeoutException e) {\n                process.destroyForcibly();\n                future.cancel(true);\n                return "Error: Command execution timeout";\n            } finally {\n                executor.shutdownNow();\n            }\n            \n            return result.isEmpty() ? "Command executed successfully (no output)" : result;\n            \n        } catch (Exception e) {\n            return "Error: " + e.getMessage();\n        }\n    }\n    \n    public static void main(String[] args) {\n        System.out.println("=== Secure Command Executor Test Cases ===\
");\n        \n        String[] testCases = {\n            "ls",\n            "pwd",\n            "echo Hello World",\n            "ls; rm -rf /",  // Malicious attempt\n            "cat /etc/passwd"  // Unauthorized command\n        };\n        \n        for (int i = 0; i < testCases.length; i++) {\n            System.out.println("Test Case " + (i + 1) + ": " + testCases[i]);\n            System.out.println("Result: " + executeCommand(testCases[i]));\n            System.out.println("---");
        }
    }
}
