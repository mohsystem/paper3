import java.nio.charset.StandardCharsets;
import java.util.Scanner;

public class Task41 {

    /**
     * Processes a string by creating a safe copy of it.
     * In Java, String objects are immutable and memory is managed automatically,
     * which inherently protects against classic buffer overflow vulnerabilities.
     *
     * @param input The string to be processed.
     * @return A new String object that is a copy of the input.
     */
    public static String processString(String input) {
        if (input == null) {
            return "";
        }
        // Creating a new string from the input string is a safe operation.
        // This simulates copying to a new "buffer" without any overflow risk.
        return new String(input.getBytes(StandardCharsets.UTF_8), StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        String[] testCases = {
            "Hello, World!",
            "",
            "This is a slightly longer string to test processing.",
            "A string with special characters: !@#$%^&*()_+",
            "A very long string to ensure that there are no issues with larger inputs. " +
            "Java's String and StringBuilder/StringBuffer classes handle dynamic resizing, " +
            "so buffer overflows of the C/C++ variety are not a concern with standard string operations."
        };

        System.out.println("--- Running Test Cases ---");
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("\nTest Case " + (i + 1) + ":");
            String input = testCases[i];
            System.out.println("Original:  \"" + input + "\"");
            String processed = processString(input);
            System.out.println("Processed: \"" + processed + "\"");
        }

        // Example with user input
        // Using try-with-resources to ensure the scanner is closed
        try (Scanner scanner = new Scanner(System.in)) {
            System.out.println("\n--- User Input Test ---");
            System.out.print("Enter a string to process: ");
            // Scanner.nextLine() safely reads the entire line of input
            String userInput = scanner.nextLine();
            String processedUserInput = processString(userInput);
            System.out.println("Processed user input: \"" + processedUserInput + "\"");
        }
    }
}