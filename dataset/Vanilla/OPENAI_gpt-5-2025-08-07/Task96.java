import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class Task96 {

    public static class ServerControl {
        private final ServerSocket serverSocket;
        private final Thread thread;
        private volatile boolean running = true;

        private ServerControl(ServerSocket ss, Thread t) {
            this.serverSocket = ss;
            this.thread = t;
        }

        public int getPort() {
            return serverSocket.getLocalPort();
        }

        public void stop() {
            running = false;
            try {
                serverSocket.close();
            } catch (IOException ignored) {}
            try {
                thread.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }

    public static ServerControl startEchoServer(int port) throws IOException {
        ServerSocket serverSocket = new ServerSocket();
        serverSocket.setReuseAddress(true);
        serverSocket.bind(new InetSocketAddress("127.0.0.1", port));
        Thread t = new Thread(() -> {
            try {
                while (!Thread.currentThread().isInterrupted()) {
                    Socket client;
                    try {
                        client = serverSocket.accept();
                    } catch (SocketException se) {
                        // Likely closed during shutdown
                        break;
                    }
                    try (Socket c = client;
                         InputStream in = c.getInputStream();
                         OutputStream out = c.getOutputStream()) {
                        byte[] buf = new byte[4096];
                        int n;
                        while ((n = in.read(buf)) != -1) {
                            out.write(buf, 0, n);
                            out.flush();
                        }
                    } catch (IOException ignored) {}
                }
            } catch (Exception ignored) {
            } finally {
                try {
                    serverSocket.close();
                } catch (IOException ignored) {}
            }
        }, "EchoServerThread");
        t.setDaemon(true);
        t.start();
        return new ServerControl(serverSocket, t);
    }

    public static String echoOnce(String host, int port, String message) throws IOException {
        byte[] data = message.getBytes(StandardCharsets.UTF_8);
        try (Socket s = new Socket()) {
            s.connect(new InetSocketAddress(host, port), 3000);
            s.setSoTimeout(3000);
            OutputStream out = s.getOutputStream();
            InputStream in = s.getInputStream();
            out.write(data);
            out.flush();
            s.shutdownOutput();

            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] buf = new byte[4096];
            int n;
            while ((n = in.read(buf)) != -1) {
                baos.write(buf, 0, n);
            }
            return baos.toString(StandardCharsets.UTF_8);
        }
    }

    public static void main(String[] args) throws Exception {
        ServerControl server = startEchoServer(0);
        int port = server.getPort();

        String[] tests = new String[] {
            "hello",
            "world",
            "こんにちは",
            "1234567890",
            "echo test with spaces"
        };

        for (String msg : tests) {
            String echoed = echoOnce("127.0.0.1", port, msg);
            System.out.println("Sent:   " + msg);
            System.out.println("Echoed: " + echoed);
            System.out.println("Match:  " + msg.equals(echoed));
            System.out.println("---");
        }

        server.stop();
    }
}