#!/usr/bin/env python3
import os

print("Content-Type: text/html\r")
print("\r")
print("<html><body>")
print("<h1>CGI Test</h1>")
print("<p>Query String: " + os.environ.get('QUERY_STRING', 'None') + "</p>")
print("<p>Request Method: " + os.environ.get('REQUEST_METHOD', 'None') + "</p>")
print("</body></html>")