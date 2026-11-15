/**
 * Node.js HTTP server for Hospital Patient Priority Queue Web UI
 * Run   : node server.js
 * Open  : http://localhost:8080
 */
const http  = require('http');
const fs    = require('fs');
const path  = require('path');
const { exec } = require('child_process');

const PORT    = 8080;
const WEB_DIR = path.join(__dirname, 'web');

const MIME = {
  '.html': 'text/html',
  '.css' : 'text/css',
  '.js'  : 'application/javascript',
  '.png' : 'image/png',
  '.jpg' : 'image/jpeg',
  '.ico' : 'image/x-icon',
};

const server = http.createServer((req, res) => {
  let filePath = path.join(WEB_DIR, req.url === '/' ? 'index.html' : req.url);
  const ext  = path.extname(filePath);
  const mime = MIME[ext] || 'text/plain';

  fs.readFile(filePath, (err, data) => {
    if (err) {
      res.writeHead(404); res.end('Not found');
      return;
    }
    res.writeHead(200, { 'Content-Type': mime });
    res.end(data);
  });
});

server.listen(PORT, () => {
  console.log('='.repeat(55));
  console.log('  HOSPITAL PATIENT PRIORITY QUEUE — WEB SERVER');
  console.log('='.repeat(55));
  console.log(`  Running at : http://localhost:${PORT}`);
  console.log(`  Web root   : ${WEB_DIR}`);
  console.log('  Press Ctrl+C to stop.\n');

  // Auto-open browser on Windows
  const url = `http://localhost:${PORT}`;
  const cmd = process.platform === 'win32' ? `start ${url}` : `open ${url}`;
  exec(cmd);
});
