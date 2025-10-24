public class Task41 {

    /**
     * Processes a string by printing it and its length.
     * In Java, the String class and standard input methods are inherently safe
     * from buffer overflow vulnerabilities due to automatic memory management and
     * bounds checking.
     *
     * @param input The string to be processed.
     */
    public static void processString(String input) {
        System.out.println("Processed String: " + input);
        System.out.println("Length: " + input.length());
        System.out.println("--------------------");
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: Regular string
        processString("Hello, World!");

        // Test Case 2: Another typical string
        processString("This is a test string.");

        // Test Case 3: Empty string
        processString("");

        // Test Case 4: A very long string
        processString("This is a very long string to demonstrate that Java's String class handles memory dynamically, so there is no fixed buffer to overflow unlike in languages like C.");

        // Test Case 5: String with special characters
        processString("!@#$%^&*()_+-=[]{}|;:'\",.<>?`~");

        System.out.println("--- End of Test Cases ---");
    }
}