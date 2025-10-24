import java.util.*;

public class Task118 {
    public static boolean isValidIPv4(String ip) {
        if (ip == null || ip.isEmpty()) return false;
        int n = ip.length();
        int seg = 0;
        int i = 0;
        while (i < n) {
            if (seg == 4) return false;
            int len = 0;
            int num = 0;
            char c = ip.charAt(i);
            if (!Character.isDigit(c)) return false;
            if (c == '0') {
                len = 1;
                i++;
                if (i < n && Character.isDigit(ip.charAt(i))) return false;
            } else {
                while (i < n && Character.isDigit(ip.charAt(i))) {
                    if (len >= 3) return false;
                    int d = ip.charAt(i) - '0';
                    num = num * 10 + d;
                    if (num > 255) return false;
                    len++;
                    i++;
                }
                if (len == 0) return false;
            }
            seg++;
            if (seg < 4) {
                if (i >= n || ip.charAt(i) != '.') return false;
                i++;
                if (i >= n) return false;
            } else {
                if (i != n) return false;
            }
        }
        return seg == 4;
    }

    private static boolean isHexGroup(String s) {
        int len = s.length();
        if (len < 1 || len > 4) return false;
        for (int i = 0; i < len; i++) {
            char ch = s.charAt(i);
            boolean isHex = (ch >= '0' && ch <= '9') ||
                            (ch >= 'a' && ch <= 'f') ||
                            (ch >= 'A' && ch <= 'F');
            if (!isHex) return false;
        }
        return true;
    }

    public static boolean isValidIPv6(String ip) {
        if (ip == null || ip.isEmpty()) return false;
        if (ip.indexOf('.') != -1) return false; // Do not support embedded IPv4
        int first = ip.indexOf("::");
        if (first != -1) {
            int second = ip.indexOf("::", first + 2);
            if (second != -1) return false; // more than one '::'
            String left = ip.substring(0, first);
            String right = ip.substring(first + 2);
            int leftCount = 0;
            if (!left.isEmpty()) {
                String[] lparts = left.split(":", -1);
                for (String p : lparts) {
                    if (p.isEmpty() || !isHexGroup(p)) return false;
                    leftCount++;
                }
            }
            int rightCount = 0;
            if (!right.isEmpty()) {
                String[] rparts = right.split(":", -1);
                for (String p : rparts) {
                    if (p.isEmpty() || !isHexGroup(p)) return false;
                    rightCount++;
                }
            }
            int total = leftCount + rightCount;
            return total < 8; // at least one zero group compressed
        } else {
            String[] parts = ip.split(":", -1);
            if (parts.length != 8) return false;
            for (String p : parts) {
                if (p.isEmpty() || !isHexGroup(p)) return false;
            }
            return true;
        }
    }

    public static String classifyIP(String ip) {
        if (isValidIPv4(ip)) return "IPv4";
        if (isValidIPv6(ip)) return "IPv6";
        return "Invalid";
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "192.168.1.1",
            "255.256.0.1",
            "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
            "2001:db8::1",
            "01.1.1.1"
        };
        for (String t : tests) {
            System.out.println(t + " => " + classifyIP(t));
        }
    }
}