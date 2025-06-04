#!/usr/bin/env python3

import os

print("Content-Type: text/html")
print()  # End of headers

print("<html><body>")
print("<h1>Hello from CGI</h1>")
print("<h2>Environment Variables:</h2>")
print("<table border='1' cellpadding='5' cellspacing='0'>")
print("<tr><th>Variable</th><th>Value</th></tr>")

for key, value in os.environ.items():
    print(f"<tr><td>{key}</td><td>{value}</td></tr>")

print("</table>")
print("</body></html>")
