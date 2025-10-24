
public class Task16 {
    public static boolean solution(String str, String ending) {
        // Input validation to prevent null pointer exceptions
        if (str == null || ending == null) {
            return false;
        }
        
        // Use built-in endsWith method which is secure and efficient
        return str.endsWith(ending);
    }
    
    public static void main(String[] args) {
        // Test case 1: Basic positive case
        System.out.println(solution("abc", "bc")); // true
        
        // Test case 2: Basic negative case
        System.out.println(solution("abc", "d")); // false
        
        // Test case 3: Empty ending (edge case)
        System.out.println(solution("abc", "")); // true
        
        // Test case 4: Ending longer than string
        System.out.println(solution("ab", "abc")); // false
        
        // Test case 5: Null safety test
        System.out.println(solution(null, "bc")); // false
    }
}
