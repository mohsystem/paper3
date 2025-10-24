import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

/**
 * Minimal, secure XML-RPC server and test client.
 * - Binds only to 127.0.0.1
 * - Disables XXE/DTD
 * - Validates sizes and types
 */
public class Task111 {

    // ===== Utility: XML escape =====
    private static String xmlEscape(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder(Math.min(4096, s.length() + 16));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&apos;"); break;
                default:
                    // Restrict to valid XML char range
                    if (c == 0x9 || c == 0xA || c == 0xD || (c >= 0x20 && c <= 0xD7FF) ||
                        (c >= 0xE000 && c <= 0xFFFD)) {
                        sb.append(c);
                    } else {
                        sb.append('?');
                    }
            }
        }
        return sb.toString();
    }

    // ===== Safe XML parsing =====
    private static Document parseXmlSecurely(byte[] data) throws Exception {
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        dbf.setNamespaceAware(false);
        dbf.setXIncludeAware(false);
        dbf.setExpandEntityReferences(false);
        dbf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        // Harden against XXE
        try { dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true); } catch (Exception ignored) {}
        try { dbf.setFeature("http://xml.org/sax/features/external-general-entities", false); } catch (Exception ignored) {}
        try { dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false); } catch (Exception ignored) {}
        try { dbf.setAttribute(XMLConstants.ACCESS_EXTERNAL_DTD, ""); } catch (Exception ignored) {}
        try { dbf.setAttribute(XMLConstants.ACCESS_EXTERNAL_SCHEMA, ""); } catch (Exception ignored) {}

        DocumentBuilder db = dbf.newDocumentBuilder();
        try (InputStream in = new BufferedInputStream(new java.io.ByteArrayInputStream(data))) {
            return db.parse(in);
        }
    }

    // ===== XML-RPC Request/Response parsing/building =====

    private static class RpcRequest {
        final String methodName;
        final List<Object> params;
        RpcRequest(String methodName, List<Object> params) {
            this.methodName = methodName;
            this.params = params;
        }
    }

    private static RpcRequest parseXmlRpcRequest(byte[] data) throws Exception {
        // Limit parse size already enforced by caller.
        Document doc = parseXmlSecurely(data);
        Element root = doc.getDocumentElement();
        if (root == null || !"methodCall".equals(root.getTagName())) {
            throw new IllegalArgumentException("Invalid XML-RPC: missing methodCall");
        }
        String methodName = null;
        List<Object> params = new ArrayList<>();
        for (Node n = root.getFirstChild(); n != null; n = n.getNextSibling()) {
            if (!(n instanceof Element)) continue;
            Element e = (Element) n;
            if ("methodName".equals(e.getTagName())) {
                methodName = textContentTrim(e);
            } else if ("params".equals(e.getTagName())) {
                for (Node pn = e.getFirstChild(); pn != null; pn = pn.getNextSibling()) {
                    if (!(pn instanceof Element)) continue;
                    Element pe = (Element) pn;
                    if (!"param".equals(pe.getTagName())) continue;
                    Element ve = firstChildElement(pe, "value");
                    if (ve == null) throw new IllegalArgumentException("Invalid XML-RPC: param missing value");
                    params.add(parseValue(ve));
                }
            }
        }
        if (methodName == null || methodName.isEmpty()) {
            throw new IllegalArgumentException("Invalid XML-RPC: missing methodName");
        }
        return new RpcRequest(methodName, params);
    }

    private static String textContentTrim(Element e) {
        StringBuilder sb = new StringBuilder();
        for (Node n = e.getFirstChild(); n != null; n = n.getNextSibling()) {
            if (n.getNodeType() == Node.TEXT_NODE || n.getNodeType() == Node.CDATA_SECTION_NODE) {
                sb.append(n.getNodeValue());
            }
        }
        return sb.toString().trim();
    }

    private static Element firstChildElement(Element parent, String name) {
        for (Node n = parent.getFirstChild(); n != null; n = n.getNextSibling()) {
            if (n instanceof Element) {
                Element e = (Element) n;
                if (name == null || name.equals(e.getTagName())) {
                    return e;
                }
            }
        }
        return null;
    }

    private static Object parseValue(Element valueElem) throws IllegalArgumentException {
        // value may contain a single type element, or direct string
        Element typeElem = firstChildElement(valueElem, null);
        if (typeElem == null) {
            String s = textContentTrim(valueElem);
            return validateString(s);
        }
        String tag = typeElem.getTagName();
        switch (tag) {
            case "int":
            case "i4": {
                String t = textContentTrim(typeElem);
                try {
                    int v = Integer.parseInt(t);
                    return v;
                } catch (NumberFormatException ex) {
                    throw new IllegalArgumentException("Invalid integer: " + t);
                }
            }
            case "boolean": {
                String t = textContentTrim(typeElem);
                if (!"0".equals(t) && !"1".equals(t)) {
                    throw new IllegalArgumentException("Invalid boolean: " + t);
                }
                return "1".equals(t);
            }
            case "string": {
                String s = textContentTrim(typeElem);
                return validateString(s);
            }
            default:
                throw new IllegalArgumentException("Unsupported XML-RPC type: " + tag);
        }
    }

    private static String buildValueXml(Object obj) {
        if (obj instanceof Integer) {
            return "<value><int>" + obj + "</int></value>";
        } else if (obj instanceof Boolean) {
            return "<value><boolean>" + (((Boolean) obj) ? "1" : "0") + "</boolean></value>";
        } else if (obj instanceof String) {
            return "<value><string>" + xmlEscape((String) obj) + "</string></value>";
        } else {
            // Fallback to string
            return "<value><string>" + xmlEscape(String.valueOf(obj)) + "</string></value>";
        }
    }

    private static byte[] buildSuccessResponse(Object result) {
        String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                + "<methodResponse><params><param>"
                + buildValueXml(result)
                + "</param></params></methodResponse>";
        return xml.getBytes(StandardCharsets.UTF_8);
    }

    private static byte[] buildFaultResponse(int code, String message) {
        String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                + "<methodResponse><fault><value><struct>"
                + "<member><name>faultCode</name><value><int>" + code + "</int></value></member>"
                + "<member><name>faultString</name><value><string>" + xmlEscape(message) + "</string></value></member>"
                + "</struct></value></fault></methodResponse>";
        return xml.getBytes(StandardCharsets.UTF_8);
    }

    // ===== Business logic with input validation =====

    private static String validateString(String s) {
        if (s == null) return "";
        if (s.length() > 1000) {
            throw new IllegalArgumentException("String too long");
        }
        return s;
    }

    private static Object dispatch(String methodName, List<Object> params) {
        switch (methodName) {
            case "add": {
                requireParamCount(methodName, params, 2);
                int a = asInt(params.get(0));
                int b = asInt(params.get(1));
                long sum = (long) a + (long) b;
                if (sum > Integer.MAX_VALUE || sum < Integer.MIN_VALUE) {
                    throw new IllegalArgumentException("Integer overflow");
                }
                return (int) sum;
            }
            case "concat": {
                requireParamCount(methodName, params, 2);
                String a = asString(params.get(0));
                String b = asString(params.get(1));
                String res = a + b;
                if (res.length() > 2000) {
                    throw new IllegalArgumentException("Resulting string too long");
                }
                return res;
            }
            case "factorial": {
                requireParamCount(methodName, params, 1);
                int n = asInt(params.get(0));
                if (n < 0 || n > 12) { // 13! exceeds int
                    throw new IllegalArgumentException("n must be 0..12");
                }
                int f = 1;
                for (int i = 2; i <= n; i++) f *= i;
                return f;
            }
            case "reverse": {
                requireParamCount(methodName, params, 1);
                String s = asString(params.get(0));
                StringBuilder sb = new StringBuilder(s);
                return sb.reverse().toString();
            }
            case "isEven": {
                requireParamCount(methodName, params, 1);
                int n = asInt(params.get(0));
                return (n % 2) == 0;
            }
            default:
                throw new IllegalArgumentException("Unknown method: " + methodName);
        }
    }

    private static void requireParamCount(String method, List<Object> params, int expected) {
        if (params.size() != expected) {
            throw new IllegalArgumentException("Method " + method + " expects " + expected + " params");
        }
    }

    private static int asInt(Object o) {
        if (!(o instanceof Integer)) throw new IllegalArgumentException("Expected int");
        return (Integer) o;
    }

    private static String asString(Object o) {
        if (!(o instanceof String)) throw new IllegalArgumentException("Expected string");
        return validateString((String) o);
    }

    // ===== HTTP Server =====

    private static class RpcHttpHandler implements HttpHandler {
        private static final int MAX_REQUEST_SIZE = 10 * 1024;

        @Override
        public void handle(HttpExchange exchange) {
            try {
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendBytes(exchange, 405, "text/plain; charset=utf-8", "Method Not Allowed".getBytes(StandardCharsets.UTF_8));
                    return;
                }
                Headers reqHeaders = exchange.getRequestHeaders();
                String contentType = reqHeaders.getFirst("Content-Type");
                if (contentType == null || !contentType.toLowerCase().contains("text/xml")) {
                    // Accept but enforce XML parse later
                }
                byte[] body = readRequestBody(exchange.getRequestBody(), reqHeaders.getFirst("Content-Length"), MAX_REQUEST_SIZE);
                RpcRequest req = parseXmlRpcRequest(body);
                Object result = dispatch(req.methodName, req.params);
                byte[] resp = buildSuccessResponse(result);
                sendBytes(exchange, 200, "text/xml; charset=utf-8", resp);
            } catch (IllegalArgumentException ex) {
                byte[] fault = buildFaultResponse(1, ex.getMessage() == null ? "Invalid request" : ex.getMessage());
                safeSend(exchange, 200, "text/xml; charset=utf-8", fault);
            } catch (Exception ex) {
                byte[] fault = buildFaultResponse(2, "Server error");
                safeSend(exchange, 200, "text/xml; charset=utf-8", fault);
            }
        }

        private static byte[] readRequestBody(InputStream in, String contentLengthHeader, int maxSize) throws Exception {
            int contentLength = -1;
            if (contentLengthHeader != null) {
                try {
                    contentLength = Integer.parseInt(contentLengthHeader.trim());
                } catch (NumberFormatException ignored) {}
            }
            try (InputStream bis = new BufferedInputStream(in);
                 ByteArrayOutputStream baos = new ByteArrayOutputStream()) {
                int remaining = maxSize;
                byte[] buf = new byte[4096];
                int read;
                while ((read = bis.read(buf, 0, Math.min(buf.length, remaining))) != -1) {
                    baos.write(buf, 0, read);
                    remaining -= read;
                    if (remaining <= 0) break;
                    if (contentLength >= 0 && baos.size() >= contentLength) break;
                }
                if (baos.size() > maxSize) {
                    throw new IllegalArgumentException("Request too large");
                }
                return baos.toByteArray();
            }
        }

        private static void sendBytes(HttpExchange exchange, int status, String contentType, byte[] data) throws Exception {
            Headers respHeaders = exchange.getResponseHeaders();
            respHeaders.set("Content-Type", contentType);
            respHeaders.set("X-Content-Type-Options", "nosniff");
            exchange.sendResponseHeaders(status, data.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(data);
                os.flush();
            }
        }

        private static void safeSend(HttpExchange exchange, int status, String contentType, byte[] data) {
            try {
                sendBytes(exchange, status, contentType, data);
            } catch (Exception ignored) {
            }
        }
    }

    private static class XmlRpcServer {
        private final HttpServer server;
        private final int port;

        XmlRpcServer(int port) throws Exception {
            InetAddress localhost = InetAddress.getByName("127.0.0.1");
            InetSocketAddress addr = new InetSocketAddress(localhost, port);
            server = HttpServer.create(addr, 0);
            server.createContext("/", new RpcHttpHandler());
            server.setExecutor(Executors.newFixedThreadPool(4));
            this.port = addr.getPort();
        }

        void start() {
            server.start();
        }

        void stop() {
            server.stop(0);
        }

        int getPort() {
            // When bound to a specific port, return that. If it was 0, HttpServer doesn't expose auto port clearly.
            // Our code chooses a concrete port beforehand, so this is accurate.
            return port;
        }
    }

    // ===== Client utilities for tests =====

    private static String buildXmlRpcRequest(String method, List<Object> params) {
        StringBuilder sb = new StringBuilder();
        sb.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        sb.append("<methodCall><methodName>").append(xmlEscape(method)).append("</methodName><params>");
        for (Object p : params) {
            sb.append("<param>").append(buildValueXml(p)).append("</param>");
        }
        sb.append("</params></methodCall>");
        return sb.toString();
    }

    private static Object parseXmlRpcResponse(byte[] data) throws Exception {
        Document doc = parseXmlSecurely(data);
        Element root = doc.getDocumentElement();
        if (root == null || !"methodResponse".equals(root.getTagName())) {
            throw new IllegalArgumentException("Invalid XML-RPC response");
        }
        Element fault = firstChildElement(root, "fault");
        if (fault != null) {
            throw new IllegalStateException("Fault");
        }
        Element params = firstChildElement(root, "params");
        if (params == null) throw new IllegalArgumentException("Invalid response: missing params");
        Element param = firstChildElement(params, "param");
        if (param == null) throw new IllegalArgumentException("Invalid response: missing param");
        Element value = firstChildElement(param, "value");
        if (value == null) throw new IllegalArgumentException("Invalid response: missing value");
        // Parse same as request values
        return parseValue(value);
    }

    private static Object callXmlRpc(int port, String method, List<Object> params) throws Exception {
        String xml = buildXmlRpcRequest(method, params);
        URL url = new URL("http://127.0.0.1:" + port + "/");
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setRequestMethod("POST");
        conn.setDoOutput(true);
        conn.setConnectTimeout(3000);
        conn.setReadTimeout(3000);
        conn.setRequestProperty("Content-Type", "text/xml; charset=utf-8");
        byte[] payload = xml.getBytes(StandardCharsets.UTF_8);
        conn.setFixedLengthStreamingMode(payload.length);
        try (OutputStream os = conn.getOutputStream()) {
            os.write(payload);
        }
        int code = conn.getResponseCode();
        InputStream is = (code >= 200 && code < 300) ? conn.getInputStream() : conn.getErrorStream();
        byte[] resp;
        try (InputStream in = new BufferedInputStream(is);
             ByteArrayOutputStream baos = new ByteArrayOutputStream()) {
            byte[] buf = new byte[4096];
            int r;
            while ((r = in.read(buf)) != -1) baos.write(buf, 0, r);
            resp = baos.toByteArray();
        }
        return parseXmlRpcResponse(resp);
    }

    // ===== Main with 5 test cases =====
    public static void main(String[] args) throws Exception {
        // Choose an available port securely
        int port;
        try (ServerSocket ss = new ServerSocket(0, 50, InetAddress.getByName("127.0.0.1"))) {
            port = ss.getLocalPort();
        }
        XmlRpcServer server = new XmlRpcServer(port);
        server.start();

        try {
            Object r1 = callXmlRpc(server.getPort(), "add", List.of(5, 7));
            System.out.println("add(5,7) = " + r1);

            Object r2 = callXmlRpc(server.getPort(), "concat", List.of("Hello, ", "World"));
            System.out.println("concat = " + r2);

            Object r3 = callXmlRpc(server.getPort(), "factorial", List.of(5));
            System.out.println("factorial(5) = " + r3);

            Object r4 = callXmlRpc(server.getPort(), "reverse", List.of("abcd"));
            System.out.println("reverse = " + r4);

            Object r5 = callXmlRpc(server.getPort(), "isEven", List.of(8));
            System.out.println("isEven(8) = " + r5);
        } finally {
            server.stop();
        }
    }
}