import os
import json
import urllib
from urllib import request

headers = {
    "Authorization": f"Bearer {os.environ['GH_PAT']}",
}
user = "octoml"
repo = "tvm"
pr_number = "34"


def fetch_pr(user, repo, number):
    url = f"https://api.github.com/repos/{user}/{repo}/pulls/{number}"
    req = urllib.request.Request(url, headers=headers)
    with urllib.request.urlopen(req) as response:
        response = json.loads(response.read())
    return response


def update_pr_title(user, repo, number, title):
    url = f"https://api.github.com/repos/{user}/{repo}/pulls/{number}"
    data = {"title": title}
    data = json.dumps(data).encode()
    req = urllib.request.Request(url, headers=headers, data=data)
    with urllib.request.urlopen(req) as response:
        response = json.loads(response.read())
    return response


current_title = fetch_pr(user, repo, pr_number)["title"]
if current_title.startswith("[skip ci]"):
    print("PR title already starts with '[skip ci]', not doing anything")
else:
    print("Adding '[skip ci]' to PR title")
    update_pr_title(user, repo, pr_number, title=f"[skip ci] {current_title}")
