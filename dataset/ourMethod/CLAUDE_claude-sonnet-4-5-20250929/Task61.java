
import java.io.ByteArrayInputStream;
import java.io.StringReader;
import java.nio.charset.StandardCharsets;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;

public class Task61 {
    
    public static String parseAndGetRootElement(String xmlString) {
        if (xmlString == null || xmlString.trim().isEmpty()) {
            throw new IllegalArgumentException("XML string cannot be null or empty");
        }
        
        if (xmlString.length() > 1000000) {
            throw new IllegalArgumentException("XML string exceeds maximum allowed length");
        }
        
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            
            // Disable external entity processing to prevent XXE attacks
            factory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            factory.setFeature("http://xml.org/sax/features/external-general-entities", false);
            factory.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            factory.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
            factory.setXIncludeAware(false);
            factory.setExpandEntityReferences(false);
            
            DocumentBuilder builder = factory.newDocumentBuilder();
            
            ByteArrayInputStream inputStream = new ByteArrayInputStream(
                xmlString.getBytes(StandardCharsets.UTF_8)
            );
            
            Document document = builder.parse(inputStream);
            document.getDocumentElement().normalize();
            
            Element rootElement = document.getDocumentElement();
            
            if (rootElement == null) {
                return "";
            }
            
            return rootElement.getNodeName();
            
        } catch (Exception e) {
            throw new RuntimeException("Failed to parse XML: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple XML
        try {
            String xml1 = "<root><child>data</child></root>";
            System.out.println("Test 1: " + parseAndGetRootElement(xml1));
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: XML with attributes
        try {
            String xml2 = "<document type=\\"test\\"><section>content</section></document>";
            System.out.println("Test 2: " + parseAndGetRootElement(xml2));
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: Nested XML
        try {
            String xml3 = "<library><books><book><title>Title</title></book></books></library>";
            System.out.println("Test 3: " + parseAndGetRootElement(xml3));
        } catch (Exception e) {
            System.out.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Empty string (should fail)
        try {
            String xml4 = "";
            System.out.println("Test 4: " + parseAndGetRootElement(xml4));
        } catch (Exception e) {
            System.out.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test case 5: XML with namespace
        try {
            String xml5 = "<ns:root xmlns:ns=\\"http://example.com\\"><ns:child>data</ns:child></ns:root>";
            System.out.println("Test 5: " + parseAndGetRootElement(xml5));
        } catch (Exception e) {
            System.out.println("Test 5 failed: " + e.getMessage());
        }
    }
}
