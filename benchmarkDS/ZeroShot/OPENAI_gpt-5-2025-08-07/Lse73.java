import java.nio.charset.StandardCharsets;

public class Lse73 {
    public static final class Person {
        public String name;
        public int age;
        public double height;

        @Override
        public String toString() {
            return "Person{name='" + name + "', age=" + age + ", height=" + height + "}";
        }
    }

    // Allocates memory for a Person object and returns a reference to it.
    public static Person allocatePerson() {
        return new Person();
    }

    // Safely populate the person fields with basic validation.
    public static boolean setPerson(Person p, String name, int age, double height) {
        if (p == null) return false;
        if (age < 0 || age > 150) return false;
        if (!(height > 0.0 && height < 3.0)) return false;

        if (name == null) name = "";
        // Truncate to 63 characters to mirror fixed-size buffer constraints.
        if (name.length() > 63) {
            // Ensure we don't break surrogate pairs
            byte[] bytes = name.substring(0, 63).getBytes(StandardCharsets.UTF_8);
            String truncated = new String(bytes, StandardCharsets.UTF_8);
            // If truncation caused an invalid UTF-8 ending, back off until valid
            while (truncated.length() > 0 && truncated.length() > 63) {
                truncated = truncated.substring(0, truncated.length() - 1);
            }
            name = truncated.length() > 63 ? truncated.substring(0, 63) : truncated;
            if (name.length() > 63) name = name.substring(0, 63);
        }

        p.name = name;
        p.age = age;
        p.height = height;
        return true;
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] names = {
            "Alice",
            "BobTheBuilderWithAReallyLongNameThatExceedsSixtyThreeCharacters_ABCDEFG",
            "",
            "张伟",
            "Eve"
        };
        int[] ages = {30, 45, 20, 28, 35};
        double[] heights = {1.65, 1.80, 1.70, 1.75, 1.82};

        for (int i = 0; i < 5; i++) {
            Person p = allocatePerson();
            boolean ok = setPerson(p, names[i], ages[i], heights[i]);
            System.out.println("Allocation " + (i + 1) + " success=" + ok + " -> " + p);
        }
    }
}