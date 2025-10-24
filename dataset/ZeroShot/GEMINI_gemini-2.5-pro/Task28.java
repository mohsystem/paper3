import java.util.Arrays;

public class Task28 {

    /**
     * Creates the display text for people who liked an item.
     *
     * @param names An array of names of people who liked the item. Must not be null.
     * @return The formatted string.
     */
    public static String whoLikesIt(String... names) {
        // A switch statement is a clean way to handle the different cases based on the number of names.
        switch (names.length) {
            case 0:
                return "no one likes this";
            case 1:
                // String.format is used for safe and readable string construction.
                return String.format("%s likes this", names[0]);
            case 2:
                return String.format("%s and %s like this", names[0], names[1]);
            case 3:
                return String.format("%s, %s and %s like this", names[0], names[1], names[2]);
            default:
                // For 4 or more names, we calculate the number of "others".
                return String.format("%s, %s and %d others like this", names[0], names[1], names.length - 2);
        }
    }

    public static void main(String[] args) {
        // Test Cases
        System.out.println("Test Case 1: []");
        System.out.println("Expected: no one likes this");
        System.out.println("Actual:   " + whoLikesIt());
        System.out.println("--------------------");

        System.out.println("Test Case 2: [\"Peter\"]");
        System.out.println("Expected: Peter likes this");
        System.out.println("Actual:   " + whoLikesIt("Peter"));
        System.out.println("--------------------");

        System.out.println("Test Case 3: [\"Jacob\", \"Alex\"]");
        System.out.println("Expected: Jacob and Alex like this");
        System.out.println("Actual:   " + whoLikesIt("Jacob", "Alex"));
        System.out.println("--------------------");

        System.out.println("Test Case 4: [\"Max\", \"John\", \"Mark\"]");
        System.out.println("Expected: Max, John and Mark like this");
        System.out.println("Actual:   " + whoLikesIt("Max", "John", "Mark"));
        System.out.println("--------------------");

        System.out.println("Test Case 5: [\"Alex\", \"Jacob\", \"Mark\", \"Max\"]");
        System.out.println("Expected: Alex, Jacob and 2 others like this");
        System.out.println("Actual:   " + whoLikesIt("Alex", "Jacob", "Mark", "Max"));
        System.out.println("--------------------");
    }
}