public final class Task141 {
    private static final int MAX_LEN = 1_000_000;

    public static String reverseString(final String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input cannot be null");
        }
        if (input.length() > MAX_LEN) {
            throw new IllegalArgumentException("Input too long");
        }
        final int[] cps = input.codePoints().toArray();
        final StringBuilder sb = new StringBuilder(cps.length);
        for (int i = cps.length - 1; i >= 0; --i) {
            sb.appendCodePoint(cps[i]);
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "",
            "a",
            "abc",
            "Hello, World!",
            "12345!@#$%"
        };
        for (String t : tests) {
            String r = reverseString(t);
            System.out.println(r);
        }
    }
}