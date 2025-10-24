public class Task104 {

    public static String handleInput(String input, int bufferSize) {
        if (input == null || bufferSize <= 0) {
            return "";
        }
        if (input.length() > bufferSize) {
            return input.substring(0, bufferSize);
        }
        return input;
    }

    public static void main(String[] args) {
        final int BUFFER_SIZE = 16;
        String[] testCases = {
            "Hello World",
            "This is a long string that will be truncated",
            "1234567890123456", // Exactly 16 characters
            "",
            "Another long test for truncation"
        };

        for (String test : testCases) {
            String result = handleInput(test, BUFFER_SIZE);
            System.out.println("Input:  \"" + test + "\"");
            System.out.println("Output: \"" + result + "\"\n");
        }
    }
}