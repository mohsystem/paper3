import java.util.concurrent.ThreadLocalRandom;

public class Lse117 {

    // Function 'a' converts a float to string
    public static String a(float value) {
        return Float.toString(value);
    }

    // Generate a random float in [0.0, 1.0)
    public static float randomFloat() {
        return ThreadLocalRandom.current().nextFloat();
    }

    public static void main(String[] args) {
        // 5 test cases
        for (int i = 1; i <= 5; i++) {
            float a_val = randomFloat();
            float b_val = randomFloat();
            float c_val = randomFloat();

            String str_a = a(a_val);
            String str_b = a(b_val);
            String str_c = a(c_val);

            System.out.println("Test Case " + i + ":");
            System.out.println("str_a = " + str_a);
            System.out.println("str_b = " + str_b);
            System.out.println("str_c = " + str_c);
            System.out.println();
        }
    }
}