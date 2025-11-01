import java.util.Random;

public class Lse117 {
    public static String a(float x) {
        return Float.toString(x);
    }

    public static void runTestCase(Random rng) {
        float fa = rng.nextFloat();
        float fb = rng.nextFloat();
        float fc = rng.nextFloat();

        String str_a = a(fa);
        String str_b = a(fb);
        String str_c = a(fc);

        System.out.println("str_a=" + str_a + ", str_b=" + str_b + ", str_c=" + str_c);
    }

    public static void main(String[] args) {
        Random rng = new Random();
        for (int i = 0; i < 5; i++) {
            runTestCase(rng);
        }
    }
}