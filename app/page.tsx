'use client';

import { useState } from 'react';

export default function Home() {
  const [input, setInput] = useState('');
  const [response, setResponse] = useState('');
  const [loading, setLoading] = useState(false);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!input) return;

    setLoading(true);
    setResponse('');

    try {
      const res = await fetch('/api/chat', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ message: input }),
      });

      const data = await res.json();
      if (res.ok) {
        setResponse(data.reply);
      } else {
        setResponse('Error: ' + data.error);
      }
    } catch (error) {
      setResponse('Failed to send message.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="min-h-screen flex items-center justify-center p-8 font-[family-name:var(--font-geist-sans)] bg-gray-50 dark:bg-zinc-950">
      <main className="w-full max-w-2xl flex flex-col gap-8 bg-white dark:bg-zinc-900 p-8 rounded-2xl shadow-xl">
        <div className="flex flex-col gap-2">
          <h1 className="text-3xl font-bold text-gray-900 dark:text-white">Groq AI Assistant</h1>
          <p className="text-gray-500 dark:text-gray-400">Powered by Next.js & Llama 3</p>
        </div>

        <div className="flex flex-col gap-4 min-h-[200px] p-4 bg-gray-50 dark:bg-zinc-950 rounded-xl border border-gray-100 dark:border-zinc-800">
          {response ? (
            <div className="text-gray-800 dark:text-gray-200 whitespace-pre-wrap">{response}</div>
          ) : (
            <div className="text-gray-400 italic flex h-full items-center justify-center">
              AI response will appear here...
            </div>
          )}
        </div>

        <form onSubmit={handleSubmit} className="flex gap-4">
          <input
            type="text"
            value={input}
            onChange={(e) => setInput(e.target.value)}
            placeholder="Ask something..."
            className="flex-1 px-4 py-3 rounded-xl border border-gray-200 dark:border-zinc-700 bg-white dark:bg-zinc-800 focus:outline-none focus:ring-2 focus:ring-blue-500 transition-all text-gray-900 dark:text-white"
          />
          <button
            type="submit"
            disabled={loading}
            className="px-6 py-3 bg-blue-600 hover:bg-blue-700 text-white font-medium rounded-xl transition-all disabled:opacity-50 disabled:cursor-not-allowed"
          >
            {loading ? 'Sending...' : 'Send'}
          </button>
        </form>
      </main>
    </div>
  );
}
