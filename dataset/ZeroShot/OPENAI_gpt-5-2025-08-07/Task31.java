import java.util.*;

public class Task31 {
    public static String dnaComplement(String dna) {
        if (dna == null) {
            throw new IllegalArgumentException("Input DNA string cannot be null.");
        }
        int n = dna.length();
        if (n == 0) {
            throw new IllegalArgumentException("Input DNA string cannot be empty.");
        }
        StringBuilder sb = new StringBuilder(n);
        for (int i = 0; i < n; i++) {
            char ch = dna.charAt(i);
            char up = (ch >= 'a' && ch <= 'z') ? (char)(ch - 32) : ch; // fast ASCII uppercase
            switch (up) {
                case 'A': sb.append('T'); break;
                case 'T': sb.append('A'); break;
                case 'C': sb.append('G'); break;
                case 'G': sb.append('C'); break;
                default:
                    throw new IllegalArgumentException("Invalid DNA character at index " + i + ": '" + ch + "'");
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = {
            "ATTGC",   // expected: TAACG
            "GTAT",    // expected: CATA
            "A",       // expected: T
            "aaaa",    // expected: TTTT
            "gCatTa"   // expected: CGTAAT
        };
        for (String t : tests) {
            try {
                String res = dnaComplement(t);
                System.out.println("Input: " + t + " -> " + res);
            } catch (IllegalArgumentException ex) {
                System.out.println("Input: " + t + " -> Error: " + ex.getMessage());
            }
        }
    }
}