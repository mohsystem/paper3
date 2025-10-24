import java.io.BufferedInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLEncoder;
import java.nio.file.Paths;

public class Task77 {

    public static String downloadFtpFile(String host, int port, String username, String password, String remotePath) {
        String localName = Paths.get(remotePath).getFileName().toString();
        String path = remotePath.startsWith("/") ? remotePath : "/" + remotePath;
        InputStream in = null;
        FileOutputStream out = null;
        try {
            String userEnc = URLEncoder.encode(username, "UTF-8");
            String passEnc = URLEncoder.encode(password, "UTF-8");
            String urlString = "ftp://" + userEnc + ":" + passEnc + "@" + host + ":" + port + path + ";type=i";
            URL url = new URL(urlString);

            in = new BufferedInputStream(url.openStream());
            out = new FileOutputStream(localName);

            byte[] buffer = new byte[8192];
            int bytesRead;
            while ((bytesRead = in.read(buffer, 0, buffer.length)) != -1) {
                out.write(buffer, 0, bytesRead);
            }
            return localName;
        } catch (Exception e) {
            return null;
        } finally {
            try {
                if (in != null) in.close();
            } catch (IOException ignored) {}
            try {
                if (out != null) out.close();
            } catch (IOException ignored) {}
        }
    }

    public static void main(String[] args) {
        if (args.length >= 4) {
            String host = args[0];
            String user = args[1];
            String pass = args[2];
            String remote = args[3];
            int port = 21;
            if (args.length >= 5) {
                try {
                    port = Integer.parseInt(args[4]);
                } catch (NumberFormatException ignored) {}
            }
            String saved = downloadFtpFile(host, port, user, pass, remote);
            System.out.println(saved != null ? "Downloaded: " + saved : "Download failed");
        } else {
            // 5 test cases using public test FTP server
            String host = "test.rebex.net";
            int port = 21;
            String user = "demo";
            String pass = "password";
            String[] files = new String[] {
                "pub/example/readme.txt",
                "pub/example/ConsoleClient.png",
                "pub/example/KeyGenerator.png",
                "pub/example/WinFormClient.png",
                "pub/example/WinSshShell.png"
            };
            for (String f : files) {
                String saved = downloadFtpFile(host, port, user, pass, f);
                System.out.println((saved != null ? "Downloaded: " : "Failed: ") + f);
            }
        }
    }
}