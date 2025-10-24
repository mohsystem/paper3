public class Task104 {
    private static final int BUFFER_SIZE = 16;

    public static String handleInput(String input) {
        if (input == null) {
            input = "";
        }
        int cpCount = input.codePointCount(0, input.length());
        int toCopy = Math.min(cpCount, BUFFER_SIZE - 1);
        StringBuilder sb = new StringBuilder(toCopy);
        for (int i = 0, copied = 0; i < input.length() && copied < toCopy; ) {
            int cp = input.codePointAt(i);
            sb.appendCodePoint(cp);
            i += Character.charCount(cp);
            copied++;
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "Hello",
            "1234567890ABCDEF",
            "",
            "SafeBuffer!",
            "This is a very long string that exceeds the buffer size!"
        };
        for (int i = 0; i < tests.length; i++) {
            String res = handleInput(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + res);
        }
    }
}