public class Lse124 {

    /**
     * Processes a single command-line argument based on the prompt's logic.
     *
     * @param args The array of command-line arguments.
     */
    public static void processArgument(String[] args) {
        // 1. Check if exactly one argument is provided.
        if (args.length != 1) {
            System.err.println("Error: Please provide exactly one argument.");
            // 6. Exit the logical flow.
            return;
        }

        // 2. Get the argument.
        String input = args[0];

        // 3. Print the original argument.
        System.out.println("Original: '" + input + "'");

        // 4. Trim the trailing whitespace from the argument.
        // Using regex \\s+$ to match one or more whitespace characters at the end of the string.
        String trimmedInput = input.replaceAll("\\s+$", "");

        // 5. Print the argument again.
        System.out.println("Trimmed: '" + trimmedInput + "'");
        
        // 6. The method ends, effectively exiting this logical block.
    }

    /**
     * Main method with 5 test cases.
     */
    public static void main(String[] args) {
        System.out.println("--- Test Case 1: No arguments ---");
        processArgument(new String[]{});

        System.out.println("\n--- Test Case 2: Two arguments ---");
        processArgument(new String[]{"first", "second"});

        System.out.println("\n--- Test Case 3: One argument, no trailing space ---");
        processArgument(new String[]{"hello"});

        System.out.println("\n--- Test Case 4: One argument with trailing spaces ---");
        processArgument(new String[]{"world  "});

        System.out.println("\n--- Test Case 5: One argument with leading and trailing spaces ---");
        processArgument(new String[]{"  spaced out   "});
    }
}