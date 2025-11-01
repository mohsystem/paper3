import java.util.Arrays;

public class Lse79 {

    /**
     * Allocates two 1024-char buffers and copies up to 100 characters
     * from the input strings into them.
     * @param input1 The first string to copy.
     * @param input2 The second string to copy.
     */
    public static void processStrings(String input1, String input2) {
        // Allocate first 1024-byte buffer
        char[] buf1 = new char[1024];
        // Copy up to 100 characters from input1 into buf1
        if (input1 != null) {
            int lengthToCopy = Math.min(input1.length(), 100);
            input1.getChars(0, lengthToCopy, buf1, 0);
        }

        // Allocate second 1024-byte buffer
        char[] buf2 = new char[1024];
        // Copy up to 100 characters from input2 into buf2
        if (input2 != null) {
            int lengthToCopy = Math.min(input2.length(), 100);
            input2.getChars(0, lengthToCopy, buf2, 0);
        }

        // Find the actual length of the content in the buffer for printing
        int len1 = 0;
        while (len1 < buf1.length && buf1[len1] != '\0') {
            len1++;
        }
        String result1 = new String(buf1, 0, len1);

        int len2 = 0;
        while (len2 < buf2.length && buf2[len2] != '\0') {
            len2++;
        }
        String result2 = new String(buf2, 0, len2);

        System.out.println("Buffer 1 content: \"" + result1 + "\"");
        System.out.println("Buffer 2 content: \"" + result2 + "\"");
    }

    public static void main(String[] args) {
        String[][] testCases = {
            {"Hello World", "This is a test"},
            {"A short string", "A very very very very very very very very very very very very very very very very very very very long string that will be truncated."},
            {"", "An empty string for the first input."},
            {"Another test case with numbers 12345.", ""},
            {"Last test case! Using special characters: !@#$%^&*()", "And another one."}
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            System.out.println("Input 1: \"" + testCases[i][0] + "\"");
            System.out.println("Input 2: \"" + testCases[i][1] + "\"");
            processStrings(testCases[i][0], testCases[i][1]);
            System.out.println("---------------------\n");
        }
    }
}