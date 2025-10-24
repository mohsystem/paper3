import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.LinkedHashMap;
import java.util.concurrent.Executors;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class Task111 {

    private static final int PORT = 18080;
    private static final String HOST = "127.0.0.1";
    private static final int MAX_BODY_BYTES = 1_048_576; // 1MB
    private static final List<String> ALLOWED_METHODS;
    static {
        List<String> m = new ArrayList<>();
        m.add("sum");
        m.add("multiply");
        m.add("concat");
        m.add("ping");
        m.add("echo");
        m.add("system.listMethods");
        ALLOWED_METHODS = Collections.unmodifiableList(m);
    }

    private static HttpServer server;

    // RPC methods
    public static int sum(int[] arr) {
        long total = 0L;
        if (arr != null) {
            for (int v : arr) {
                total += v;
                if (total > Integer.MAX_VALUE || total < Integer.MIN_VALUE) {
                    throw new IllegalArgumentException("Sum overflow");
                }
            }
        }
        return (int) total;
    }

    public static int multiply(int a, int b) {
        long prod = (long) a * (long) b;
        if (prod > Integer.MAX_VALUE || prod < Integer.MIN_VALUE) {
            throw new IllegalArgumentException("Multiply overflow");
        }
        return (int) prod;
    }

    public static String concat(String a, String b) {
        if (a == null || b == null) {
            throw new IllegalArgumentException("Null string");
        }
        if (a.length() + b.length() > 1_000_000) {
            throw new IllegalArgumentException("String too long");
        }
        return a + b;
    }

    public static String ping() {
        return "pong";
    }

    public static String echo(String s) {
        if (s == null) return "";
        if (s.length() > 1_000_000) {
            throw new IllegalArgumentException("String too long");
        }
        return s;
    }

    private static void startServer() throws IOException {
        server = HttpServer.create(new InetSocketAddress(HOST, PORT), 0);
        server.createContext("/RPC2", new RpcHandler());
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
    }

    private static void stopServer() {
        if (server != null) {
            server.stop(0);
        }
    }

    // Simple XML-RPC HTTP handler
    static class RpcHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!exchange.getRequestMethod().equalsIgnoreCase("POST")) {
                    sendStatus(exchange, 405, "Only POST allowed");
                    return;
                }
                Headers reqHeaders = exchange.getRequestHeaders();
                String ctype = reqHeaders.getFirst("Content-Type");
                if (ctype == null || !ctype.toLowerCase().contains("text/xml")) {
                    // Continue to parse anyway per XML-RPC leniency, but we can gate here if desired
                }
                byte[] body = readRequestBody(exchange.getRequestBody(), MAX_BODY_BYTES);
                if (body == null) {
                    sendStatus(exchange, 413, "Request too large");
                    return;
                }
                Document doc;
                try {
                    doc = parseXmlSecure(body);
                } catch (Exception e) {
                    sendXmlFault(exchange, 400, "Malformed XML");
                    return;
                }
                String methodName = getTextContent(doc, "methodName");
                if (methodName == null || methodName.isEmpty()) {
                    sendXmlFault(exchange, 400, "Missing methodName");
                    return;
                }
                if (!ALLOWED_METHODS.contains(methodName)) {
                    sendXmlFault(exchange, 403, "Method not allowed");
                    return;
                }

                List<Object> params = parseParams(doc);
                Object result;
                try {
                    result = dispatchMethod(methodName, params);
                } catch (IllegalArgumentException ex) {
                    sendXmlFault(exchange, 422, xmlEscape(ex.getMessage()));
                    return;
                } catch (Exception ex) {
                    sendXmlFault(exchange, 500, "Server error");
                    return;
                }

                String resp = buildSuccessResponse(result);
                byte[] respBytes = resp.getBytes(StandardCharsets.UTF_8);
                Headers headers = exchange.getResponseHeaders();
                headers.set("Content-Type", "text/xml; charset=utf-8");
                exchange.sendResponseHeaders(200, respBytes.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(respBytes);
                }
            } catch (Exception e) {
                try {
                    sendXmlFault(exchange, 500, "Unhandled server error");
                } catch (Exception ignore) {
                }
            }
        }

        private byte[] readRequestBody(InputStream is, int max) throws IOException {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] buf = new byte[8192];
            int r;
            int total = 0;
            while ((r = is.read(buf)) != -1) {
                total += r;
                if (total > max) {
                    return null;
                }
                baos.write(buf, 0, r);
            }
            return baos.toByteArray();
        }

        private Document parseXmlSecure(byte[] xmlBytes) throws Exception {
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            dbf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
            dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
            dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            dbf.setXIncludeAware(false);
            dbf.setExpandEntityReferences(false);
            dbf.setNamespaceAware(false);
            DocumentBuilder db = dbf.newDocumentBuilder();
            try (ByteArrayInputStream bais = new ByteArrayInputStream(xmlBytes)) {
                return db.parse(bais);
            }
        }

        private String getTextContent(Document doc, String tagName) {
            NodeList nl = doc.getElementsByTagName(tagName);
            if (nl.getLength() == 0) return null;
            return nl.item(0).getTextContent();
        }

        private List<Object> parseParams(Document doc) {
            NodeList paramsList = doc.getElementsByTagName("params");
            List<Object> out = new ArrayList<>();
            if (paramsList.getLength() == 0) return out;
            Element paramsEl = (Element) paramsList.item(0);
            NodeList paramNodes = paramsEl.getElementsByTagName("param");
            for (int i = 0; i < paramNodes.getLength(); i++) {
                Element paramEl = (Element) paramNodes.item(i);
                NodeList vList = paramEl.getElementsByTagName("value");
                if (vList.getLength() > 0) {
                    out.add(parseValue((Element) vList.item(0)));
                } else {
                    out.add(null);
                }
            }
            return out;
        }

        private Object parseValue(Element valueEl) {
            Element child = firstElementChild(valueEl);
            if (child == null) {
                // Bare value treated as string
                return valueEl.getTextContent();
            }
            String tag = child.getTagName();
            switch (tag) {
                case "string":
                    return child.getTextContent();
                case "int":
                case "i4":
                    return Integer.parseInt(child.getTextContent().trim());
                case "i8":
                    return Long.parseLong(child.getTextContent().trim());
                case "double":
                    return Double.parseDouble(child.getTextContent().trim());
                case "boolean": {
                    String t = child.getTextContent().trim();
                    return "1".equals(t) || "true".equalsIgnoreCase(t);
                }
                case "array": {
                    Element data = firstElementByTag(child, "data");
                    List<Object> list = new ArrayList<>();
                    if (data != null) {
                        NodeList vals = data.getElementsByTagName("value");
                        for (int i = 0; i < vals.getLength(); i++) {
                            list.add(parseValue((Element) vals.item(i)));
                        }
                    }
                    return list;
                }
                case "struct": {
                    Map<String, Object> map = new LinkedHashMap<>();
                    NodeList members = child.getElementsByTagName("member");
                    for (int i = 0; i < members.getLength(); i++) {
                        Element mem = (Element) members.item(i);
                        String name = null;
                        Object val = null;
                        NodeList nameNodes = mem.getElementsByTagName("name");
                        if (nameNodes.getLength() > 0) {
                            name = nameNodes.item(0).getTextContent();
                        }
                        NodeList valueNodes = mem.getElementsByTagName("value");
                        if (valueNodes.getLength() > 0) {
                            val = parseValue((Element) valueNodes.item(0));
                        }
                        if (name != null) {
                            map.put(name, val);
                        }
                    }
                    return map;
                }
                case "dateTime.iso8601":
                case "base64":
                default:
                    return child.getTextContent();
            }
        }

        private Element firstElementChild(Element e) {
            Node n = e.getFirstChild();
            while (n != null) {
                if (n.getNodeType() == Node.ELEMENT_NODE) return (Element) n;
                n = n.getNextSibling();
            }
            return null;
        }

        private Element firstElementByTag(Element e, String tag) {
            NodeList nl = e.getElementsByTagName(tag);
            if (nl.getLength() == 0) return null;
            return (Element) nl.item(0);
        }

        private Object dispatchMethod(String method, List<Object> params) {
            switch (method) {
                case "system.listMethods":
                    return new ArrayList<>(ALLOWED_METHODS);
                case "sum": {
                    // Accept either one array param or varargs of ints
                    int[] arr;
                    if (params.size() == 1 && params.get(0) instanceof List) {
                        List<?> lst = (List<?>) params.get(0);
                        arr = new int[lst.size()];
                        for (int i = 0; i < lst.size(); i++) {
                            Object v = lst.get(i);
                            if (v instanceof Number) {
                                arr[i] = ((Number) v).intValue();
                            } else {
                                throw new IllegalArgumentException("Non-numeric in array");
                            }
                        }
                    } else {
                        arr = new int[params.size()];
                        for (int i = 0; i < params.size(); i++) {
                            Object v = params.get(i);
                            if (v instanceof Number) {
                                arr[i] = ((Number) v).intValue();
                            } else {
                                throw new IllegalArgumentException("Non-numeric param");
                            }
                        }
                    }
                    return sum(arr);
                }
                case "multiply": {
                    if (params.size() != 2) throw new IllegalArgumentException("Need 2 params");
                    int a = toInt(params.get(0));
                    int b = toInt(params.get(1));
                    return multiply(a, b);
                }
                case "concat": {
                    if (params.size() != 2) throw new IllegalArgumentException("Need 2 params");
                    String a = toStr(params.get(0));
                    String b = toStr(params.get(1));
                    return concat(a, b);
                }
                case "ping": {
                    if (!params.isEmpty()) throw new IllegalArgumentException("No params expected");
                    return ping();
                }
                case "echo": {
                    if (params.size() != 1) throw new IllegalArgumentException("Need 1 param");
                    return echo(toStr(params.get(0)));
                }
                default:
                    throw new IllegalArgumentException("Unknown method");
            }
        }

        private int toInt(Object o) {
            if (o instanceof Number) return ((Number) o).intValue();
            if (o instanceof String) return Integer.parseInt(((String) o).trim());
            throw new IllegalArgumentException("Expected integer");
        }

        private String toStr(Object o) {
            if (o == null) return "";
            return String.valueOf(o);
        }

        private void sendStatus(HttpExchange ex, int code, String msg) throws IOException {
            byte[] out = msg.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
            ex.sendResponseHeaders(code, out.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(out);
            }
        }

        private void sendXmlFault(HttpExchange ex, int faultCode, String faultString) throws IOException {
            String xml = "<?xml version=\"1.0\"?>"
                    + "<methodResponse>"
                    + "<fault>"
                    + "<value><struct>"
                    + "<member><name>faultCode</name><value><int>" + faultCode + "</int></value></member>"
                    + "<member><name>faultString</name><value><string>" + xmlEscape(faultString) + "</string></value></member>"
                    + "</struct></value>"
                    + "</fault>"
                    + "</methodResponse>";
            byte[] bytes = xml.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().set("Content-Type", "text/xml; charset=utf-8");
            ex.sendResponseHeaders(200, bytes.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(bytes);
            }
        }

        private String buildSuccessResponse(Object result) {
            StringBuilder sb = new StringBuilder();
            sb.append("<?xml version=\"1.0\"?>");
            sb.append("<methodResponse><params><param><value>");
            sb.append(buildValueXml(result));
            sb.append("</value></param></params></methodResponse>");
            return sb.toString();
        }

        private String buildValueXml(Object v) {
            if (v == null) {
                return "<nil/>";
            }
            if (v instanceof Integer || v instanceof Short || v instanceof Byte) {
                return "<int>" + v.toString() + "</int>";
            }
            if (v instanceof Long) {
                long l = (Long) v;
                if (l >= Integer.MIN_VALUE && l <= Integer.MAX_VALUE) {
                    return "<int>" + l + "</int>";
                } else {
                    return "<i8>" + l + "</i8>";
                }
            }
            if (v instanceof Double || v instanceof Float) {
                double d = ((Number) v).doubleValue();
                return "<double>" + Double.toString(d) + "</double>";
            }
            if (v instanceof Boolean) {
                return "<boolean>" + (((Boolean) v) ? "1" : "0") + "</boolean>";
            }
            if (v instanceof String) {
                return "<string>" + xmlEscape((String) v) + "</string>";
            }
            if (v instanceof List) {
                StringBuilder sb = new StringBuilder();
                sb.append("<array><data>");
                for (Object item : (List<?>) v) {
                    sb.append("<value>").append(buildValueXml(item)).append("</value>");
                }
                sb.append("</data></array>");
                return sb.toString();
            }
            if (v.getClass().isArray()) {
                StringBuilder sb = new StringBuilder();
                sb.append("<array><data>");
                Object[] arr = toObjectArray(v);
                for (Object item : arr) {
                    sb.append("<value>").append(buildValueXml(item)).append("</value>");
                }
                sb.append("</data></array>");
                return sb.toString();
            }
            // Fallback to string
            return "<string>" + xmlEscape(v.toString()) + "</string>";
        }

        private Object[] toObjectArray(Object array) {
            if (array instanceof Object[]) return (Object[]) array;
            int len = java.lang.reflect.Array.getLength(array);
            Object[] out = new Object[len];
            for (int i = 0; i < len; i++) {
                out[i] = java.lang.reflect.Array.get(array, i);
            }
            return out;
        }
    }

    private static String xmlEscape(String s) {
        StringBuilder sb = new StringBuilder(s.length() + 16);
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&apos;"); break;
                default:
                    if (c < 0x20 && c != '\n' && c != '\r' && c != '\t') {
                        // skip control chars
                    } else {
                        sb.append(c);
                    }
            }
        }
        return sb.toString();
    }

    private static Object call(String method, Object[] params) throws Exception {
        String xmlReq = buildMethodCall(method, params);
        byte[] payload = xmlReq.getBytes(StandardCharsets.UTF_8);
        URL url = new URL("http://" + HOST + ":" + PORT + "/RPC2");
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setConnectTimeout(3000);
        conn.setReadTimeout(5000);
        conn.setRequestMethod("POST");
        conn.setDoOutput(true);
        conn.setRequestProperty("Content-Type", "text/xml; charset=utf-8");
        conn.setRequestProperty("Content-Length", Integer.toString(payload.length));
        try (OutputStream os = conn.getOutputStream()) {
            os.write(payload);
        }
        int code = conn.getResponseCode();
        InputStream is = (code >= 200 && code < 300) ? conn.getInputStream() : conn.getErrorStream();
        byte[] resp = readAll(is, 2_000_000);
        Document doc = new RpcHandler().parseXmlSecure(resp);
        Element root = doc.getDocumentElement(); // methodResponse
        Element child = firstElementChild(root);
        if (child == null) throw new IOException("Invalid response");
        if ("fault".equals(child.getTagName())) {
            // parse fault
            NodeList ints = doc.getElementsByTagName("int");
            NodeList strs = doc.getElementsByTagName("string");
            String msg = "Fault";
            if (strs.getLength() > 0) msg = strs.item(0).getTextContent();
            throw new IOException("XML-RPC Fault: " + msg);
        } else if ("params".equals(child.getTagName())) {
            NodeList paramNodes = child.getElementsByTagName("param");
            if (paramNodes.getLength() == 0) return null;
            Element valEl = (Element) ((Element) paramNodes.item(0)).getElementsByTagName("value").item(0);
            return new RpcHandler().parseValue(valEl);
        } else {
            throw new IOException("Unknown response");
        }
    }

    private static Element firstElementChild(Element e) {
        Node n = e.getFirstChild();
        while (n != null) {
            if (n.getNodeType() == Node.ELEMENT_NODE) return (Element) n;
            n = n.getNextSibling();
        }
        return null;
    }

    private static byte[] readAll(InputStream is, int max) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buf = new byte[8192];
        int r;
        int total = 0;
        while ((r = is.read(buf)) != -1) {
            total += r;
            if (total > max) throw new IOException("Response too large");
            baos.write(buf, 0, r);
        }
        return baos.toByteArray();
    }

    private static String buildMethodCall(String method, Object[] params) {
        StringBuilder sb = new StringBuilder();
        sb.append("<?xml version=\"1.0\"?>");
        sb.append("<methodCall>");
        sb.append("<methodName>").append(xmlEscape(method)).append("</methodName>");
        sb.append("<params>");
        if (params != null) {
            for (Object p : params) {
                sb.append("<param><value>").append(new RpcHandler().buildValueXml(p)).append("</value></param>");
            }
        }
        sb.append("</params>");
        sb.append("</methodCall>");
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        startServer();
        Thread.sleep(200);
        Object r1 = call("sum", new Object[]{ new int[]{1,2,3,4,5} });
        Object r2 = call("multiply", new Object[]{ 6, 7 });
        Object r3 = call("concat", new Object[]{ "Hello, ", "World!" });
        Object r4 = call("ping", new Object[]{} );
        Object r5 = call("echo", new Object[]{ "SampleText" });

        System.out.println(r1);
        System.out.println(r2);
        System.out.println(r3);
        System.out.println(r4);
        System.out.println(r5);

        stopServer();
    }
}