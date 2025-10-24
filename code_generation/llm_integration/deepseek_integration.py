import os
from openai import OpenAI

class DeepseekIntegration:
    def __init__(self):
        os.environ['OPENAI_API_KEY'] = 'sk-e941d0bfbe044c928e7c8ab4e92c83e8'
        # os.environ['OPENAI_API_KEY'] = 'sk-proj-tVYBVKignUJEbsNfZQulT3BlbkFJfKB5rof08KoYvqx0o3S8' #my account
        self.client = OpenAI()

    def get_completion_content(self, prompt, selected_model="deepseek-chat"):
        # https://platform.openai.com/settings/organization/limits
        # https://platform.openai.com/docs/models
        completion = self.client.chat.completions.create(
            max_tokens=4096,
            top_p=0.90,
            temperature=0.9,
            model=selected_model,
            messages=[
                {"role": "user", "content": prompt}
            ]
        )
        return completion
#
# from openai import OpenAI
# client = OpenAI()

# completion = client.chat.completions.create(
#     model="gpt-4o",
#     messages=[
#         {"role": "user", "content": "Write java code to sum two numbers"}
#     ]
# )
#
# print(completion.choices[0].message.content)