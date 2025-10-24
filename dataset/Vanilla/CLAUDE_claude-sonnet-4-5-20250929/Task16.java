
public class Task16 {
    public static boolean solution(String str, String ending) {
        return str.endsWith(ending);
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(solution("abc", "bc")); // true
        
        // Test case 2
        System.out.println(solution("abc", "d")); // false
        
        // Test case 3
        System.out.println(solution("hello world", "world")); // true
        
        // Test case 4
        System.out.println(solution("test", "")); // true
        
        // Test case 5
        System.out.println(solution("programming", "ing")); // true
    }
}
