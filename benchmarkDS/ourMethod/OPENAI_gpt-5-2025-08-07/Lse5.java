import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Lse5 {

    // Core function: returns the element at the specified index if valid.
    public static String getElementAt(String[] arr, int index) {
        if (arr == null) {
            throw new IllegalArgumentException("Invalid input");
        }
        if (index < 0 || index >= arr.length) {
            throw new IllegalArgumentException("Invalid index");
        }
        return arr[index];
    }

    // Safely parse an integer index from a string.
    private static Integer safeParseIndex(String s) {
        if (s == null) {
            return null;
        }
        s = s.trim();
        if (s.isEmpty()) {
            return null;
        }
        // Restrict length to prevent abusive inputs
        if (s.length() > 11) { // allows for "-2147483648"
            return null;
        }
        try {
            return Integer.parseInt(s);
        } catch (NumberFormatException ex) {
            return null;
        }
    }

    // Create a predefined array of elements
    private static String[] createElements() {
        return new String[] { "alpha", "bravo", "charlie", "delta", "echo", "foxtrot" };
    }

    public static void main(String[] args) {
        String[] elements = createElements();

        if (args != null && args.length >= 1) {
            Integer idx = safeParseIndex(args[0]);
            if (idx == null) {
                System.out.println("Invalid input");
                return;
            }
            try {
                String value = getElementAt(elements, idx);
                System.out.println(value);
            } catch (IllegalArgumentException ex) {
                System.out.println("Invalid index");
            }
            return;
        }

        // 5 test cases
        int[] testIndices = new int[] { 0, 2, 5, -1, 6 };
        for (int t : testIndices) {
            try {
                String value = getElementAt(elements, t);
                System.out.println("Index " + t + " -> " + value);
            } catch (IllegalArgumentException ex) {
                System.out.println("Index " + t + " -> Invalid index");
            }
        }
    }
}