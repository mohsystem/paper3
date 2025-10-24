/*
Chain-of-Through process:
1) Problem understanding:
   - Read a line from console, attempt to convert to a 32-bit integer.
   - Provide a function that accepts a string parameter and returns a structured result.
2) Security requirements:
   - Validate input length to avoid excessive memory usage.
   - Trim whitespace and validate characters to prevent unexpected parsing behavior.
   - Handle overflow/underflow and malformed input safely without throwing unhandled exceptions.
3) Secure coding generation:
   - Implement tryParseInt with strict validation and error handling.
   - Avoid closing System.in inadvertently; do not use insecure APIs.
4) Code review:
   - Checked for null handling, bounds checks, and robust parsing.
   - Verified no resource leaks or unhandled exceptions in main flow.
5) Secure code output:
   - The final code performs all validations, handles errors gracefully, and demonstrates 5 test cases.
*/

import java.util.Scanner;

public class Task98 {

    static final class ParseResult {
        final boolean success;
        final Integer value;
        final String error;

        ParseResult(boolean success, Integer value, String error) {
            this.success = success;
            this.value = value;
            this.error = error;
        }

        @Override
        public String toString() {
            if (success) {
                return "Success: " + value;
            }
            return "Error: " + error;
        }
    }

    // Function that accepts input as parameter and returns output
    public static ParseResult tryParseInt(String input) {
        // Validate null
        if (input == null) {
            return new ParseResult(false, null, "Input is null");
        }

        String s = input.trim();

        // Enforce reasonable length
        if (s.length() == 0) {
            return new ParseResult(false, null, "Empty input");
        }
        if (s.length() > 1000) {
            return new ParseResult(false, null, "Input too long");
        }

        // Validate characters: optional sign followed by digits
        // Prevents issues like "123abc"
        boolean hasSign = false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (i == 0 && (c == '+' || c == '-')) {
                hasSign = true;
                continue;
            }
            if (c < '0' || c > '9') {
                return new ParseResult(false, null, "Invalid characters");
            }
        }

        try {
            int value = Integer.parseInt(s);
            return new ParseResult(true, value, null);
        } catch (NumberFormatException ex) {
            return new ParseResult(false, null, "Number out of int range");
        }
    }

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in); // Do not close to avoid closing System.in
        System.out.println("Enter a value to convert to integer:");
        if (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            ParseResult res = tryParseInt(line);
            System.out.println(res.toString());
        } else {
            System.out.println("No input received.");
        }

        // 5 test cases
        String[] tests = new String[] {
            "123",
            "  -42  ",
            "2147483647",
            "2147483648",
            "abc"
        };
        System.out.println("---- Test Cases ----");
        for (String t : tests) {
            ParseResult r = tryParseInt(t);
            System.out.println("Input: \"" + t + "\" -> " + r.toString());
        }
    }
}