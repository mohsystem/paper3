public class Lse124 {

    /**
     * Processes a single command-line argument according to the specified logic.
     * @param args Command-line arguments. Expects exactly one argument.
     */
    public static void main(String[] args) {
        // 1) Check for exactly one argument. If not, print an error and exit.
        if (args.length != 1) {
            System.err.println("Error: Please provide exactly one argument.");
            System.exit(1);
        }

        // 2) Get the argument and store it.
        String input = args[0];

        // 3) Print the original argument.
        System.out.println("Original: '" + input + "'");

        // 4) Trim the trailing whitespace.
        // We use a regular expression to remove whitespace characters (\\s) from the end of the string ($).
        String trimmedInput = input.replaceAll("\\s+$", "");

        // 5) Print the trimmed argument.
        System.out.println("Trimmed:  '" + trimmedInput + "'");

        // 6) Exit the program.
        System.exit(0);

        /*
         * Main method with 5 test cases (to be run from the command line).
         *
         * How to compile and run:
         * javac Lse124.java
         *
         * Test Case 1: Argument with trailing spaces
         * java Lse124 "hello world   "
         * Expected Output:
         * Original: 'hello world   '
         * Trimmed:  'hello world'
         *
         * Test Case 2: Argument with leading and trailing spaces
         * java Lse124 "  leading and trailing  "
         * Expected Output:
         * Original: '  leading and trailing  '
         * Trimmed:  '  leading and trailing'
         *
         * Test Case 3: Argument with no trailing whitespace
         * java Lse124 "no-whitespace"
         * Expected Output:
         * Original: 'no-whitespace'
         * Trimmed:  'no-whitespace'
         *
         * Test Case 4: No arguments (error case)
         * java Lse124
         * Expected Output:
         * Error: Please provide exactly one argument.
         *
         * Test Case 5: Multiple arguments (error case)
         * java Lse124 "one" "two"
         * Expected Output:
         * Error: Please provide exactly one argument.
         */
    }
}