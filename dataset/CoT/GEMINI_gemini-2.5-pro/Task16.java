public class Task16 {

    /**
     * Checks if the first string ends with the second string.
     *
     * @param str    The main string.
     * @param ending The string to check for at the end of str.
     * @return true if str ends with ending, false otherwise.
     */
    public static boolean solution(String str, String ending) {
        if (str == null || ending == null) {
            return false;
        }
        return str.endsWith(ending);
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: solution(\"abc\", \"bc\")");
        System.out.println("Expected: true");
        System.out.println("Actual: " + solution("abc", "bc"));
        System.out.println();

        System.out.println("Test Case 2: solution(\"abc\", \"d\")");
        System.out.println("Expected: false");
        System.out.println("Actual: " + solution("abc", "d"));
        System.out.println();

        System.out.println("Test Case 3: solution(\"samurai\", \"ai\")");
        System.out.println("Expected: true");
        System.out.println("Actual: " + solution("samurai", "ai"));
        System.out.println();

        System.out.println("Test Case 4: solution(\"abc\", \"\")");
        System.out.println("Expected: true");
        System.out.println("Actual: " + solution("abc", ""));
        System.out.println();

        System.out.println("Test Case 5: solution(\"abc\", \"abcde\")");
        System.out.println("Expected: false");
        System.out.println("Actual: " + solution("abc", "abcde"));
        System.out.println();
    }
}