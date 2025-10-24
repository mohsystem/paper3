import java.nio.charset.StandardCharsets;

public final class Task3 {

    private static final int ALPHABET_MASK = (1 << 26) - 1;
    private static final int MAX_LEN = 5_000_000;

    public static boolean isPangram(final String input) {
        if (input == null) {
            return false;
        }
        if (input.length() > MAX_LEN) {
            return false;
        }

        int mask = 0;
        for (int i = 0; i < input.length(); i++) {
            char ch = input.charAt(i);
            if (ch >= 'A' && ch <= 'Z') {
                ch = (char) (ch - 'A' + 'a');
            }
            if (ch >= 'a' && ch <= 'z') {
                mask |= 1 << (ch - 'a');
                if (mask == ALPHABET_MASK) {
                    return true;
                }
            }
        }
        return mask == ALPHABET_MASK;
    }

    private static void runTest(final String s) {
        boolean result = isPangram(s);
        System.out.println(result);
    }

    public static void main(String[] args) {
        runTest("The quick brown fox jumps over the lazy dog");
        runTest("Sphinx of black quartz, judge my vow");
        runTest("Hello, World!");
        runTest("Pack my box with five dozen liquor jugs.");
        runTest("");
    }
}