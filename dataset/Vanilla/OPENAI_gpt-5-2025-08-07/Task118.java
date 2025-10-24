import java.util.*;

public class Task118 {

    public static boolean isValidIPv4(String ip) {
        if (ip == null) return false;
        ip = ip.trim();
        if (ip.length() == 0) return false;
        if (ip.startsWith(".") || ip.endsWith(".")) return false;
        String[] parts = ip.split("\\.", -1);
        if (parts.length != 4) return false;
        for (String p : parts) {
            if (p.length() == 0 || p.length() > 3) return false;
            for (int i = 0; i < p.length(); i++) {
                if (!Character.isDigit(p.charAt(i))) return false;
            }
            if (p.length() > 1 && p.charAt(0) == '0') return false;
            try {
                int val = Integer.parseInt(p);
                if (val < 0 || val > 255) return false;
            } catch (NumberFormatException e) {
                return false;
            }
        }
        return true;
    }

    private static boolean isHexGroup(String s) {
        if (s.length() < 1 || s.length() > 4) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean isHex = (c >= '0' && c <= '9')
                    || (c >= 'a' && c <= 'f')
                    || (c >= 'A' && c <= 'F');
            if (!isHex) return false;
        }
        return true;
    }

    public static boolean isValidIPv6(String ip) {
        if (ip == null) return false;
        ip = ip.trim();
        if (ip.length() == 0) return false;
        if (ip.indexOf('.') != -1) return false; // do not support embedded IPv4
        int firstDC = ip.indexOf("::");
        if (firstDC != -1) {
            int secondDC = ip.indexOf("::", firstDC + 2);
            if (secondDC != -1) return false; // more than one '::'
            String left = ip.substring(0, firstDC);
            String right = ip.substring(firstDC + 2);
            String[] leftParts = left.isEmpty() ? new String[0] : left.split(":", -1);
            String[] rightParts = right.isEmpty() ? new String[0] : right.split(":", -1);
            // No empty parts allowed on either side when using split with -1
            for (String p : leftParts) if (p.length() == 0 || !isHexGroup(p)) return false;
            for (String p : rightParts) if (p.length() == 0 || !isHexGroup(p)) return false;
            int groups = leftParts.length + rightParts.length;
            if (groups > 7) return false; // must leave room for at least one zero group
            return true;
        } else {
            String[] parts = ip.split(":", -1);
            if (parts.length != 8) return false;
            for (String p : parts) {
                if (!isHexGroup(p)) return false;
            }
            return true;
        }
    }

    public static String validateIPAddress(String ip) {
        if (isValidIPv4(ip)) return "IPv4";
        if (isValidIPv6(ip)) return "IPv6";
        return "Neither";
    }

    public static String normalizeIPv4(String ip) {
        if (!isValidIPv4(ip)) return null;
        String[] parts = ip.trim().split("\\.", -1);
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 4; i++) {
            int val = Integer.parseInt(parts[i]);
            if (i > 0) sb.append('.');
            sb.append(val);
        }
        return sb.toString();
    }

    private static String pad4Upper(String hex) {
        String h = hex.toUpperCase(Locale.ROOT);
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 4 - h.length(); i++) sb.append('0');
        sb.append(h);
        return sb.toString();
    }

    public static String normalizeIPv6(String ip) {
        if (!isValidIPv6(ip)) return null;
        ip = ip.trim();
        int dc = ip.indexOf("::");
        List<String> groups = new ArrayList<>(8);
        if (dc != -1) {
            String left = ip.substring(0, dc);
            String right = ip.substring(dc + 2);
            String[] leftParts = left.isEmpty() ? new String[0] : left.split(":", -1);
            String[] rightParts = right.isEmpty() ? new String[0] : right.split(":", -1);
            for (String p : leftParts) groups.add(p);
            int zerosToInsert = 8 - (leftParts.length + rightParts.length);
            for (int i = 0; i < zerosToInsert; i++) groups.add("0");
            for (String p : rightParts) groups.add(p);
        } else {
            groups.addAll(Arrays.asList(ip.split(":", -1)));
        }
        if (groups.size() != 8) return null; // safety
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 8; i++) {
            if (i > 0) sb.append(':');
            sb.append(pad4Upper(groups.get(i)));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "172.16.254.1",
            "2001:0db8:85a3:0000:0000:8A2E:0370:7334",
            "2001:db8:85a3::8a2e:370:7334",
            "256.256.256.256",
            "01.1.1.1"
        };
        for (String ip : tests) {
            String kind = validateIPAddress(ip);
            System.out.println("IP: " + ip + " -> " + kind);
            if (kind.equals("IPv4")) {
                System.out.println("Normalized IPv4: " + normalizeIPv4(ip));
            } else if (kind.equals("IPv6")) {
                System.out.println("Normalized IPv6: " + normalizeIPv6(ip));
            }
            System.out.println();
        }
    }
}