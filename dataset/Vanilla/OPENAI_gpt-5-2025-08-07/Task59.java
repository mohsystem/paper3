public class Task59 {
    public static String accum(String s) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (i > 0) sb.append('-');
            sb.append(Character.toUpperCase(c));
            for (int j = 0; j < i; j++) {
                sb.append(Character.toLowerCase(c));
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = {
            "abcd",
            "RqaEzty",
            "cwAt",
            "ZpglnRxqenU",
            "NyffsGeyylB"
        };
        for (String t : tests) {
            System.out.println(accum(t));
        }
    }
}