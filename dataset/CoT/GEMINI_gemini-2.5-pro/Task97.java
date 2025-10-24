import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpression;
import javax.xml.xpath.XPathFactory;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import javax.xml.XMLConstants;

public class Task97 {

    /**
     * Executes an XPath query on a given XML file.
     *
     * @param xmlFilePath The path to the XML file.
     * @param xpathExpression The XPath expression to execute.
     * @return A list of strings representing the query result.
     */
    public static List<String> executeXPath(String xmlFilePath, String xpathExpression) {
        List<String> results = new ArrayList<>();
        try {
            File xmlFile = new File(xmlFilePath);
            if (!xmlFile.exists()) {
                System.err.println("Error: XML file not found at " + xmlFilePath);
                return results;
            }

            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            
            // Secure processing: Disable DTDs and external entities to prevent XXE
            factory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
            factory.setFeature("http://xml.org/sax/features/external-general-entities", false);
            factory.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
            factory.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
            factory.setXIncludeAware(false);
            factory.setExpandEntityReferences(false);
            
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document doc = builder.parse(xmlFile);

            XPathFactory xPathfactory = XPathFactory.newInstance();
            XPath xpath = xPathfactory.newXPath();
            XPathExpression expr = xpath.compile(xpathExpression);

            NodeList nl = (NodeList) expr.evaluate(doc, XPathConstants.NODESET);

            for (int i = 0; i < nl.getLength(); i++) {
                results.add(nl.item(i).getTextContent());
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
        return results;
    }
    
    // Main method with 5 test cases
    public static void main(String[] args) {
        // Create a dummy XML file for testing
        String xmlFileName = "test97.xml";
        try (FileWriter writer = new FileWriter(xmlFileName)) {
            writer.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            writer.write("<bookstore>\n");
            writer.write("  <book id=\"1\">\n");
            writer.write("    <title lang=\"en\">Harry Potter</title>\n");
            writer.write("    <author>J. K. Rowling</author>\n");
            writer.write("  </book>\n");
            writer.write("  <book id=\"2\">\n");
            writer.write("    <title lang=\"en\">Learning XML</title>\n");
            writer.write("    <author>Erik T. Ray</author>\n");
            writer.write("  </book>\n");
             writer.write("  <novel id=\"3\">\n");
            writer.write("    <title lang=\"en\">The Great Gatsby</title>\n");
            writer.write("    <author>F. Scott Fitzgerald</author>\n");
            writer.write("  </novel>\n");
            writer.write("</bookstore>");
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }

        // Simulating command line arguments: xpath, filename
        String[][] testCases = {
            {"/bookstore/book/title", xmlFileName}, // Test Case 1: Get all book titles
            {"/bookstore/book[@id='1']/title", xmlFileName}, // Test Case 2: Get title of book with id=1
            {"//author", xmlFileName}, // Test Case 3: Get all authors
            {"/bookstore/novel/title", xmlFileName}, // Test Case 4: Get novel titles
            {"/bookstore/book[@id='99']/title", xmlFileName} // Test Case 5: Query for non-existent node
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            String xpath = testCases[i][0];
            String filename = testCases[i][1];
            System.out.println("Executing XPath: " + xpath + " on " + filename);
            
            List<String> results = executeXPath(filename, xpath);
            
            System.out.println("Result: " + results);
            System.out.println();
        }

        // Clean up the dummy file
        new File(xmlFileName).delete();
    }
}