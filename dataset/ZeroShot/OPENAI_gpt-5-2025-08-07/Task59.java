public class Task59 {
    public static String accum(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (i > 0) sb.append('-');
            sb.append(Character.toUpperCase(c));
            char lower = Character.toLowerCase(c);
            for (int k = 0; k < i; k++) sb.append(lower);
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {"abcd", "RqaEzty", "cwAt", "", "Z"};
        for (String t : tests) {
            System.out.println(accum(t));
        }
    }
}