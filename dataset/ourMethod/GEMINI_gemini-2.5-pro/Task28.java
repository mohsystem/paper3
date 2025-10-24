import java.util.Arrays;

public class Task28 {

    /**
     * Creates a display text for "likes" based on an array of names.
     * @param names An array of strings representing the names of people who liked an item.
     * @return A formatted string.
     */
    public static String whoLikesIt(String[] names) {
        // According to Rule #1, validate input. A null check is a basic validation.
        if (names == null) {
            return "no one likes this";
        }

        switch (names.length) {
            case 0:
                return "no one likes this";
            case 1:
                return String.format("%s likes this", names[0]);
            case 2:
                return String.format("%s and %s like this", names[0], names[1]);
            case 3:
                return String.format("%s, %s and %s like this", names[0], names[1], names[2]);
            default:
                return String.format("%s, %s and %d others like this", names[0], names[1], names.length - 2);
        }
    }

    public static void main(String[] args) {
        // Test case 1
        String[] test1 = {};
        System.out.println("Input: " + Arrays.toString(test1) + " -> Output: \"" + whoLikesIt(test1) + "\"");

        // Test case 2
        String[] test2 = {"Peter"};
        System.out.println("Input: " + Arrays.toString(test2) + " -> Output: \"" + whoLikesIt(test2) + "\"");

        // Test case 3
        String[] test3 = {"Jacob", "Alex"};
        System.out.println("Input: " + Arrays.toString(test3) + " -> Output: \"" + whoLikesIt(test3) + "\"");

        // Test case 4
        String[] test4 = {"Max", "John", "Mark"};
        System.out.println("Input: " + Arrays.toString(test4) + " -> Output: \"" + whoLikesIt(test4) + "\"");

        // Test case 5
        String[] test5 = {"Alex", "Jacob", "Mark", "Max"};
        System.out.println("Input: " + Arrays.toString(test5) + " -> Output: \"" + whoLikesIt(test5) + "\"");
    }
}