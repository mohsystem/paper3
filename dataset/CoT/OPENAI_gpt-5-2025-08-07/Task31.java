// Java implementation
// Chain-of-Through process (high-level):
// 1) Problem understanding: compute complementary DNA strand.
// 2) Security requirements: validate input, handle null and invalid chars.
// 3) Secure coding generation: strict switch mapping, no unsafe ops.
// 4) Code review: ensure no unchecked states, use StringBuilder.
// 5) Secure code output: throw clear exceptions on invalid input.

import java.util.*;

public class Task31 {
    public static String dnaComplement(String dna) {
        if (dna == null) {
            throw new IllegalArgumentException("Input DNA string is null");
        }
        StringBuilder sb = new StringBuilder(dna.length());
        for (int i = 0; i < dna.length(); i++) {
            char c = dna.charAt(i);
            switch (c) {
                case 'A': case 'a': sb.append('T'); break;
                case 'T': case 't': sb.append('A'); break;
                case 'C': case 'c': sb.append('G'); break;
                case 'G': case 'g': sb.append('C'); break;
                default:
                    throw new IllegalArgumentException("Invalid DNA character at index " + i + ": " + c);
            }
        }
        return sb.toString();
    }

    // 5 test cases
    public static void main(String[] args) {
        String[] tests = {
            "ATTGC",
            "GTAT",
            "A",
            "GATTACA",
            "CCGGTTAA"
        };
        for (String t : tests) {
            try {
                System.out.println(t + " -> " + dnaComplement(t));
            } catch (IllegalArgumentException ex) {
                System.out.println("Error for \"" + t + "\": " + ex.getMessage());
            }
        }
    }
}