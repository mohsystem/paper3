public class Task41 {

    /**
     * Processes a string. In Java, string and array handling are memory-safe due to
     * automatic bounds checking by the JVM. Buffer overflows in the C/C++ sense
     * do not occur; instead, an IndexOutOfBoundsException is thrown.
     *
     * @param input The string to be processed.
     */
    public static void processString(String input) {
        System.out.println("Processing input: \"" + input + "\"");
        // In Java, operations on strings are inherently safe from buffer overflows.
        String processedString = "Processed: " + input.toUpperCase();
        System.out.println("Output: \"" + processedString + "\"");
        System.out.println("------------------------------------");
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: A normal, short string
        processString("Hello World");

        // Test Case 2: An empty string
        processString("");

        // Test Case 3: A string with various special characters
        processString("!@#$%^&*()_+-=[]{}|;':,./<>?");

        // Test Case 4: A very long string
        processString("This is a very long string that in a language like C could potentially cause a buffer overflow if not handled carefully. In Java, the String object will simply resize to accommodate the data.");

        // Test Case 5: A string with Unicode characters
        processString("Secure Code (安全なコード)");
    }
}