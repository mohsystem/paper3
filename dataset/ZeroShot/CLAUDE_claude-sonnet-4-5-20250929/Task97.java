
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathFactory;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import java.io.File;

public class Task97 {
    public static String executeXPathQuery(String xpathValue, String xmlFileName) {
        try {
            // Validate inputs
            if (xpathValue == null || xpathValue.trim().isEmpty()) {
                return "Error: XPath value cannot be empty";
            }
            if (xmlFileName == null || xmlFileName.trim().isEmpty()) {
                return "Error: XML file name cannot be empty";
            }
            
            // Sanitize xpath value to prevent injection
            String sanitizedXPath = xpathValue.replaceAll("[^a-zA-Z0-9_-]", "");
            
            // Read and parse XML file
            File xmlFile = new File(xmlFileName);
            if (!xmlFile.exists() || !xmlFile.isFile()) {
                return "Error: XML file not found or invalid";
            }
            
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            // Security features
            factory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            factory.setFeature("http://xml.org/sax/features/external-general-entities", false);
            factory.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            factory.setFeature("http://apache.org/xml/features/nonvalidating/load-external-dtd", false);
            factory.setXIncludeAware(false);
            factory.setExpandEntityReferences(false);
            
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(xmlFile);
            document.getDocumentElement().normalize();
            
            // Execute XPath query
            XPathFactory xpathFactory = XPathFactory.newInstance();
            XPath xpath = xpathFactory.newXPath();
            
            String xpathExpression = "//*[@id='" + sanitizedXPath + "']";
            NodeList nodeList = (NodeList) xpath.evaluate(xpathExpression, document, XPathConstants.NODESET);
            
            StringBuilder result = new StringBuilder();
            for (int i = 0; i < nodeList.getLength(); i++) {
                result.append(nodeList.item(i).getNodeName())
                      .append(": ")
                      .append(nodeList.item(i).getTextContent())
                      .append("\\n");
            }
            
            return result.length() > 0 ? result.toString().trim() : "No results found";
            
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        System.out.println(executeXPathQuery("test1", "test.xml"));
        System.out.println();
        
        System.out.println("Test Case 2:");
        System.out.println(executeXPathQuery("test2", "test.xml"));
        System.out.println();
        
        System.out.println("Test Case 3:");
        System.out.println(executeXPathQuery("", "test.xml"));
        System.out.println();
        
        System.out.println("Test Case 4:");
        System.out.println(executeXPathQuery("test1", "nonexistent.xml"));
        System.out.println();
        
        System.out.println("Test Case 5:");
        System.out.println(executeXPathQuery("invalid-id", "test.xml"));
    }
}
