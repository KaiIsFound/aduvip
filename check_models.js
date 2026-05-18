const https = require('https');

const apiKey = "AIzaSyAQuHICXjwQdKSQJDfs2CsxW3CX9smdwq8";
const url = `https://generativelanguage.googleapis.com/v1beta/models?key=${apiKey}`;

https.get(url, (res) => {
  let data = '';
  res.on('data', (chunk) => { data += chunk; });
  res.on('end', () => {
    try {
      const parsed = JSON.parse(data);
      if (parsed.models) {
        const models = parsed.models.map(m => m.name).filter(n => n.includes('flash'));
        console.log('Available flash models:', models);
      } else {
        console.log(data);
      }
    } catch (e) {
      console.error(e);
    }
  });
}).on('error', (err) => {
  console.error('Error:', err.message);
});
