public class Task28 {

    public static String whoLikesIt(String... names) {
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
        // Test Case 1
        String[] test1 = {};
        System.out.println(whoLikesIt(test1)); // Expected: "no one likes this"

        // Test Case 2
        String[] test2 = {"Peter"};
        System.out.println(whoLikesIt(test2)); // Expected: "Peter likes this"

        // Test Case 3
        String[] test3 = {"Jacob", "Alex"};
        System.out.println(whoLikesIt(test3)); // Expected: "Jacob and Alex like this"

        // Test Case 4
        String[] test4 = {"Max", "John", "Mark"};
        System.out.println(whoLikesIt(test4)); // Expected: "Max, John and Mark like this"

        // Test Case 5
        String[] test5 = {"Alex", "Jacob", "Mark", "Max"};
        System.out.println(whoLikesIt(test5)); // Expected: "Alex, Jacob and 2 others like this"
    }
}