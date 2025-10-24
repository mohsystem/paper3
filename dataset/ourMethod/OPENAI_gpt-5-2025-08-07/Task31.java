import java.util.Arrays;

public class Task31 {

    private static final int MAX_LEN = 10_000_000;

    public static String dnaComplement(String input) {
        if (input == null) {
            throw new IllegalArgumentException("Input DNA must not be null.");
        }
        int n = input.length();
        if (n == 0) {
            throw new IllegalArgumentException("Input DNA must be non-empty.");
        }
        if (n > MAX_LEN) {
            throw new IllegalArgumentException("Input DNA length exceeds maximum allowed.");
        }

        StringBuilder sb = new StringBuilder(n);
        for (int i = 0; i < n; i++) {
            char c = input.charAt(i);
            char u = Character.toUpperCase(c);
            switch (u) {
                case 'A':
                    sb.append('T');
                    break;
                case 'T':
                    sb.append('A');
                    break;
                case 'C':
                    sb.append('G');
                    break;
                case 'G':
                    sb.append('C');
                    break;
                default:
                    throw new IllegalArgumentException("Invalid DNA base at position " + i + ": '" + c + "'");
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "ATTGC",
            "GTAT",
            "AAAA",
            "CGCGTTAA",
            "acgtACGT"
        };
        for (String t : tests) {
            try {
                String result = dnaComplement(t);
                System.out.println("Input: " + t + " => Output: " + result);
            } catch (IllegalArgumentException ex) {
                System.out.println("Input: " + t + " => Error: " + ex.getMessage());
            }
        }
    }
}