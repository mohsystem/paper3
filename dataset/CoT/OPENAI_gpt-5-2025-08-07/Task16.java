// Chain-of-Through process:
// 1) Problem understanding: Return true if first string ends with the second string.
// 2) Security requirements: Handle null inputs safely, avoid exceptions.
// 3) Secure coding generation: Validate inputs, use safe standard method.
// 4) Code review: Checked for null handling and correct logic.
// 5) Secure code output: Finalized with test cases.
public class Task16 {
    public static boolean solution(String str, String ending) {
        if (str == null || ending == null) {
            return false;
        }
        return str.endsWith(ending);
    }

    public static void main(String[] args) {
        String[][] tests = {
            {"abc", "bc"},
            {"abc", "d"},
            {"hello", ""},
            {"", ""},
            {"abc", "abc"}
        };
        for (String[] t : tests) {
            System.out.println(solution(t[0], t[1]));
        }
    }
}