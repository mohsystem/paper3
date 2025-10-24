public class Task60 {
    public static String expandedForm(long n) {
        if (n <= 0) return "0";
        String s = Long.toString(n);
        StringBuilder sb = new StringBuilder();
        boolean first = true;
        int len = s.length();
        for (int i = 0; i < len; i++) {
            char c = s.charAt(i);
            if (c != '0') {
                if (!first) sb.append(" + ");
                sb.append(c);
                for (int j = i + 1; j < len; j++) sb.append('0');
                first = false;
            }
        }
        if (first) return "0";
        return sb.toString();
    }

    public static void main(String[] args) {
        long[] tests = {12, 42, 70304, 9000000, 102030};
        for (long t : tests) {
            System.out.println(t + " -> " + expandedForm(t));
        }
    }
}