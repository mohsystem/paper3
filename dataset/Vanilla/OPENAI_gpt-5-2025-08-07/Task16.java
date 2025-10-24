public class Task16 {
    public static boolean solution(String str, String ending) {
        if (str == null || ending == null) return false;
        return str.endsWith(ending);
    }

    public static void main(String[] args) {
        String[][] tests = {
            {"abc", "bc"},
            {"abc", "d"},
            {"hello", ""},
            {"", ""},
            {"JavaTask16", "Task16"}
        };
        for (String[] t : tests) {
            boolean res = solution(t[0], t[1]);
            System.out.println("solution(\"" + t[0] + "\", \"" + t[1] + "\") -> " + res);
        }
    }
}