import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.io.IOException;
import java.io.FileOutputStream;

public class Lse69 {
    public static String upload_image(String srcPath, String outBase64Path) throws IOException {
        byte[] data = Files.readAllBytes(Paths.get(srcPath));
        String encoded = Base64.getEncoder().encodeToString(data);
        Files.write(Paths.get(outBase64Path), encoded.getBytes(StandardCharsets.UTF_8));
        return encoded;
    }

    private static void writeBytes(String path, byte[] data) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(path)) {
            fos.write(data);
        }
    }

    public static void main(String[] args) throws Exception {
        String[] srcs = {
            "java_img1.bin", "java_img2.bin", "java_img3.bin", "java_img4.bin", "java_img5.bin"
        };
        String[] outs = {
            "java_img1.b64", "java_img2.b64", "java_img3.b64", "java_img4.b64", "java_img5.b64"
        };
        byte[][] samples = new byte[][]{
            new byte[]{0,1,2,3,4,5,6,7,8,9},
            new byte[]{(byte)0xFF,(byte)0xD8,(byte)0xFF,0x00,0x11,0x22,0x33,0x44},
            new byte[]{(byte)0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A},
            "Hello, World!".getBytes(StandardCharsets.UTF_8),
            new byte[]{(byte)0x00,(byte)0xFF,(byte)0xAA,(byte)0x55,(byte)0xCC,(byte)0x33,(byte)0x77,(byte)0x88,(byte)0x99}
        };

        for (int i = 0; i < 5; i++) {
            writeBytes(srcs[i], samples[i]);
            String b64 = upload_image(srcs[i], outs[i]);
            System.out.println("Test " + (i+1) + ": src=" + srcs[i] + ", out=" + outs[i] + ", b64_len=" + b64.length());
        }
    }
}