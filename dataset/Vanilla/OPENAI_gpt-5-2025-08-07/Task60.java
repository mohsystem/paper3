public class Task60 {
    public static String expandedForm(long num) {
        String s = Long.toString(num);
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            int d = s.charAt(i) - '0';
            if (d == 0) continue;
            long place = 1;
            for (int j = i + 1; j < s.length(); j++) place *= 10;
            long val = d * place;
            if (sb.length() > 0) sb.append(" + ");
            sb.append(val);
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        long[] tests = {12, 42, 70304, 9000000, 105};
        for (long t : tests) {
            System.out.println(t + " -> " + expandedForm(t));
        }
    }
}