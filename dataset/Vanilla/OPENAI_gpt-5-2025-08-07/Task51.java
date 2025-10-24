import java.util.*;

public class Task51 {
    public static String encrypt(String s) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c >= 'A' && c <= 'Z') {
                c = (char) ('A' + (c - 'A' + 3) % 26);
            } else if (c >= 'a' && c <= 'z') {
                c = (char) ('a' + (c - 'a' + 3) % 26);
            } else if (c >= '0' && c <= '9') {
                c = (char) ('0' + (c - '0' + 5) % 10);
            }
            sb.append(c);
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        String[] tests = {
            "Hello, World!",
            "xyz XYZ",
            "Encrypt123",
            "",
            "Attack at dawn! 09"
        };
        for (String t : tests) {
            System.out.println("Input:  " + t);
            System.out.println("Output: " + encrypt(t));
            System.out.println();
        }
    }
}