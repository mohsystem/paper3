import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public final class Task118 {
    private static final int MAX_INPUT_LENGTH = 1000;

    public static String detectIPType(String input) {
        if (input == null) return "Invalid";
        if (!isLengthSafe(input)) return "Invalid";
        if (!isAscii(input)) return "Invalid";
        if (isValidIPv4(input)) return "IPv4";
        if (isValidIPv6(input)) return "IPv6";
        return "Invalid";
    }

    public static boolean isValidIPv4(String s) {
        if (!isLengthSafe(s) || s.indexOf(' ') >= 0 || s.indexOf('\t') >= 0) return false;
        String[] parts = s.split("\\.", -1);
        if (parts.length != 4) return false;
        for (String p : parts) {
            if (p.length() == 0 || p.length() > 3) return false;
            for (int i = 0; i < p.length(); i++) {
                char c = p.charAt(i);
                if (c < '0' || c > '9') return false;
            }
            if (p.length() > 1 && p.charAt(0) == '0') return false;
            try {
                int v = Integer.parseInt(p);
                if (v < 0 || v > 255) return false;
            } catch (NumberFormatException e) {
                return false;
            }
        }
        return true;
    }

    public static String normalizeIPv4(String s) {
        if (!isValidIPv4(s)) return "";
        String[] parts = s.split("\\.", -1);
        StringBuilder sb = new StringBuilder(15);
        for (int i = 0; i < 4; i++) {
            int v = Integer.parseInt(parts[i]);
            sb.append(v);
            if (i < 3) sb.append('.');
        }
        return sb.toString();
    }

    public static boolean isValidIPv6(String s) {
        return parseIPv6ToGroups(s) != null;
    }

    public static String normalizeIPv6(String s) {
        int[] groups = parseIPv6ToGroups(s);
        if (groups == null) return "";
        StringBuilder sb = new StringBuilder(39);
        for (int i = 0; i < 8; i++) {
            String hex = Integer.toHexString(groups[i]);
            for (int k = hex.length(); k < 4; k++) sb.append('0');
            sb.append(hex);
            if (i < 7) sb.append(':');
        }
        return sb.toString();
    }

    private static int[] parseIPv6ToGroups(String s) {
        if (s == null || !isLengthSafe(s) || !isAscii(s)) return null;
        if (s.isEmpty()) return null;
        // Only hex digits and ':' allowed
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = (c == ':') || (c >= '0' && c <= '9') ||
                         (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
            if (!ok) return null;
        }

        int dcFirst = s.indexOf("::");
        if (dcFirst != -1 && s.indexOf("::", dcFirst + 2) != -1) return null; // more than one '::'

        List<Integer> groups = new ArrayList<>(8);
        if (dcFirst >= 0) {
            String left = s.substring(0, dcFirst);
            String right = s.substring(dcFirst + 2);

            List<Integer> leftGroups = parseIPv6Side(left);
            if (leftGroups == null) return null;
            List<Integer> rightGroups = parseIPv6Side(right);
            if (rightGroups == null) return null;

            int total = leftGroups.size() + rightGroups.size();
            if (total >= 8) return null;
            groups.addAll(leftGroups);
            for (int i = 0; i < 8 - total; i++) groups.add(0);
            groups.addAll(rightGroups);
        } else {
            List<Integer> all = parseIPv6Side(s);
            if (all == null || all.size() != 8) return null;
            groups.addAll(all);
        }
        if (groups.size() != 8) return null;
        int[] out = new int[8];
        for (int i = 0; i < 8; i++) out[i] = groups.get(i);
        return out;
    }

    private static List<Integer> parseIPv6Side(String side) {
        List<Integer> res = new ArrayList<>(8);
        if (side.isEmpty()) return res; // zero groups
        String[] tokens = side.split(":", -1);
        for (String t : tokens) {
            if (t.isEmpty() || t.length() > 4) return null;
            int val = 0;
            for (int i = 0; i < t.length(); i++) {
                char c = t.charAt(i);
                int v;
                if (c >= '0' && c <= '9') v = c - '0';
                else if (c >= 'a' && c <= 'f') v = 10 + (c - 'a');
                else if (c >= 'A' && c <= 'F') v = 10 + (c - 'A');
                else return null;
                val = (val << 4) | v;
                if (val < 0 || val > 0xFFFF) return null;
            }
            res.add(val);
            if (res.size() > 8) return null;
        }
        return res;
    }

    private static boolean isLengthSafe(String s) {
        // Validate reasonable bounds
        return s.length() > 0 && s.length() <= MAX_INPUT_LENGTH;
    }

    private static boolean isAscii(String s) {
        byte[] bytes = s.getBytes(StandardCharsets.US_ASCII);
        return new String(bytes, StandardCharsets.US_ASCII).equals(s);
    }

    public static String processIP(String s) {
        String type = detectIPType(s);
        if ("IPv4".equals(type)) {
            return "Type: IPv4, Normalized: " + normalizeIPv4(s);
        } else if ("IPv6".equals(type)) {
            return "Type: IPv6, Normalized: " + normalizeIPv6(s);
        } else {
            return "Invalid";
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "192.168.1.1",
            "255.255.255.255",
            "01.2.3.4",
            "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
            "2001:db8::8a2e:370:7334"
        };
        for (String t : tests) {
            System.out.println(t + " => " + processIP(t));
        }
    }
}