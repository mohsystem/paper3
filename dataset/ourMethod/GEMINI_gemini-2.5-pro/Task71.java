import java.util.Arrays;
import java.util.List;

public class Task71 {

    /**
     * Attempts to convert a string to an integer.
     *
     * @param input The string to convert.
     * @return The integer value of the string.
     * @throws NumberFormatException if the string is not a valid representation of an integer.
     */
    public static int convertStringToInt(String input) throws NumberFormatException {
        return Integer.parseInt(input);
    }

    /**
     * The main method that processes command-line arguments or runs test cases.
     *
     * @param args Command-line arguments. The first argument is used as input.
     */
    public static void main(String[] args) {
        if (args.length > 0) {
            System.out.println("Processing command line argument:");
            String input = args[0];
            try {
                int result = convertStringToInt(input);
                System.out.println("Successfully converted \"" + input + "\" to " + result);
            } catch (NumberFormatException e) {
                System.out.println("Error: Could not convert \"" + input + "\" to an integer. Invalid format or out of range.");
            }
        } else {
            System.out.println("No command line arguments provided. Running test cases:");
            List<String> testCases = Arrays.asList(
                "123",      // Valid positive integer
                "-456",     // Valid negative integer
                "0",        // Valid zero
                "abc",      // Invalid format
                "2147483648"// Out of range for standard int
            );

            for (String test : testCases) {
                try {
                    int result = convertStringToInt(test);
                    System.out.println("Input: \"" + test + "\", Output: " + result);
                } catch (NumberFormatException e) {
                    System.out.println("Input: \"" + test + "\", Error: Invalid format or out of range.");
                }
            }
        }
    }
}