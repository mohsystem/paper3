// Chain-of-Through secure implementation for a simple Caesar cipher.
// 1) Problem understanding: Implement a function that encrypts a given string using a Caesar cipher.
// 2) Security requirements: Handle null inputs, avoid undefined behavior, and keep operations within safe bounds.
// 3) Secure coding generation: Normalize keys to avoid negative modulo issues; do not modify input; use immutable returns.
// 4) Code review: Character handling limited to A-Z and a-z; others remain unchanged.
// 5) Secure code output: Final code after review.
public class Task51 {
    private static int normalizeKey(int key) {
        int k = key % 26;
        if (k < 0) k += 26;
        return k;
    }

    public static String encrypt(String input, int key) {
        if (input == null) {
            return "";
        }
        int k = normalizeKey(key);
        StringBuilder sb = new StringBuilder(input.length());
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            if (c >= 'A' && c <= 'Z') {
                int offset = c - 'A';
                char out = (char) ('A' + (offset + k) % 26);
                sb.append(out);
            } else if (c >= 'a' && c <= 'z') {
                int offset = c - 'a';
                char out = (char) ('a' + (offset + k) % 26);
                sb.append(out);
            } else {
                sb.append(c);
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = {
            "Hello, World!",
            "abcxyz ABCXYZ",
            "Attack at dawn! 123",
            "",
            "Zebra-123"
        };
        int[] keys = {3, 2, 5, 10, -1};
        for (int i = 0; i < tests.length; i++) {
            String enc = encrypt(tests[i], keys[i]);
            System.out.println("Input: " + tests[i] + " | Key: " + keys[i] + " | Encrypted: " + enc);
        }
    }
}