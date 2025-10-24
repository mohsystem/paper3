script
const https = require('https');
const { URL } = require('url');
const dns = require('dns').promises;

const MAX_URL_LENGTH = 2048;
const MAX_RESPONSE_SIZE = 1048576; // 1MB limit
const TIMEOUT_SECONDS = 30000; // 30 seconds in milliseconds

/**
 * Check if IP address is private/internal - SSRF protection
 */
function isPrivateIp(ip) {
    if (!ip) return true;
    
    const parts = ip.split('.').map(Number);
    if (parts.length !== 4 || parts.some(p => isNaN(p) || p < 0 || p > 255)) {
        return true; // Invalid IP, treat as suspicious
    }
    
    // 10.0.0.0/8
    if (parts[0] === 10) return true;
    
    // 172.16.0.0/12
    if (parts[0] === 172 && parts[1] >= 16 && parts[1] <= 31) return true;
    
    // 192.168.0.0/16
    if (parts[0] === 192 && parts[1] === 168) return true;
    
    // 127.0.0.0/8 (localhost)
    if (parts[0] === 127) return true;
    
    // 169.254.0.0/16 (link-local)
    if (parts[0] === 169 && parts[1] === 254) return true;
    
    // 0.0.0.0/8
    if (parts[0] === 0) return true;
    
    return false;
}

/**
 * Validate URL format and security requirements
 */
async function validateUrl(urlString) {
    if (!urlString || typeof urlString !== 'string') {
        return false;
    }
    
    // Check length bounds
    if (urlString.length === 0 || urlString.length > MAX_URL_LENGTH) {
        return false;
    }
    
    // SECURITY: Only allow HTTPS protocol
    if (!urlString.startsWith('https://')) {
        return false;
    }
    
    let parsedUrl;
    try {
        parsedUrl = new URL(urlString);
    } catch (e) {
        return false;
    }
    
    // SECURITY: Reject URLs with credentials
    if (parsedUrl.username || parsedUrl.password) {
        return false;
    }
    
    const hostname = parsedUrl.hostname;
    if (!hostname) {
        return false;
    }
    
    // SECURITY: Resolve hostname and check if it points to private IP
    try {
        const addresses = await dns.resolve4(hostname);
        if (!addresses || addresses.length === 0) {
            return false;
        }
        
        // Check if any resolved IP is private
        for (const ip of addresses) {
            if (isPrivateIp(ip)) {
                return false;
            }
        }
    } catch (e) {
        // DNS resolution failed - reject
        return false;
    }
    
    return true;
}

/**
 * Make secure HTTPS request with all protections enabled
 */
async function makeSecureRequest(urlString) {
    if (!urlString) {
        console.error('Error: URL is required.');
        return null;
    }
    
    // SECURITY: Validate URL before any network activity
    const isValid = await validateUrl(urlString);
    if (!isValid) {
        console.error('Error: Invalid URL. Only HTTPS URLs to public hosts are allowed.');
        return null;
    }
    
    return new Promise((resolve, reject) => {
        const parsedUrl = new URL(urlString);
        
        // SECURITY: Configure HTTPS request with certificate verification
        const options = {
            hostname: parsedUrl.hostname,
            port: parsedUrl.port || 443,
            path: parsedUrl.pathname + parsedUrl.search,
            method: 'GET',
            headers: {
                'User-Agent': 'SecureHTTPClient/1.0'
            },
            // SECURITY: Certificate verification enabled by default
            // rejectUnauthorized is true by default - never set to false
            timeout: TIMEOUT_SECONDS
        };
        
        const req = https.request(options, (res) => {
            // Check HTTP status code
            if (res.statusCode < 200 || res.statusCode >= 300) {
                console.error(`Error: HTTP request returned status code ${res.statusCode}`);
                resolve(null);
                return;
            }
            
            let data = '';
            let totalLength = 0;
            
            res.on('data', (chunk) => {
                // SECURITY: Check size limit during read
                totalLength += chunk.length;
                if (totalLength > MAX_RESPONSE_SIZE) {
                    console.error('Error: Response exceeded size limit.');
                    req.destroy();
                    resolve(null);
                    return;
                }
                data += chunk;
            });
            
            res.on('end', () => {
                resolve(data);
            });
        });
        
        req.on('error', (e) => {
            // Generic error - don't leak internal details\n            console.error('Error: Request failed.');\n            resolve(null);\n        });\n        \n        req.on('timeout', () => {\n            console.error('Error: Request timed out.');\n            req.destroy();\n            resolve(null);\n        });\n        \n        req.end();\n    });\n}\n\n/**\n * Main function with test cases\n */\nasync function main() {\n    // Test case 1: Valid HTTPS URL\n    console.log('Test 1: Valid HTTPS URL');\n    let response = await makeSecureRequest('https://example.com');\n    if (response) {\n        console.log(`Success! Response length: ${response.length} bytes`);\n        console.log(`Response preview: ${response.substring(0, 100)}...`);\n    }\n    console.log();\n    \n    // Test case 2: Invalid URL (HTTP instead of HTTPS)\n    console.log('Test 2: Invalid URL (HTTP not allowed)');\n    response = await makeSecureRequest('http://example.com');\n    if (response) {\n        console.log('Success (unexpected)');\n    } else {\n        console.log('Rejected as expected.');\n    }\n    console.log();\n    \n    // Test case 3: Empty URL\n    console.log('Test 3: Empty URL');\n    response = await makeSecureRequest('');\n    if (response) {\n        console.log('Success (unexpected)');\n    } else {\n        console.log('Rejected as expected.');\n    }\n    console.log();\n    \n    // Test case 4: URL with credentials (should be rejected)\n    console.log('Test 4: URL with credentials');\n    response = await makeSecureRequest('https://user:pass@example.com');\n    if (response) {\n        console.log('Success (unexpected)');\n    } else {\n        console.log('Rejected as expected.');\n    }\n    console.log();\n    \n    // Test case 5: Private IP address (should be rejected - SSRF protection)\n    console.log('Test 5: Private IP address (SSRF protection)');\n    response = await makeSecureRequest('https://127.0.0.1');\n    if (response) {\n        console.log('Success (unexpected)');\n    } else {\n        console.log('Rejected as expected.');\n    }\n    console.log();\n}\n\n// Run main function\nmain().catch(err => {\n    console.error('Fatal error:', err);
    process.exit(1);
});
