import java.util.Arrays;

public class Task113 {
    public static String concatenate(String[] parts) {
        if (parts == null) {
            return "";
        }
        long totalLength = 0L;
        for (String s : parts) {
            if (s != null) {
                totalLength += s.length();
                if (totalLength > Integer.MAX_VALUE) {
                    throw new IllegalArgumentException("Input too large to concatenate safely.");
                }
            }
        }
        StringBuilder sb = new StringBuilder((int) totalLength);
        for (String s : parts) {
            if (s != null) {
                sb.append(s);
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        // Test case 1
        String[] t1 = new String[] {"Hello", " ", "World", "!"};
        System.out.println("Test 1: " + concatenate(t1));

        // Test case 2 (Unicode)
        String[] t2 = new String[] {"naïve", " ", "café", " ", "😊"};
        System.out.println("Test 2: " + concatenate(t2));

        // Test case 3 (null entries)
        String[] t3 = new String[] {"A", null, "B", null, "C"};
        System.out.println("Test 3: " + concatenate(t3));

        // Test case 4 (empty array)
        String[] t4 = new String[] {};
        System.out.println("Test 4: " + concatenate(t4));

        // Test case 5 (empty strings)
        String[] t5 = new String[] {"", "", "x", "", "y", "", "z"};
        System.out.println("Test 5: " + concatenate(t5));
    }
}