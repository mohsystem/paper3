// Chain-of-Through secure implementation for email validation
// 1) Problem understanding: Validate if a given string is a syntactically valid email.
// 2) Security requirements: Avoid regex backtracking, restrict to ASCII, enforce length limits,
//    validate characters strictly, and handle null/edge cases safely.
// 3) Secure coding generation: Deterministic parsing with bounds checks and ASCII-only rules.
// 4) Code review: Ensure no unchecked indexing, no unsafe conversions, and strict conditions.
// 5) Secure code output: Final version with unit-like tests in main.

import java.util.*;

public class Task55 {
    public static boolean validateEmail(String email) {
        if (email == null) return false;
        int len = email.length();
        if (len == 0 || len > 254) return false;

        // Reject whitespace/control and non-ASCII
        for (int i = 0; i < len; i++) {
            char c = email.charAt(i);
            if (c <= 32 || c >= 127) return false; // disallow space, control, DEL, non-ASCII
        }

        int atFirst = email.indexOf('@');
        int atLast = email.lastIndexOf('@');
        if (atFirst <= 0 || atLast != atFirst || atFirst == len - 1) return false;

        String local = email.substring(0, atFirst);
        String domain = email.substring(atFirst + 1);

        // Validate local part
        if (local.length() == 0 || local.length() > 64) return false;
        if (local.charAt(0) == '.' || local.charAt(local.length() - 1) == '.') return false;

        boolean prevDot = false;
        for (int i = 0; i < local.length(); i++) {
            char c = local.charAt(i);
            if (!isLocalAllowed(c)) return false;
            if (c == '.') {
                if (prevDot) return false;
                prevDot = true;
            } else {
                prevDot = false;
            }
        }

        // Validate domain
        if (domain.length() == 0 || domain.length() > 253) return false;
        // Split by '.' while preserving empty labels to detect consecutive dots
        String[] labels = domain.split("\\.", -1);
        if (labels.length < 2) return false; // must have at least one dot

        for (String label : labels) {
            if (label.length() == 0 || label.length() > 63) return false;
            if (!isAlnumASCII(label.charAt(0)) || !isAlnumASCII(label.charAt(label.length() - 1))) return false;
            for (int i = 0; i < label.length(); i++) {
                char c = label.charAt(i);
                if (!(isAlnumASCII(c) || c == '-')) return false;
            }
        }

        String tld = labels[labels.length - 1];
        if (tld.length() < 2 || tld.length() > 63) return false;
        for (int i = 0; i < tld.length(); i++) {
            char c = tld.charAt(i);
            if (!isAlphaASCII(c)) return false;
        }

        return true;
    }

    private static boolean isAlnumASCII(char c) {
        return (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9');
    }

    private static boolean isAlphaASCII(char c) {
        return (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z');
    }

    private static boolean isLocalAllowed(char c) {
        return isAlnumASCII(c) || c == '.' || c == '_' || c == '+' || c == '-';
    }

    private static void runTest(String email) {
        System.out.println(email + " -> " + validateEmail(email));
    }

    public static void main(String[] args) {
        // 5 test cases
        runTest("user.name+tag-123@example-domain.com"); // valid
        runTest("a_b@example.co"); // valid
        runTest("user..dot@example.com"); // invalid
        runTest("user@example"); // invalid
        runTest(".startingdot@example.com"); // invalid
    }
}