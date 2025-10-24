public class Task16 {

    /**
     * Checks if the first string ends with the second string.
     * This implementation is secure as it uses the built-in String.endsWith method,
     * which handles all edge cases such as different lengths and empty strings.
     * A null check is added to prevent NullPointerException.
     *
     * @param str The main string.
     * @param ending The string to check for at the end of str.
     * @return true if str ends with ending, false otherwise.
     */
    public static boolean solution(String str, String ending) {
        // Handle null inputs to prevent NullPointerException
        if (str == null || ending == null) {
            return false;
        }
        return str.endsWith(ending);
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("('abc', 'bc') -> " + solution("abc", "bc")); // true
        System.out.println("('abc', 'd') -> " + solution("abc", "d")); // false
        System.out.println("('sushi', 'shi') -> " + solution("sushi", "shi")); // true
        System.out.println("('samurai', 'ra') -> " + solution("samurai", "ra")); // false
        System.out.println("('abc', '') -> " + solution("abc", "")); // true
    }
}