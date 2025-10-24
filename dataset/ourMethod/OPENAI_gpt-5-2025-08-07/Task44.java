import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.stream.Collectors;

public class Task44 {

    private static final int NAME_MIN_LEN = 1;
    private static final int NAME_MAX_LEN = 50;
    private static final int AGE_MIN = 0;
    private static final int AGE_MAX = 120;
    private static final int NUM_MIN = -1_000_000;
    private static final int NUM_MAX = 1_000_000;
    private static final int NUM_COUNT_MIN = 1;
    private static final int NUM_COUNT_MAX = 100;
    private static final int INPUT_MAX_LEN = 1024;

    public static void main(String[] args) {
        // 5 test cases
        try {
            System.out.println("== Test Case 1 ==");
            System.out.println(processUserData(validateName("Alice"), 29, new int[]{1, 2, 3, 4, 5}));
        } catch (Exception e) {
            System.out.println("Test 1 error: " + e.getMessage());
        }
        try {
            System.out.println("== Test Case 2 ==");
            System.out.println(processUserData(validateName("Bob O'Neil"), 0, new int[]{-10, 0, 10, 10}));
        } catch (Exception e) {
            System.out.println("Test 2 error: " + e.getMessage());
        }
        try {
            System.out.println("== Test Case 3 ==");
            System.out.println(processUserData(validateName("Charles-John"), 120, new int[]{1_000_000, -1_000_000}));
        } catch (Exception e) {
            System.out.println("Test 3 error: " + e.getMessage());
        }
        try {
            System.out.println("== Test Case 4 ==");
            System.out.println(processUserData(validateName("  Mary   Jane  "), 45, new int[]{2, 2, 2, 3, 3, 4}));
        } catch (Exception e) {
            System.out.println("Test 4 error: " + e.getMessage());
        }
        try {
            System.out.println("== Test Case 5 ==");
            System.out.println(processUserData(validateName("D'Artagnan"), 35, new int[]{7, 7, 7, 7, 7}));
        } catch (Exception e) {
            System.out.println("Test 5 error: " + e.getMessage());
        }

        // Interactive section
        try (Scanner sc = new Scanner(System.in, StandardCharsets.UTF_8)) {
            System.out.println("Enter name (letters, spaces, hyphen, apostrophe; 1-50 chars):");
            String nameLine = safeReadLine(sc);
            String name = validateName(nameLine);

            System.out.println("Enter age (0-120):");
            String ageLine = safeReadLine(sc);
            int age = validateAge(ageLine);

            System.out.println("Enter up to 100 integers between -1000000 and 1000000, separated by spaces or commas:");
            String numsLine = safeReadLine(sc);
            int[] numbers = parseNumbers(numsLine);

            String result = processUserData(name, age, numbers);
            System.out.println(result);
        } catch (Exception e) {
            System.out.println("Input error: " + e.getMessage());
        }
    }

    private static String safeReadLine(Scanner sc) {
        if (!sc.hasNextLine()) {
            throw new IllegalArgumentException("No input provided.");
        }
        String line = sc.nextLine();
        if (line == null) {
            throw new IllegalArgumentException("No input provided.");
        }
        if (line.length() > INPUT_MAX_LEN) {
            throw new IllegalArgumentException("Input too long.");
        }
        return line;
    }

    public static String validateName(String raw) {
        if (raw == null) {
            throw new IllegalArgumentException("Name is required.");
        }
        String trimmed = raw.trim();
        String normalized = collapseSpaces(trimmed);
        if (normalized.length() < NAME_MIN_LEN || normalized.length() > NAME_MAX_LEN) {
            throw new IllegalArgumentException("Name must be between " + NAME_MIN_LEN + " and " + NAME_MAX_LEN + " characters after trimming.");
        }
        for (int i = 0; i < normalized.length(); i++) {
            char c = normalized.charAt(i);
            if (!isAllowedNameChar(c)) {
                throw new IllegalArgumentException("Name contains invalid character: '" + c + "'");
            }
        }
        return normalized;
    }

    private static String collapseSpaces(String s) {
        StringBuilder sb = new StringBuilder(s.length());
        boolean prevSpace = false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == ' ') {
                if (!prevSpace) {
                    sb.append(c);
                    prevSpace = true;
                }
            } else {
                sb.append(c);
                prevSpace = false;
            }
        }
        return sb.toString();
    }

    private static boolean isAllowedNameChar(char c) {
        if (c >= 'A' && c <= 'Z') return true;
        if (c >= 'a' && c <= 'z') return true;
        if (c == ' ' || c == '-' || c == '\'') return true;
        return false;
    }

    public static int validateAge(String s) {
        if (s == null) throw new IllegalArgumentException("Age is required.");
        String t = s.trim();
        if (t.isEmpty()) throw new IllegalArgumentException("Age is empty.");
        if (t.length() > 3) throw new IllegalArgumentException("Age length invalid.");
        int val;
        try {
            val = Integer.parseInt(t);
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Age must be an integer.");
        }
        if (val < AGE_MIN || val > AGE_MAX) {
            throw new IllegalArgumentException("Age must be between " + AGE_MIN + " and " + AGE_MAX + ".");
        }
        return val;
    }

    public static int[] parseNumbers(String input) {
        if (input == null) throw new IllegalArgumentException("Numbers input is required.");
        String t = input.trim();
        if (t.isEmpty()) throw new IllegalArgumentException("Numbers input is empty.");
        String[] parts = t.split("[,\\s]+");
        if (parts.length < NUM_COUNT_MIN || parts.length > NUM_COUNT_MAX) {
            throw new IllegalArgumentException("Provide between " + NUM_COUNT_MIN + " and " + NUM_COUNT_MAX + " numbers.");
        }
        int[] nums = new int[parts.length];
        for (int i = 0; i < parts.length; i++) {
            String token = parts[i].trim();
            if (token.isEmpty()) throw new IllegalArgumentException("Invalid empty number token.");
            int val;
            try {
                val = Integer.parseInt(token);
            } catch (NumberFormatException e) {
                throw new IllegalArgumentException("Invalid integer: " + token);
            }
            if (val < NUM_MIN || val > NUM_MAX) {
                throw new IllegalArgumentException("Number out of range: " + val);
            }
            nums[i] = val;
        }
        return nums;
    }

    public static String processUserData(String name, int age, int[] numbers) {
        if (name == null) throw new IllegalArgumentException("Name is required.");
        if (age < AGE_MIN || age > AGE_MAX) throw new IllegalArgumentException("Age out of range.");
        if (numbers == null) throw new IllegalArgumentException("Numbers are required.");
        if (numbers.length < NUM_COUNT_MIN || numbers.length > NUM_COUNT_MAX) {
            throw new IllegalArgumentException("Numbers count out of range.");
        }
        for (int v : numbers) {
            if (v < NUM_MIN || v > NUM_MAX) {
                throw new IllegalArgumentException("Number out of range: " + v);
            }
        }

        long sum = 0L;
        int min = numbers[0];
        int max = numbers[0];
        int evens = 0;
        int odds = 0;
        for (int v : numbers) {
            sum += v;
            if (v < min) min = v;
            if (v > max) max = v;
            if ((v & 1) == 0) evens++; else odds++;
        }
        double avg = sum / (double) numbers.length;

        int[] sorted = Arrays.copyOf(numbers, numbers.length);
        Arrays.sort(sorted);
        double median;
        if (sorted.length % 2 == 1) {
            median = sorted[sorted.length / 2];
        } else {
            int mid1 = sorted[sorted.length / 2 - 1];
            int mid2 = sorted[sorted.length / 2];
            median = (mid1 + mid2) / 2.0;
        }

        // Unique sorted
        TreeSet<Integer> uniqueSet = new TreeSet<>();
        for (int v : numbers) uniqueSet.add(v);
        String uniqueList = uniqueSet.stream()
                .map(String::valueOf)
                .collect(Collectors.joining(", ", "[", "]"));

        StringBuilder sb = new StringBuilder();
        sb.append("Result for ").append(name).append("\n");
        sb.append("Age next year: ").append(age + 1).append("\n");
        sb.append(String.format(Locale.US, "Count: %d, Min: %d, Max: %d, Sum: %d, Average: %.2f, Median: %.2f%n",
                numbers.length, min, max, sum, avg, median));
        sb.append("Evens: ").append(evens).append(", Odds: ").append(odds).append("\n");
        sb.append("Unique sorted: ").append(uniqueList);
        return sb.toString();
    }
}