import java.util.*;

public class Task8 {

    public static char findMissingLetter(char[] arr) {
        if (arr == null || arr.length < 2) {
            throw new IllegalArgumentException("Input array must have at least 2 characters.");
        }
        boolean isLower = Character.isLowerCase(arr[0]);
        boolean isUpper = Character.isUpperCase(arr[0]);
        if (!isLower && !isUpper) {
            throw new IllegalArgumentException("Input must contain alphabetic characters.");
        }
        for (int i = 1; i < arr.length; i++) {
            if (!Character.isLetter(arr[i])) {
                throw new IllegalArgumentException("Input must contain only letters.");
            }
            if ((isLower && !Character.isLowerCase(arr[i])) || (isUpper && !Character.isUpperCase(arr[i]))) {
                throw new IllegalArgumentException("All letters must be in the same case.");
            }
        }
        int expected = arr[0] + 1;
        for (int i = 1; i < arr.length; i++) {
            if (arr[i] != expected) {
                return (char) expected;
            }
            expected++;
        }
        throw new IllegalArgumentException("No missing letter found.");
    }

    public static void main(String[] args) {
        char[][] tests = new char[][] {
            new char[] {'a','b','c','d','f'}, // -> 'e'
            new char[] {'O','Q','R','S'},     // -> 'P'
            new char[] {'m','n','p','q'},     // -> 'o'
            new char[] {'A','B','C','E'},     // -> 'D'
            new char[] {'t','v'}              // -> 'u'
        };
        for (char[] t : tests) {
            char missing = findMissingLetter(t);
            System.out.println("Missing: '" + missing + "'");
        }
    }
}