
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.regex.*;

class Task111 {
    private static final int PORT = 8080;
    private static Map<String, Object> methods = new HashMap<>();
    
    static {
        methods.put("add", (Object[] params) -> {
            int a = (Integer) params[0];
            int b = (Integer) params[1];
            return a + b;
        });
        
        methods.put("subtract", (Object[] params) -> {
            int a = (Integer) params[0];
            int b = (Integer) params[1];
            return a - b;
        });
        
        methods.put("multiply", (Object[] params) -> {
            int a = (Integer) params[0];
            int b = (Integer) params[1];
            return a * b;
        });
        
        methods.put("greet", (Object[] params) -> {
            String name = (String) params[0];
            return "Hello, " + name + "!";
        });
        
        methods.put("concat", (Object[] params) -> {
            String a = (String) params[0];
            String b = (String) params[1];
            return a + b;
        });
    }
    
    interface RPCMethod {
        Object execute(Object[] params);
    }
    
    public static void startServer() {
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            System.out.println("XML-RPC Server started on port " + PORT);
            
            while (true) {
                Socket clientSocket = serverSocket.accept();
                handleClient(clientSocket);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    private static void handleClient(Socket clientSocket) {
        try (BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
             PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true)) {
            
            StringBuilder request = new StringBuilder();
            String line;
            int contentLength = 0;
            
            while ((line = in.readLine()) != null && !line.isEmpty()) {
                if (line.startsWith("Content-Length:")) {
                    contentLength = Integer.parseInt(line.substring(15).trim());
                }
            }
            
            if (contentLength > 0) {
                char[] body = new char[contentLength];
                in.read(body, 0, contentLength);
                request.append(new String(body));
            }
            
            String response = processXMLRPC(request.toString());
            
            out.println("HTTP/1.1 200 OK");
            out.println("Content-Type: text/xml");
            out.println("Content-Length: " + response.length());
            out.println();
            out.println(response);
            
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    private static String processXMLRPC(String xmlRequest) {
        try {
            String methodName = extractMethodName(xmlRequest);
            Object[] params = extractParams(xmlRequest);
            
            if (methods.containsKey(methodName)) {
                RPCMethod method = (RPCMethod) methods.get(methodName);
                Object result = method.execute(params);
                return buildSuccessResponse(result);
            } else {
                return buildFaultResponse(-1, "Method not found: " + methodName);
            }
        } catch (Exception e) {
            return buildFaultResponse(-2, "Error processing request: " + e.getMessage());
        }
    }
    
    private static String extractMethodName(String xml) {
        Pattern pattern = Pattern.compile("<methodName>(.*?)</methodName>");
        Matcher matcher = pattern.matcher(xml);
        if (matcher.find()) {
            return matcher.group(1);
        }
        return "";
    }
    
    private static Object[] extractParams(String xml) {
        List<Object> params = new ArrayList<>();
        Pattern paramPattern = Pattern.compile("<param>.*?<value>(.*?)</value>.*?</param>", Pattern.DOTALL);
        Matcher matcher = paramPattern.matcher(xml);
        
        while (matcher.find()) {
            String valueContent = matcher.group(1);
            if (valueContent.contains("<int>") || valueContent.contains("<i4>")) {
                Pattern intPattern = Pattern.compile("<(?:int|i4)>(.*?)</(?:int|i4)>");
                Matcher intMatcher = intPattern.matcher(valueContent);
                if (intMatcher.find()) {
                    params.add(Integer.parseInt(intMatcher.group(1)));
                }
            } else if (valueContent.contains("<string>")) {
                Pattern stringPattern = Pattern.compile("<string>(.*?)</string>");
                Matcher stringMatcher = stringPattern.matcher(valueContent);
                if (stringMatcher.find()) {
                    params.add(stringMatcher.group(1));
                }
            } else {
                params.add(valueContent.trim());
            }
        }
        
        return params.toArray();
    }
    
    private static String buildSuccessResponse(Object result) {
        StringBuilder response = new StringBuilder();
        response.append("<?xml version=\\"1.0\\"?>\\n");
        response.append("<methodResponse>\\n");
        response.append("  <params>\\n");
        response.append("    <param>\\n");
        response.append("      <value>");
        
        if (result instanceof Integer) {
            response.append("<int>").append(result).append("</int>");
        } else if (result instanceof String) {
            response.append("<string>").append(result).append("</string>");
        } else {
            response.append(result);
        }
        
        response.append("</value>\\n");
        response.append("    </param>\\n");
        response.append("  </params>\\n");
        response.append("</methodResponse>");
        
        return response.toString();
    }
    
    private static String buildFaultResponse(int faultCode, String faultString) {
        return "<?xml version=\\"1.0\\"?>\\n" +
               "<methodResponse>\\n" +
               "  <fault>\\n" +
               "    <value>\\n" +
               "      <struct>\\n" +
               "        <member>\\n" +
               "          <name>faultCode</name>\\n" +
               "          <value><int>" + faultCode + "</int></value>\\n" +
               "        </member>\\n" +
               "        <member>\\n" +
               "          <name>faultString</name>\\n" +
               "          <value><string>" + faultString + "</string></value>\\n" +
               "        </member>\\n" +
               "      </struct>\\n" +
               "    </value>\\n" +
               "  </fault>\\n" +
               "</methodResponse>";
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Process add request");
        String request1 = "<?xml version=\\"1.0\\"?><methodCall><methodName>add</methodName><params><param><value><int>5</int></value></param><param><value><int>3</int></value></param></params></methodCall>";
        System.out.println(processXMLRPC(request1));
        
        System.out.println("\\nTest Case 2: Process subtract request");
        String request2 = "<?xml version=\\"1.0\\"?><methodCall><methodName>subtract</methodName><params><param><value><int>10</int></value></param><param><value><int>4</int></value></param></params></methodCall>";
        System.out.println(processXMLRPC(request2));
        
        System.out.println("\\nTest Case 3: Process multiply request");
        String request3 = "<?xml version=\\"1.0\\"?><methodCall><methodName>multiply</methodName><params><param><value><int>6</int></value></param><param><value><int>7</int></value></param></params></methodCall>";
        System.out.println(processXMLRPC(request3));
        
        System.out.println("\\nTest Case 4: Process greet request");
        String request4 = "<?xml version=\\"1.0\\"?><methodCall><methodName>greet</methodName><params><param><value><string>Alice</string></value></param></params></methodCall>";
        System.out.println(processXMLRPC(request4));
        
        System.out.println("\\nTest Case 5: Process invalid method request");
        String request5 = "<?xml version=\\"1.0\\"?><methodCall><methodName>invalidMethod</methodName><params></params></methodCall>";
        System.out.println(processXMLRPC(request5));
    }
}
