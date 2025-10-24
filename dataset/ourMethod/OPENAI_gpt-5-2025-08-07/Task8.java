import java.util.Arrays;

public class Task8 {
    public static char findMissingLetter(final char[] arr) {
        if (arr == null || arr.length < 2) {
            throw new IllegalArgumentException("Invalid input: array must have at least 2 letters.");
        }

        char first = arr[0];
        boolean isUpper;
        if (first >= 'A' && first <= 'Z') {
            isUpper = true;
        } else if (first >= 'a' && first <= 'z') {
            isUpper = false;
        } else {
            throw new IllegalArgumentException("Invalid input: elements must be letters A-Z or a-z.");
        }

        for (int i = 0; i < arr.length; i++) {
            char c = arr[i];
            if (isUpper) {
                if (c < 'A' || c > 'Z') {
                    throw new IllegalArgumentException("Invalid input: mixed case or non-letter detected.");
                }
            } else {
                if (c < 'a' || c > 'z') {
                    throw new IllegalArgumentException("Invalid input: mixed case or non-letter detected.");
                }
            }
            if (i > 0) {
                if (c <= arr[i - 1]) {
                    throw new IllegalArgumentException("Invalid input: letters must be strictly increasing.");
                }
                int diff = c - arr[i - 1];
                if (diff > 1) {
                    return (char) (arr[i - 1] + 1);
                }
            }
        }

        char candidate = (char) (arr[0] + arr.length);
        if (isUpper) {
            if (candidate < 'A' || candidate > 'Z') {
                throw new IllegalArgumentException("Missing letter out of range.");
            }
        } else {
            if (candidate < 'a' || candidate > 'z') {
                throw new IllegalArgumentException("Missing letter out of range.");
            }
        }
        return candidate;
    }

    public static void main(String[] args) {
        char[][] tests = new char[][]{
            new char[]{'a', 'b', 'c', 'd', 'f'},
            new char[]{'O', 'Q', 'R', 'S'},
            new char[]{'g', 'h', 'j', 'k'},
            new char[]{'A', 'B', 'C', 'E'},
            new char[]{'t', 'u', 'v', 'x'}
        };

        for (int i = 0; i < tests.length; i++) {
            char result = findMissingLetter(tests[i]);
            System.out.println("Test " + (i + 1) + ": " + result);
        }
    }
}