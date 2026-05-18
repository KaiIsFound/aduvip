const https = require('https');

const apiKey = "AIzaSyAQuHICXjwQdKSQJDfs2CsxW3CX9smdwq8";
const url = `https://generativelanguage.googleapis.com/v1beta/models/gemini-flash-latest:generateContent?key=${apiKey}`;

const data = JSON.stringify({
  contents: [{ parts: [{ text: "Hello" }] }]
});

const req = https.request(url, {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' }
}, (res) => {
  let responseData = '';
  res.on('data', (chunk) => { responseData += chunk; });
  res.on('end', () => {
    console.log('Status:', res.statusCode);
    console.log(responseData);
  });
});

req.on('error', (err) => console.error(err));
req.write(data);
req.end();
