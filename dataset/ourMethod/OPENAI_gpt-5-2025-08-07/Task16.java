import java.nio.charset.StandardCharsets;

public class Task16 {
    public static boolean solution(String str, String ending) {
        if (str == null || ending == null) {
            return false; // Fail closed on invalid input
        }
        // Directly use safe, built-in method
        return str.endsWith(ending);
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
            {"abc", "bc"},
            {"abc", "d"},
            {"", ""},
            {"hello", ""},
            {"你好世界", "世界"}
        };

        for (String[] t : tests) {
            String a = t[0];
            String b = t[1];
            boolean res = solution(a, b);
            System.out.printf("solution(\"%s\", \"%s\") = %b%n", a, b, res);
        }
    }
}