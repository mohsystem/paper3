
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpression;
import javax.xml.xpath.XPathFactory;
import org.w3c.dom.Document;
import org.w3c.dom.NodeList;
import java.io.File;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Task97 {
    private static final String BASE_DIRECTORY = System.getProperty("user.dir");
    private static final int MAX_XPATH_LENGTH = 1000;
    private static final int MAX_FILENAME_LENGTH = 255;

    public static String executeXPathQuery(String xpathValue, String xmlFileName) {
        if (xpathValue == null || xpathValue.isEmpty() || xpathValue.length() > MAX_XPATH_LENGTH) {
            return "Error: Invalid XPath value";
        }
        if (xmlFileName == null || xmlFileName.isEmpty() || xmlFileName.length() > MAX_FILENAME_LENGTH) {
            return "Error: Invalid file name";
        }

        if (!xmlFileName.matches("^[a-zA-Z0-9_\\\\-\\\\.]+\\\\.xml$")) {
            return "Error: Invalid file name format";
        }

        try {
            Path basePath = Paths.get(BASE_DIRECTORY).toRealPath();
            Path filePath = basePath.resolve(xmlFileName).normalize();
            
            if (!filePath.startsWith(basePath)) {
                return "Error: Path traversal attempt detected";
            }

            File xmlFile = filePath.toFile();
            if (!xmlFile.exists() || !xmlFile.isFile()) {
                return "Error: File does not exist or is not a regular file";
            }

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

            String sanitizedXPath = xpathValue.replaceAll("[^a-zA-Z0-9_\\\\-@\\\\[\\\\]=/'\\" ]", "");
            String xpathQuery = "/tag[@id=" + sanitizedXPath + "]";

            XPathFactory xpathFactory = XPathFactory.newInstance();
            XPath xpath = xpathFactory.newXPath();
            XPathExpression expression = xpath.compile(xpathQuery);
            NodeList nodeList = (NodeList) expression.evaluate(document, XPathConstants.NODESET);

            StringBuilder result = new StringBuilder();
            for (int i = 0; i < nodeList.getLength(); i++) {
                result.append(nodeList.item(i).getTextContent());
                if (i < nodeList.getLength() - 1) {
                    result.append(", ");
                }
            }

            return result.length() > 0 ? result.toString() : "No results found";

        } catch (Exception e) {
            return "Error: " + e.getClass().getSimpleName();
        }
    }

    public static void main(String[] args) {
        System.out.println("Test Case 1:");
        String result1 = executeXPathQuery("'test1'", "test1.xml");
        System.out.println(result1);

        System.out.println("\\nTest Case 2:");
        String result2 = executeXPathQuery("'test2'", "test2.xml");
        System.out.println(result2);

        System.out.println("\\nTest Case 3:");
        String result3 = executeXPathQuery("", "test.xml");
        System.out.println(result3);

        System.out.println("\\nTest Case 4:");
        String result4 = executeXPathQuery("'test'", "../etc/passwd");
        System.out.println(result4);

        System.out.println("\\nTest Case 5:");
        String result5 = executeXPathQuery("'test'", "nonexistent.xml");
        System.out.println(result5);
    }
}
