"""
Simple HTTP server for Hospital Patient Priority Queue Web UI
Run: python server.py
Then open: http://localhost:8080
"""
import http.server
import socketserver
import os
import webbrowser
import threading

PORT = 8080
WEB_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "web")

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEB_DIR, **kwargs)
    def log_message(self, format, *args):
        print(f"  [{self.address_string()}] {format % args}")

def open_browser():
    import time; time.sleep(0.8)
    webbrowser.open(f"http://localhost:{PORT}")

if __name__ == "__main__":
    os.chdir(WEB_DIR)
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        print("=" * 55)
        print("  HOSPITAL PATIENT PRIORITY QUEUE — WEB SERVER")
        print("=" * 55)
        print(f"  Serving at: http://localhost:{PORT}")
        print(f"  Web root  : {WEB_DIR}")
        print("  Press Ctrl+C to stop.\n")
        threading.Thread(target=open_browser, daemon=True).start()
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n  Server stopped.")
