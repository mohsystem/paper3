public class Task16 {
    public static boolean solution(String str, String ending) {
        if (str == null || ending == null) return false;
        int lenS = str.length();
        int lenE = ending.length();
        if (lenE > lenS) return false;
        return str.regionMatches(lenS - lenE, ending, 0, lenE);
    }

    public static void main(String[] args) {
        String[][] tests = {
            {"abc", "bc"},
            {"abc", "d"},
            {"", ""},
            {"abc", ""},
            {"a", "ab"}
        };
        for (String[] t : tests) {
            System.out.println(solution(t[0], t[1]) ? "true" : "false");
        }
    }
}