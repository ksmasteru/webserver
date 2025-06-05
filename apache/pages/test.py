import time
import sys

print("Content-Type: text/html\\r\\n\\r\\n")
print("<html><body>")


counter = 0
start_time = time.time()

try:
    while True:
        counter += 1
        if counter % 100000 == 0:  # Print every 100k iterations
            elapsed = time.time() - start_time
            print(f"<p>Iteration {counter}, elapsed: {elapsed:.1f}s</p>")
        
        # Simulate some work
        dummy = sum(range(100))
        
except KeyboardInterrupt:
    print(f"<p>❌ Script interrupted after {counter} iterations</p>")
    print("</body></html>")
    sys.exit(124)  # Timeout exit code