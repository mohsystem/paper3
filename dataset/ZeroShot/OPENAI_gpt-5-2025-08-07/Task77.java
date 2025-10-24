import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.Locale;

public class Task77 {

    public static String downloadFile(String host, int port, String username, String password, String remotePath, int timeoutMillis) {
        Socket control = null;
        BufferedReader reader = null;
        BufferedWriter writer = null;
        Socket data = null;
        FileOutputStream fos = null;
        String localFile = null;

        try {
            control = new Socket();
            control.connect(new InetSocketAddress(host, port), timeoutMillis);
            control.setSoTimeout(timeoutMillis);
            reader = new BufferedReader(new InputStreamReader(control.getInputStream(), StandardCharsets.US_ASCII));
            writer = new BufferedWriter(new OutputStreamWriter(control.getOutputStream(), StandardCharsets.US_ASCII));

            FtpReply reply = readReply(reader);
            if (reply.code != 220) throw new IOException("Unexpected greeting: " + reply.code);

            sendCommand(writer, "USER " + safeArg(username));
            reply = readReply(reader);
            if (reply.code == 331) {
                sendCommand(writer, "PASS " + safeArg(password));
                reply = readReply(reader);
            }
            if (reply.code != 230) throw new IOException("Login failed: " + reply.code);

            sendCommand(writer, "TYPE I");
            reply = readReply(reader);
            if (reply.code != 200) throw new IOException("Failed to set binary mode: " + reply.code);

            // Try EPSV first
            int dataPort = -1;
            sendCommand(writer, "EPSV");
            reply = readReply(reader);
            if (reply.code == 229) {
                dataPort = parseEpsvPort(reply.message);
            } else {
                // Fallback to PASV
                sendCommand(writer, "PASV");
                reply = readReply(reader);
                if (reply.code != 227) throw new IOException("PASV/EPSV failed: " + reply.code);
                InetSocketAddress pasvAddr = parsePasvAddress(reply.message);
                if (pasvAddr == null) throw new IOException("Could not parse PASV response");
                InetAddress controlAddr = control.getInetAddress();
                // Mitigate bounce attack: use control connection address.
                dataPort = pasvAddr.getPort();
            }

            if (dataPort <= 0 || dataPort > 65535) throw new IOException("Invalid data port: " + dataPort);

            // Open data connection to same address as control
            data = new Socket();
            data.connect(new InetSocketAddress(control.getInetAddress(), dataPort), timeoutMillis);
            data.setSoTimeout(timeoutMillis);

            sendCommand(writer, "RETR " + remotePath);
            reply = readReply(reader);
            if (reply.code >= 400) throw new IOException("Server refused RETR: " + reply.code);

            String baseName = sanitizeFilename(new File(remotePath).getName());
            if (baseName.isEmpty()) baseName = "downloaded.bin";
            localFile = uniqueFilename(baseName);

            fos = new FileOutputStream(localFile);
            InputStream din = data.getInputStream();
            byte[] buf = new byte[8192];
            int n;
            while ((n = din.read(buf)) != -1) {
                fos.write(buf, 0, n);
            }
            fos.flush();
            safeClose(fos); fos = null;
            safeClose(data); data = null;

            reply = readReply(reader);
            if (reply.code != 226 && reply.code != 250) throw new IOException("Transfer did not complete: " + reply.code);

            // Try to quit gracefully
            try {
                sendCommand(writer, "QUIT");
                readReply(reader);
            } catch (Exception ignore) {}

            return localFile;
        } catch (Exception ex) {
            if (localFile != null) {
                File f = new File(localFile);
                if (f.exists()) {
                    try { f.delete(); } catch (Exception ignore) {}
                }
            }
            return null;
        } finally {
            safeClose(fos);
            safeClose(data);
            safeClose(reader);
            safeClose(writer);
            safeClose(control);
        }
    }

    private static String safeArg(String s) {
        return s == null ? "" : s.replaceAll("[\\r\\n]", "");
    }

    private static void sendCommand(BufferedWriter writer, String cmd) throws IOException {
        writer.write(cmd);
        writer.write("\r\n");
        writer.flush();
    }

    private static FtpReply readReply(BufferedReader reader) throws IOException {
        String line = reader.readLine();
        if (line == null) throw new EOFException("Connection closed");
        if (line.length() < 3 || !Character.isDigit(line.charAt(0)) || !Character.isDigit(line.charAt(1)) || !Character.isDigit(line.charAt(2))) {
            throw new IOException("Malformed reply: " + line);
        }
        int code = Integer.parseInt(line.substring(0, 3));
        StringBuilder sb = new StringBuilder();
        sb.append(line).append("\n");
        boolean multiline = line.length() > 3 && line.charAt(3) == '-';
        if (multiline) {
            String end = String.format(Locale.ROOT, "%03d ", code);
            String l;
            while ((l = reader.readLine()) != null) {
                sb.append(l).append("\n");
                if (l.startsWith(end)) break;
            }
        }
        FtpReply r = new FtpReply();
        r.code = code;
        r.message = sb.toString();
        return r;
    }

    private static InetSocketAddress parsePasvAddress(String message) {
        // Look for (h1,h2,h3,h4,p1,p2)
        int start = message.indexOf('(');
        int end = message.indexOf(')', start + 1);
        if (start == -1 || end == -1) return null;
        String inside = message.substring(start + 1, end);
        String[] parts = inside.split(",");
        if (parts.length != 6) return null;
        try {
            String host = String.join(".", parts[0].trim(), parts[1].trim(), parts[2].trim(), parts[3].trim());
            int p1 = Integer.parseInt(parts[4].trim());
            int p2 = Integer.parseInt(parts[5].trim());
            int port = (p1 << 8) + p2;
            return new InetSocketAddress(host, port);
        } catch (Exception e) {
            return null;
        }
    }

    private static int parseEpsvPort(String message) {
        // 229 Entering Extended Passive Mode (|||port|)
        int first = message.indexOf('(');
        int last = message.indexOf(')', first + 1);
        if (first == -1 || last == -1) return -1;
        String inside = message.substring(first + 1, last);
        // The delimiter is the first char
        if (inside.length() < 5) return -1;
        char delim = inside.charAt(0);
        String[] parts = inside.split("\\" + delim);
        // Expected: "", "", "", "port", ""
        for (String p : parts) {
            // no-op, just to ensure split works
        }
        if (parts.length >= 4) {
            try {
                return Integer.parseInt(parts[3]);
            } catch (NumberFormatException ignored) {}
        }
        return -1;
    }

    private static String sanitizeFilename(String name) {
        if (name == null) return "";
        StringBuilder sb = new StringBuilder();
        for (char c : name.toCharArray()) {
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '.' || c == '_' || c == '-') {
                sb.append(c);
            } else {
                sb.append('_');
            }
        }
        String result = sb.toString();
        // Avoid hidden or empty names
        if (result.equals("") || result.equals(".") || result.equals("..")) return "";
        return result;
    }

    private static String uniqueFilename(String base) {
        File f = new File(base);
        if (!f.exists()) return base;
        String name = base;
        String prefix = base;
        String ext = "";
        int dot = base.lastIndexOf('.');
        if (dot > 0) {
            prefix = base.substring(0, dot);
            ext = base.substring(dot);
        }
        for (int i = 1; i < 10000; i++) {
            name = prefix + "_" + i + ext;
            f = new File(name);
            if (!f.exists()) return name;
        }
        return prefix + "_" + System.currentTimeMillis() + ext;
    }

    private static void safeClose(Closeable c) {
        if (c != null) {
            try { c.close(); } catch (Exception ignore) {}
        }
    }

    private static void safeClose(Socket s) {
        if (s != null) {
            try { s.close(); } catch (Exception ignore) {}
        }
    }

    private static class FtpReply {
        int code;
        String message;
    }

    public static void main(String[] args) {
        if (args.length >= 5) {
            String host = args[0];
            int port = 21;
            try { port = Integer.parseInt(args[1]); } catch (Exception ignored) {}
            String user = args[2];
            String pass = args[3];
            String remote = args[4];

            String saved = downloadFile(host, port, user, pass, remote, 15000);
            if (saved != null) {
                System.out.println("Downloaded to: " + saved);
            } else {
                System.out.println("Download failed");
            }
        } else {
            // 5 test cases (placeholders)
            String[][] tests = new String[][] {
                {"localhost","21","user","pass","/sensitive.txt"},
                {"127.0.0.1","2121","anonymous","anonymous@","/readme.txt"},
                {"ftp.example.com","21","user1","secret1","/pub/file1.bin"},
                {"ftp.example.org","21","user2","secret2","/data/report.pdf"},
                {"192.0.2.1","21","test","test","/tmp/sample.dat"}
            };
            for (int i = 0; i < tests.length; i++) {
                try {
                    String saved = downloadFile(tests[i][0], Integer.parseInt(tests[i][1]), tests[i][2], tests[i][3], tests[i][4], 5000);
                    System.out.println("Test " + (i+1) + ": " + (saved != null ? "Downloaded to " + saved : "Failed"));
                } catch (Exception e) {
                    System.out.println("Test " + (i+1) + ": Failed");
                }
            }
        }
    }
}