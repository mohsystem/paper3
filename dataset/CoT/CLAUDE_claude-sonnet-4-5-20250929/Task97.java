
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathFactory;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Task97 {
    
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    
    public static String executeXPathQuery(String xpathValue, String xmlFileName) {
        try {
            // Validate inputs
            if (xpathValue == null || xpathValue.trim().isEmpty()) {
                return "Error: XPath value is required";
            }
            if (xmlFileName == null || xmlFileName.trim().isEmpty()) {
                return "Error: XML filename is required";
            }
            
            // Sanitize XPath input - prevent injection
            String sanitizedXPath = sanitizeXPath(xpathValue);
            
            // Validate file path - prevent path traversal
            Path filePath = Paths.get(xmlFileName).normalize();
            File xmlFile = filePath.toFile();
            
            if (!xmlFile.exists()) {
                return "Error: File does not exist";
            }
            
            if (!xmlFile.isFile()) {
                return "Error: Path is not a file";
            }
            
            // Check file size
            if (Files.size(filePath) > MAX_FILE_SIZE) {
                return "Error: File size exceeds maximum allowed size";
            }
            
            // Secure XML parsing - disable XXE
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            factory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            factory.setFeature("http://xml.org/sax/features/external-general-entities", false);
            factory.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            factory.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
            factory.setXIncludeAware(false);
            factory.setExpandEntityReferences(false);
            
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(xmlFile);
            document.getDocumentElement().normalize();
            
            // Construct XPath query
            String xpathQuery = "/tag[@id='" + sanitizedXPath + "']";
            
            // Execute XPath query
            XPathFactory xpathFactory = XPathFactory.newInstance();
            XPath xpath = xpathFactory.newXPath();
            NodeList nodeList = (NodeList) xpath.evaluate(xpathQuery, document, XPathConstants.NODESET);
            
            // Build result
            StringBuilder result = new StringBuilder();
            for (int i = 0; i < nodeList.getLength(); i++) {
                result.append("Node ").append(i + 1).append(": ")
                      .append(nodeList.item(i).getTextContent()).append("\\n");
            }
            
            return result.length() > 0 ? result.toString() : "No matching nodes found";
            
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    private static String sanitizeXPath(String input) {
        // Remove potentially dangerous characters for XPath injection
        return input.replaceAll("[^a-zA-Z0-9_-]", "");
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("=== Test Case 1: Valid input ===");
        System.out.println(executeXPathQuery("test1", "test.xml"));
        
        System.out.println("\\n=== Test Case 2: Empty XPath ===");
        System.out.println(executeXPathQuery("", "test.xml"));
        
        System.out.println("\\n=== Test Case 3: Non-existent file ===");
        System.out.println(executeXPathQuery("test2", "nonexistent.xml"));
        
        System.out.println("\\n=== Test Case 4: Null inputs ===");
        System.out.println(executeXPathQuery(null, "test.xml"));
        
        System.out.println("\\n=== Test Case 5: Special characters in XPath ===");
        System.out.println(executeXPathQuery("test'OR'1'='1", "test.xml"));
        
        // Command line usage
        if (args.length == 2) {
            System.out.println("\\n=== Command Line Execution ===");
            System.out.println(executeXPathQuery(args[0], args[1]));
        }
    }
}
