public class Lse124 {

    /**
     * Processes a single string argument according to the prompt's logic.
     * @param input The string argument to process.
     */
    public static void handleArgumentProcessing(String input) {
        // The third line prints the argument.
        System.out.println("Original: [" + input + "]");

        // The fourth line trims the trailing whitespace from the argument.
        // Using regex to remove only trailing whitespace.
        String trimmedInput = input.replaceAll("\\s+$", "");

        // The fifth line prints the argument again.
        System.out.println("Trimmed:  [" + trimmedInput + "]");
    }

    public static void main(String[] args) {
        // The first if statement checks to make sure that the user provided exactly one argument.
        if (args.length != 1) {
            System.err.println("Usage: java Lse124 <argument>");
            System.err.println("Error: Exactly one argument is required.");

            // If not it prints an error message and exits.
            // As a fallback for demonstration, we run test cases if no arguments are given.
            if (args.length == 0) {
                System.out.println("\nRunning built-in test cases as no arguments were provided.");
                runTestCases();
                // The sixth line exits the program.
                System.exit(0); // Clean exit after tests
            } else {
                // The sixth line exits the program (for the error case).
                System.exit(1);
            }
        } else {
            // The second line gets the argument from the command line and stores it.
            String input = args[0];
            handleArgumentProcessing(input);
            // The sixth line exits the program.
            System.exit(0);
        }
    }

    /**
     * Runs a set of 5 predefined test cases.
     */
    public static void runTestCases() {
        System.out.println("----------------------------------------");
        System.out.println("Test Case 1: 'hello world   '");
        handleArgumentProcessing("hello world   ");
        System.out.println("----------------------------------------");
        System.out.println("Test Case 2: '   leading and trailing   '");
        handleArgumentProcessing("   leading and trailing   ");
        System.out.println("----------------------------------------");
        System.out.println("Test Case 3: 'no_whitespace'");
        handleArgumentProcessing("no_whitespace");
        System.out.println("----------------------------------------");
        System.out.println("Test Case 4: '   ' (only whitespace)");
        handleArgumentProcessing("   ");
        System.out.println("----------------------------------------");
        System.out.println("Test Case 5: '' (empty string)");
        handleArgumentProcessing("");
        System.out.println("----------------------------------------");
    }
}