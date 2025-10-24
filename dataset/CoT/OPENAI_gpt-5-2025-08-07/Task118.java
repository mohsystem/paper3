// Chain-of-Through process in code generation:
// 1) Problem understanding:
//    Implement functions to process and validate IP addresses (IPv4 and IPv6). Return "IPv4", "IPv6", or "Invalid".
// 2) Security requirements:
//    - No use of fragile regex with catastrophic backtracking.
//    - Robust parsing with bounds checks, no assumptions about input well-formedness.
// 3) Secure coding generation:
//    - Manual parsers for IPv4/IPv6 with strict rules (no leading zeros in IPv4, IPv6 with at most one '::', groups 1-4 hex chars).
// 4) Code review:
//    - Handle nulls, whitespace trim, avoid exceptions, check counts and characters line-by-line.
// 5) Secure code output:
//    - Final code below with mitigations applied.

import java.util.*;

public class Task118 {
    // Public API
    public static String validateIPAddress(String ip) {
        if (ip == null) return "Invalid";
        String s = ip.trim();
        if (s.isEmpty()) return "Invalid";
        if (isValidIPv4(s)) return "IPv4";
        if (isValidIPv6(s)) return "IPv6";
        return "Invalid";
    }

    // Helpers
    private static boolean isDigit(char c) {
        return c >= '0' && c <= '9';
    }
    private static boolean isHex(char c) {
        return (c >= '0' && c <= '9')
            || (c >= 'a' && c <= 'f')
            || (c >= 'A' && c <= 'F');
    }

    private static boolean isValidIPv4(String s) {
        int n = s.length();
        int segments = 0;
        int i = 0;
        if (n == 0) return false;

        int numDigits = 0;
        int value = 0;
        boolean firstDigitZero = false;

        for (int idx = 0; idx < n; idx++) {
            char c = s.charAt(idx);
            if (c == '.') {
                // segment boundary
                if (numDigits == 0) return false; // empty segment
                segments++;
                if (segments > 3) return false; // too many segments
                // finalize checks for segment
                // leading zero not allowed if more than 1 digit
                if (numDigits > 1 && firstDigitZero) return false;
                if (value > 255) return false;
                // reset for next segment
                numDigits = 0;
                value = 0;
                firstDigitZero = false;
            } else if (isDigit(c)) {
                if (numDigits == 0) {
                    firstDigitZero = (c == '0');
                }
                if (numDigits >= 3) return false; // more than 3 digits
                value = value * 10 + (c - '0');
                numDigits++;
                if (value > 255) return false;
            } else {
                return false; // invalid char
            }
        }
        // finalize last segment
        if (numDigits == 0) return false;
        if (segments != 3) return false; // must have 3 dots -> 4 segments
        if (numDigits > 1 && firstDigitZero) return false;
        if (value > 255) return false;
        return true;
    }

    private static int parseIPv6GroupsSegment(String seg) {
        // Parses groups in a segment without any '::'. Returns number of groups or -1 if invalid.
        int n = seg.length();
        if (n == 0) return 0;
        int i = 0, count = 0;
        while (i < n) {
            int start = i;
            int len = 0;
            while (i < n && isHex(seg.charAt(i)) && len < 4) {
                i++;
                len++;
            }
            if (len == 0) return -1; // empty group
            // if next char is hex again, group too long (>4)
            if (i < n && isHex(seg.charAt(i))) return -1;
            count++;
            if (i == n) break;
            if (seg.charAt(i) != ':') return -1;
            i++; // consume ':'
            if (i == n) return -1; // trailing colon -> empty group
        }
        return count;
    }

    private static boolean isValidIPv6(String s) {
        int n = s.length();
        if (n == 0) return false;

        int dcIndex = s.indexOf("::");
        if (dcIndex >= 0) {
            if (s.indexOf("::", dcIndex + 1) != -1) return false; // more than one '::'
            String left = s.substring(0, dcIndex);
            String right = s.substring(dcIndex + 2);
            int leftCount = parseIPv6GroupsSegment(left);
            if (leftCount < 0) return false;
            int rightCount = parseIPv6GroupsSegment(right);
            if (rightCount < 0) return false;
            int total = leftCount + rightCount;
            if (total > 7) return false; // even with '::', total groups cannot exceed 7 here
            int missing = 8 - total;
            if (missing < 1) return false; // '::' must compress at least one group
            return true;
        } else {
            // No '::', must have exactly 8 groups
            // Validate with parser over full string (no empty groups allowed)
            int count = parseIPv6GroupsSegment(s);
            return count == 8;
        }
    }

    // Demo main with 5 test cases
    public static void main(String[] args) {
        String[] tests = new String[] {
            "172.16.254.1",
            "2001:0db8:85a3:0000:0000:8A2E:0370:7334",
            "256.256.256.256",
            "2001:db8::8a2e:370:7334",
            "01.1.1.1"
        };
        for (String t : tests) {
            System.out.println("Input: " + t + " -> " + validateIPAddress(t));
        }
    }
}