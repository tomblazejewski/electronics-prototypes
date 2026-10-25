"""Minimal receiver: appends each POSTed reading to readings.csv. Stdlib only."""
import csv, json, time
from http.server import BaseHTTPRequestHandler, HTTPServer


class Handler(BaseHTTPRequestHandler):
    def do_POST(self):
        body = self.rfile.read(int(self.headers.get("Content-Length", 0)))
        try:
            d = json.loads(body)
        except ValueError:
            self.send_response(400); self.end_headers(); return
        with open("readings.csv", "a", newline="") as f:
            csv.writer(f).writerow(
                [time.time(), d.get("device"), d.get("density_kg_m3"), d.get("temp_c")])
        self.send_response(204); self.end_headers()


HTTPServer(("0.0.0.0", 8000), Handler).serve_forever()
