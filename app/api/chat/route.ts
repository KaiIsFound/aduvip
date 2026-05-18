import { NextResponse } from 'next/server';
import Groq from 'groq-sdk';

// Initialize Groq SDK with the API key from environment variables
const groq = new Groq({
  apiKey: process.env.GROQ_API_KEY,
});

export async function POST(request: Request) {
  try {
    const { message } = await request.json();

    if (!message) {
      return NextResponse.json(
        { error: 'Message is required' },
        { status: 400 }
      );
    }

    // Call the Groq API
    const chatCompletion = await groq.chat.completions.create({
      messages: [
        {
          role: 'system',
          content: 'You are a helpful AI assistant.',
        },
        {
          role: 'user',
          content: message,
        },
      ],
      model: 'llama3-8b-8192', // You can change the model here (e.g. mixtral-8x7b-32768, llama3-70b-8192)
      temperature: 0.7,
      max_tokens: 1024,
    });

    const reply = chatCompletion.choices[0]?.message?.content || 'No response';

    return NextResponse.json({ reply });
  } catch (error: any) {
    console.error('Groq API Error:', error);
    return NextResponse.json(
      { error: error.message || 'An error occurred during the Groq API call' },
      { status: 500 }
    );
  }
}
