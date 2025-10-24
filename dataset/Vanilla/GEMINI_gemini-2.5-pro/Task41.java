public class Task41 {

    /**
     * Processes the input string. In Java, String objects and the JVM's memory
     * management are inherently safe from classic buffer overflow vulnerabilities.
     * Accessing characters out of bounds would throw an IndexOutOfBoundsException
     * rather than corrupting memory.
     *
     * @param input The string to be processed.
     */
    public static void processString(String input) {
        System.out.println("Processing input: \"" + input + "\"");
        System.out.println("Length: " + input.length());
        // In a real application, further safe processing would occur here.
        System.out.println("Processing complete.");
        System.out.println("----------------------------------------");
    }

    public static void main(String[] args) {
        System.out.println("--- Java Test Cases (Buffer Overflow Safe by Design) ---");

        // Test Case 1: A typical short string
        processString("Hello, World!");

        // Test Case 2: An empty string
        processString("");

        // Test Case 3: A long string that would overflow a small fixed buffer in C/C++
        processString("This is a very long string designed to test the dynamic memory allocation of Java's String class, which prevents buffer overflows automatically.");

        // Test Case 4: A string with special characters and unicode
        processString("!@#$%^&*()_+-={}[]|;'<>,.?/`~ \u20AC");

        // Test Case 5: A string with internal null characters (valid in Java)
        processString("A string with a\0null character.");
    }
}