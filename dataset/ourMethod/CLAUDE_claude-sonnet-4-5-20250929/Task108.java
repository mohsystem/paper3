
import javax.crypto.*;
import javax.crypto.spec.*;
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.security.spec.*;
import java.util.*;
import java.util.concurrent.*;

public class Task108 {
    private static final String MAGIC = "RMI1";
    private static final int VERSION = 1;
    private static final int SALT_SIZE = 16;
    private static final int IV_SIZE = 12;
    private static final int TAG_SIZE = 16;
    private static final int PBKDF2_ITERATIONS = 210000;
    private static final int MAX_PAYLOAD_SIZE = 1048576;
    private static final int MAX_METHOD_NAME_LENGTH = 128;
    private static final int MAX_OBJECT_ID_LENGTH = 64;
    
    static class SecureRemoteObject {
        private final Map<String, Object> storage = new ConcurrentHashMap<>();
        private final Set<String> allowedMethods = new HashSet<>(Arrays.asList("get", "set", "remove", "exists"));
        
        public synchronized String invoke(String method, String objectId, String value) {
            if (method == null || objectId == null) {
                return "ERROR: null parameters";
            }
            if (method.length() > MAX_METHOD_NAME_LENGTH || objectId.length() > MAX_OBJECT_ID_LENGTH) {
                return "ERROR: parameter too long";
            }
            if (!allowedMethods.contains(method)) {
                return "ERROR: method not allowed";
            }
            
            switch (method) {
                case "get":
                    Object obj = storage.get(objectId);
                    return obj != null ? obj.toString() : "NULL";
                case "set":
                    if (value == null || value.length() > 1024) {
                        return "ERROR: invalid value";
                    }
                    storage.put(objectId, value);
                    return "OK";
                case "remove":
                    storage.remove(objectId);
                    return "OK";
                case "exists":
                    return storage.containsKey(objectId) ? "TRUE" : "FALSE";
                default:
                    return "ERROR: unknown method";
            }
        }
    }
    
    static class SecureProtocol {
        private final SecretKey key;
        
        public SecureProtocol(String passphrase) throws Exception {
            if (passphrase == null || passphrase.length() < 12) {
                throw new IllegalArgumentException("Passphrase too short");
            }
            byte[] salt = new byte[SALT_SIZE];
            SecureRandom.getInstanceStrong().nextBytes(salt);
            this.key = deriveKey(passphrase, salt);
        }
        
        private SecretKey deriveKey(String passphrase, byte[] salt) throws Exception {
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            KeySpec spec = new PBEKeySpec(passphrase.toCharArray(), salt, PBKDF2_ITERATIONS, 256);
            SecretKey tmp = factory.generateSecret(spec);
            return new SecretKeySpec(tmp.getEncoded(), "AES");
        }
        
        public byte[] encrypt(String plaintext) throws Exception {
            if (plaintext == null || plaintext.length() > MAX_PAYLOAD_SIZE) {
                throw new IllegalArgumentException("Invalid plaintext");
            }
            
            byte[] iv = new byte[IV_SIZE];
            SecureRandom.getInstanceStrong().nextBytes(iv);
            
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_SIZE * 8, iv);
            cipher.init(Cipher.ENCRYPT_MODE, key, gcmSpec);
            
            byte[] plainBytes = plaintext.getBytes(StandardCharsets.UTF_8);
            byte[] ciphertext = cipher.doFinal(plainBytes);
            
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            baos.write(MAGIC.getBytes(StandardCharsets.UTF_8));
            baos.write(VERSION);
            baos.write(iv);
            baos.write(ciphertext);
            
            return baos.toByteArray();
        }
        
        public String decrypt(byte[] encrypted) throws Exception {
            if (encrypted == null || encrypted.length < MAGIC.length() + 1 + IV_SIZE + TAG_SIZE) {
                throw new IllegalArgumentException("Invalid encrypted data");
            }
            
            ByteArrayInputStream bais = new ByteArrayInputStream(encrypted);
            byte[] magicBytes = new byte[MAGIC.length()];
            bais.read(magicBytes);
            
            if (!MAGIC.equals(new String(magicBytes, StandardCharsets.UTF_8))) {
                throw new SecurityException("Invalid magic");
            }
            
            int version = bais.read();
            if (version != VERSION) {
                throw new SecurityException("Invalid version");
            }
            
            byte[] iv = new byte[IV_SIZE];
            bais.read(iv);
            
            byte[] ciphertext = new byte[encrypted.length - MAGIC.length() - 1 - IV_SIZE];
            bais.read(ciphertext);
            
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_SIZE * 8, iv);
            cipher.init(Cipher.DECRYPT_MODE, key, gcmSpec);
            
            byte[] plainBytes = cipher.doFinal(ciphertext);
            return new String(plainBytes, StandardCharsets.UTF_8);
        }
    }
    
    static class RemoteServer {
        private final SecureRemoteObject remoteObject;
        private final SecureProtocol protocol;
        private final int port;
        
        public RemoteServer(int port, String passphrase) throws Exception {
            if (port < 1024 || port > 65535) {
                throw new IllegalArgumentException("Invalid port");
            }
            this.port = port;
            this.remoteObject = new SecureRemoteObject();
            this.protocol = new SecureProtocol(passphrase);
        }
        
        public void start() {
            try (ServerSocket serverSocket = new ServerSocket(port)) {
                serverSocket.setSoTimeout(30000);
                
                try (Socket clientSocket = serverSocket.accept()) {
                    clientSocket.setSoTimeout(10000);
                    handleClient(clientSocket);
                }
            } catch (Exception e) {
                System.err.println("Server error");
            }
        }
        
        private void handleClient(Socket clientSocket) {
            try (InputStream in = clientSocket.getInputStream();
                 OutputStream out = clientSocket.getOutputStream()) {
                
                byte[] buffer = new byte[MAX_PAYLOAD_SIZE];
                int bytesRead = in.read(buffer);
                
                if (bytesRead <= 0 || bytesRead > MAX_PAYLOAD_SIZE) {
                    return;
                }
                
                byte[] request = Arrays.copyOf(buffer, bytesRead);
                String decrypted = protocol.decrypt(request);
                
                String[] parts = decrypted.split("\\\\|", 3);
                if (parts.length < 2) {
                    return;
                }
                
                String method = parts[0];
                String objectId = parts[1];
                String value = parts.length > 2 ? parts[2] : null;
                
                String result = remoteObject.invoke(method, objectId, value);
                byte[] response = protocol.encrypt(result);
                out.write(response);
                out.flush();
                
            } catch (Exception e) {
                System.err.println("Client handling error");
            }
        }
    }
    
    static class RemoteClient {
        private final String host;
        private final int port;
        private final SecureProtocol protocol;
        
        public RemoteClient(String host, int port, String passphrase) throws Exception {
            if (host == null || host.isEmpty() || port < 1024 || port > 65535) {
                throw new IllegalArgumentException("Invalid connection parameters");
            }
            this.host = host;
            this.port = port;
            this.protocol = new SecureProtocol(passphrase);
        }
        
        public String call(String method, String objectId, String value) {
            try (Socket socket = new Socket(host, port)) {
                socket.setSoTimeout(10000);
                
                String request = method + "|" + objectId + (value != null ? "|" + value : "");
                byte[] encrypted = protocol.encrypt(request);
                
                try (OutputStream out = socket.getOutputStream();
                     InputStream in = socket.getInputStream()) {
                    
                    out.write(encrypted);
                    out.flush();
                    
                    byte[] buffer = new byte[MAX_PAYLOAD_SIZE];
                    int bytesRead = in.read(buffer);
                    
                    if (bytesRead <= 0) {
                        return "ERROR: no response";
                    }
                    
                    byte[] response = Arrays.copyOf(buffer, bytesRead);
                    return protocol.decrypt(response);
                }
            } catch (Exception e) {
                return "ERROR: connection failed";
            }
        }
    }
    
    public static void main(String[] args) {
        try {
            String passphrase = "TestPassphrase123!@#";
            int testPort = 9999;
            
            Thread serverThread = new Thread(() -> {
                try {
                    RemoteServer server = new RemoteServer(testPort, passphrase);
                    server.start();
                } catch (Exception e) {
                    System.err.println("Server startup failed");
                }
            });
            serverThread.start();
            
            Thread.sleep(1000);
            
            RemoteClient client = new RemoteClient("localhost", testPort, passphrase);
            
            System.out.println("Test 1 - Set value: " + client.call("set", "key1", "value1"));
            System.out.println("Test 2 - Get value: " + client.call("get", "key1", null));
            System.out.println("Test 3 - Check exists: " + client.call("exists", "key1", null));
            System.out.println("Test 4 - Remove value: " + client.call("remove", "key1", null));
            System.out.println("Test 5 - Get removed: " + client.call("get", "key1", null));
            
            serverThread.join(2000);
            
        } catch (Exception e) {
            System.err.println("Test execution failed");
        }
    }
}
