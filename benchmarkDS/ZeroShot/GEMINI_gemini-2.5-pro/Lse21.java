public class Lse21 {

    /**
     * Parses a string input, adds 1000 to it, and returns the result.
     * This method includes security checks for input format and potential integer overflow.
     *
     * @param input The string representation of the number.
     * @return The number after adding 1000.
     * @throws NumberFormatException if the input string is not a valid number.
     * @throws ArithmeticException if adding 1000 would result in an overflow.
     */
    public static long addThousand(String input) throws NumberFormatException, ArithmeticException {
        if (input == null) {
            throw new NumberFormatException("Error: Input string cannot be null.");
        }
        
        long number;
        try {
            number = Long.parseLong(input);
        } catch (NumberFormatException e) {
            // Re-throw with a more specific message
            throw new NumberFormatException("Error: Input is not a valid integer.");
        }

        // Secure check for potential overflow before performing addition
        if (number > Long.MAX_VALUE - 1000) {
            throw new ArithmeticException("Error: Adding 1000 would cause an overflow.");
        }
        
        return number + 1000;
    }

    /**
     * The main method reads a single command-line argument, processes it, and prints the result.
     * Includes 5 test cases in the comments demonstrating how to run the program.
     */
    public static void main(String[] args) {
        /*
         * Main method to run the program.
         * This program expects exactly one command-line argument.
         *
         * How to compile and run:
         * 1. Compile: javac Lse21.java
         * 2. Run:     java Lse21 <value>
         *
         * Test Cases:
         * 1. Normal positive value: java Lse21 50
         *    Expected output: 1050
         * 2. Normal negative value: java Lse21 -2000
         *    Expected output: -1000
         * 3. Zero value:            java Lse21 0
         *    Expected output: 1000
         * 4. Invalid input (text):  java Lse21 "hello"
         *    Expected output: Error: Input is not a valid integer.
         * 5. Overflow value:        java Lse21 9223372036854775807
         *    Expected output: Error: Adding 1000 would cause an overflow.
        */
        if (args.length != 1) {
            System.err.println("Usage: java Lse21 <integer_value>");
            return; // Exit if incorrect number of arguments
        }

        try {
            long result = addThousand(args[0]);
            System.out.println(result);
        } catch (NumberFormatException | ArithmeticException e) {
            System.err.println(e.getMessage());
        }
    }
}