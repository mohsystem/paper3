public class Task16 {

    public static boolean solution(String str, String ending) {
        if (str == null || ending == null) {
            return false;
        }
        return str.endsWith(ending);
    }

    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: solution('abc', 'bc')");
        System.out.println("Result: " + solution("abc", "bc")); // Expected: true

        System.out.println("\nTest Case 2: solution('abc', 'd')");
        System.out.println("Result: " + solution("abc", "d")); // Expected: false

        System.out.println("\nTest Case 3: solution('samurai', 'ai')");
        System.out.println("Result: " + solution("samurai", "ai")); // Expected: true
        
        System.out.println("\nTest Case 4: solution('abc', '')");
        System.out.println("Result: " + solution("abc", "")); // Expected: true

        System.out.println("\nTest Case 5: solution('test', 'testing')");
        System.out.println("Result: " + solution("test", "testing")); // Expected: false
    }
}