public class Task66 {

    /**
     * Executes a bash shell command, validates its format, and returns the result.
     *
     * @param command The shell command to execute.
     * @return A string containing the standard output, standard error, and exit code.
     */
    public static String executeCommand(String command) {
        // 1. Validation: Check for null or empty/whitespace command
        if (command == null || command.trim().isEmpty()) {
            return "Validation Error: Command cannot be null or empty.";
        }

        StringBuilder output = new StringBuilder();
        Process process;
        try {
            // 2. Execute command using the shell. This is for Unix-like systems.
            ProcessBuilder builder = new ProcessBuilder("bash", "-c", command);
            process = builder.start();

            // 3. Capture Standard Output from the command
            java.io.BufferedReader stdInput = new java.io.BufferedReader(new java.io.InputStreamReader(process.getInputStream()));
            String line;
            while ((line = stdInput.readLine()) != null) {
                output.append(line).append("\n");
            }

            // 4. Capture Standard Error from the command
            java.io.BufferedReader stdError = new java.io.BufferedReader(new java.io.InputStreamReader(process.getErrorStream()));
            while ((line = stdError.readLine()) != null) {
                output.append("ERROR: ").append(line).append("\n");
            }

            // 5. Wait for the process to complete and get the exit code
            int exitCode = process.waitFor();
            output.append("Exited with code: ").append(exitCode);

        } catch (java.io.IOException | InterruptedException e) {
            output.append("Execution Error: ").append(e.getMessage());
            // Restore the interrupted status
            if (e instanceof InterruptedException) {
                Thread.currentThread().interrupt();
            }
        }

        return output.toString();
    }

    public static void main(String[] args) {
        String[] testCommands = {
            "echo 'Hello from Bash!'",            // Test Case 1: Simple echo
            "ls -l",                              // Test Case 2: List files
            "ls non_existent_directory",          // Test Case 3: Command with an error
            "echo 'one\\ntwo\\nthree' | grep 'two'", // Test Case 4: Command with a pipe
            "invalidcommand_xyz_123"              // Test Case 5: Invalid command
        };

        for (int i = 0; i < testCommands.length; i++) {
            System.out.println("--- Executing Test Case " + (i + 1) + ": `" + testCommands[i] + "` ---");
            String result = executeCommand(testCommands[i]);
            System.out.println("Result:\n" + result);
            System.out.println("--- End Test Case " + (i + 1) + " ---\n");
        }
    }
}