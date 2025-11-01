import java.lang.System;

public class Lse124 {

    /**
     * Processes a single string argument: prints it, trims trailing whitespace, and prints it again.
     * @param input The string to process.
     */
    public static void processArgument(String input) {
        if (input == null) {
            System.err.println("Error: Input to processArgument is null.");
            return;
        }
        // The third line prints the argument.
        System.out.println("Original: '" + input + "'");

        // The fourth line trims the trailing whitespace from the argument.
        // Using a regular expression to remove only trailing whitespace.
        String trimmedInput = input.replaceAll("\\s+$", "");

        // The fifth line prints the argument again.
        System.out.println("Trimmed:  '" + trimmedInput + "'");
        System.out.println("--------------------");
    }

    /**
     * Runs a set of 5 test cases to demonstrate the functionality of processArgument.
     */
    public static void runTests() {
        System.out.println("Running test cases...");
        System.out.println("--------------------");
        processArgument("This is a test.   ");
        processArgument("No trailing space.");
        processArgument("   Leading and trailing spaces.   ");
        processArgument(" \t\n"); // All whitespace
        processArgument("");      // Empty string
    }

    public static void main(String[] args) {
        // The first if statement checks to make sure that the user provided exactly one argument.
        if (args.length != 1) {
            // If not it prints an error message and exits.
            System.err.println("Error: Please provide exactly one argument.");
            System.err.println("Usage: java Lse124 <argument>");
            
            // Run test cases if no arguments are provided, to satisfy the prompt's test case requirement.
            if (args.length == 0) {
                runTests();
            }

            // The sixth line exits the program.
            System.exit(1);
        }

        // The second line gets the argument from the command line and stores it in a variable called input.
        String input = args[0];

        processArgument(input);

        // The sixth line exits the program.
        System.exit(0);
    }
}