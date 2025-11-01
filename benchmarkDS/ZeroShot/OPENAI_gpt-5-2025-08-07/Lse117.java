import java.security.SecureRandom;

public class Lse117 {
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();

    public static String a(float value) {
        return Float.toString(value);
    }

    public static String[] generateRandomFloatStrings() {
        float fa = SECURE_RANDOM.nextFloat();
        float fb = SECURE_RANDOM.nextFloat();
        float fc = SECURE_RANDOM.nextFloat();

        String str_a = a(fa);
        String str_b = a(fb);
        String str_c = a(fc);

        return new String[] { str_a, str_b, str_c };
    }

    public static void main(String[] args) {
        for (int i = 0; i < 5; i++) {
            String[] res = generateRandomFloatStrings();
            System.out.println(res[0] + " " + res[1] + " " + res[2]);
        }
    }
}