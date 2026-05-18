require('dotenv').config({ path: '.env.local' });
const Groq = require('groq-sdk');

const groq = new Groq({
  apiKey: process.env.GROQ_API_KEY
});

async function main() {
  try {
    const chatCompletion = await groq.chat.completions.create({
      messages: [
        {
          role: 'user',
          content: 'Hello! Please say this is a test from Groq.',
        },
      ],
      model: 'llama-3.1-8b-instant', // Supported model
    });
    
    console.log(chatCompletion.choices[0]?.message?.content);
  } catch (error) {
    console.error('Error calling Groq:', error);
  }
}

main();
