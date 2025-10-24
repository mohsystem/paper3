from google import genai
from google.genai import types

# Only run this block for Google AI API
client = genai.Client(api_key='AIzaSyB54k37O691yozKJlOcE0bczSajg1F0Qw0')


response = client.models.generate_content(
    model="gemini-1.5-pro-002",
    contents=types.Part.from_text('Why is sky blue?'),
    config=types.GenerateContentConfig(
        temperature=0.90,
        top_p=0.90,
        # top_k=20,
        # candidate_count=1,
        # seed=5,
        max_output_tokens=4096,
        # presence_penalty=0.0,
        # frequency_penalty=0.0,
        safety_settings= [types.SafetySetting(
            category='HARM_CATEGORY_HATE_SPEECH',
            threshold='BLOCK_ONLY_HIGH',
        )]
    )
)

print(response.text)