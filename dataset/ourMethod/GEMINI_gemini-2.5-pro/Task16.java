import java.util.Objects;

public class Task16 {

    /**
     * Checks if the first string ends with the second string.
     *
     * @param str The main string.
     * @param ending The potential ending string.
     * @return true if str ends with ending, false otherwise.
     */
    public static boolean solution(String str, String ending) {
        if (str == null || ending == null) {
            return false;
        }
        return str.endsWith(ending);
    }

    public static void main(String[] args) {
        // Test Case 1: Standard true case
        System.out.println("Test 1: solution('abc', 'bc') -> " + solution("abc", "bc")); // Expected: true

        // Test Case 2: Standard false case
        System.out.println("Test 2: solution('abc', 'd') -> " + solution("abc", "d")); // Expected: false
        
        // Test Case 3: Ending is the same as the string
        System.out.println("Test 3: solution('samantha', 'samantha') -> " + solution("samantha", "samantha")); // Expected: true
        
        // Test Case 4: Ending is an empty string
        System.out.println("Test 4: solution('abc', '') -> " + solution("abc", "")); // Expected: true
        
        // Test Case 5: Main string is shorter than the ending
        System.out.println("Test 5: solution('a', 'abc') -> " + solution("a", "abc")); // Expected: false
    }
}