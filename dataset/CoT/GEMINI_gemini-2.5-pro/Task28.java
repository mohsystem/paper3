public class Task28 {

    /**
     * Creates a display text for who likes an item based on an array of names.
     * @param names An array of strings containing the names of people who like the item.
     * @return A formatted string.
     */
    public static String whoLikesIt(String[] names) {
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
        // Test Case 1: 0 names
        String[] names1 = {};
        System.out.println(whoLikesIt(names1));

        // Test Case 2: 1 name
        String[] names2 = {"Peter"};
        System.out.println(whoLikesIt(names2));

        // Test Case 3: 2 names
        String[] names3 = {"Jacob", "Alex"};
        System.out.println(whoLikesIt(names3));

        // Test Case 4: 3 names
        String[] names4 = {"Max", "John", "Mark"};
        System.out.println(whoLikesIt(names4));

        // Test Case 5: 4 or more names
        String[] names5 = {"Alex", "Jacob", "Mark", "Max"};
        System.out.println(whoLikesIt(names5));
    }
}