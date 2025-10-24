
public class Task16 {
    public static boolean solution(String str, String ending) {
        if (str == null || ending == null) {
            return false;
        }
        return str.endsWith(ending);
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println(solution("abc", "bc"));      // true
        System.out.println(solution("abc", "d"));       // false
        System.out.println(solution("hello", "lo"));    // true
        System.out.println(solution("test", ""));       // true
        System.out.println(solution("", "test"));       // false
    }
}
