import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.*;
import java.net.http.*;
import java.time.Duration;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class Task129 {

    private static final Pattern SAFE_FILENAME = Pattern.compile("^[A-Za-z0-9._-]{1,255}$");
    private static final int MAX_DOWNLOAD_BYTES = 5 * 1024 * 1024; // 5 MB
    private static final Duration CONNECT_TIMEOUT = Duration.ofSeconds(5);
    private static final Duration REQUEST_TIMEOUT = Duration.ofSeconds(15);

    public static byte[] retrieveFileFromServer(String baseUrl, String fileName) throws Exception {
        if (baseUrl == null || fileName == null) {
            throw new IllegalArgumentException("Base URL and file name must not be null");
        }

        URI baseUri;
        try {
            baseUri = new URI(baseUrl);
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException("Invalid base URL");
        }

        String scheme = baseUri.getScheme();
        if (scheme == null || !(scheme.equalsIgnoreCase("https") || scheme.equalsIgnoreCase("http"))) {
            throw new IllegalArgumentException("Only http and https schemes are allowed");
        }

        if (baseUri.getHost() == null) {
            throw new IllegalArgumentException("Base URL must include a valid host");
        }

        if (!isSafeRemoteHost(baseUri)) {
            throw new SecurityException("Disallowed destination host");
        }

        if (!isSafeFilename(fileName)) {
            throw new IllegalArgumentException("Invalid file name");
        }

        URI target = resolveFileUri(baseUri, "files", fileName);

        HttpClient client = HttpClient.newBuilder()
                .connectTimeout(CONNECT_TIMEOUT)
                .followRedirects(HttpClient.Redirect.NEVER) // prevent redirect-based SSRF
                .build();

        HttpRequest request = HttpRequest.newBuilder()
                .uri(target)
                .timeout(REQUEST_TIMEOUT)
                .header("Accept", "application/octet-stream")
                .GET()
                .build();

        HttpResponse<InputStream> response = client.send(request, HttpResponse.BodyHandlers.ofInputStream());

        int status = response.statusCode();
        if (status != 200) {
            throw new IOException("HTTP error: " + status);
        }

        List<String> clHeaders = response.headers().allValues("Content-Length");
        if (!clHeaders.isEmpty()) {
            try {
                long len = Long.parseLong(clHeaders.get(0));
                if (len < 0 || len > MAX_DOWNLOAD_BYTES) {
                    throw new IOException("Content too large");
                }
            } catch (NumberFormatException ignored) {
            }
        }

        try (InputStream is = response.body(); ByteArrayOutputStream baos = new ByteArrayOutputStream()) {
            byte[] buf = new byte[8192];
            int read;
            int total = 0;
            while ((read = is.read(buf)) != -1) {
                total += read;
                if (total > MAX_DOWNLOAD_BYTES) {
                    throw new IOException("Download exceeded size limit");
                }
                baos.write(buf, 0, read);
            }
            return baos.toByteArray();
        }
    }

    private static boolean isSafeFilename(String name) {
        if (!SAFE_FILENAME.matcher(name).matches()) return false;
        if (name.contains("..")) return false;
        return true;
    }

    private static URI resolveFileUri(URI base, String folder, String fileName) {
        String basePath = base.getPath();
        if (basePath == null) basePath = "/";
        if (!basePath.endsWith("/")) basePath += "/";
        String path = basePath + sanitizePathSegment(folder) + "/" + sanitizePathSegment(fileName);
        try {
            return new URI(base.getScheme(), base.getAuthority(), path, null, null);
        } catch (URISyntaxException e) {
            throw new IllegalArgumentException("Failed to build target URI");
        }
    }

    private static String sanitizePathSegment(String seg) {
        // We already ensure only safe characters, so return as-is
        return seg;
    }

    private static boolean isSafeRemoteHost(URI uri) throws UnknownHostException {
        String host = uri.getHost();
        if (host == null) return false;
        InetAddress[] addresses = InetAddress.getAllByName(host);
        for (InetAddress addr : addresses) {
            if (addr.isAnyLocalAddress()) return false;
            if (addr.isLoopbackAddress()) return false;
            if (addr.isLinkLocalAddress()) return false;
            if (addr.isSiteLocalAddress()) return false;
            if (addr.isMulticastAddress()) return false;
        }
        return true;
    }

    private static void runTest(String baseUrl, String fileName) {
        try {
            byte[] data = retrieveFileFromServer(baseUrl, fileName);
            System.out.println("OK: " + fileName + " size=" + data.length);
        } catch (Exception e) {
            System.out.println("ERR: " + fileName + " -> " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        String base = "https://example.com";
        List<String> tests = new ArrayList<>();
        tests.add("document.txt");
        tests.add("image-file_01.png");
        tests.add("..bad.txt");           // invalid
        tests.add("notes..txt");          // invalid
        tests.add("really_really_long-name.v1.2.3.tar.gz");

        for (String fn : tests) {
            runTest(base, fn);
        }
    }
}