import java.io.FileWriter;
import java.io.IOException;
import java.util.Random;

public class Lse118 {
    public static String generateAndWrite(String filePath, long seed) {
        Random rnd = new Random(seed);
        float f1 = rnd.nextFloat();
        float f2 = rnd.nextFloat();
        float f3 = rnd.nextFloat();
        String concatenated = Float.toString(f1) + "," + Float.toString(f2) + "," + Float.toString(f3);
        try (FileWriter fw = new FileWriter(filePath)) {
            fw.write(concatenated);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        return concatenated;
    }

    public static void main(String[] args) {
        System.out.println(generateAndWrite("out_java_1.txt", 1L));
        System.out.println(generateAndWrite("out_java_2.txt", 2L));
        System.out.println(generateAndWrite("out_java_3.txt", 3L));
        System.out.println(generateAndWrite("out_java_4.txt", 4L));
        System.out.println(generateAndWrite("out_java_5.txt", 5L));
    }
}