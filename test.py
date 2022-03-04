import json
import re
from typing import List

with open("prs.json") as f:
    prs = json.load(f)


def find_tags(title: str) -> List[str]:
    m = re.findall(r'\[.*?\]', title)
    tags = [t.replace("[", "").replace("]", "").lower() for t in m]
    return tags


all_tags = {}

for pr in prs:
    # print(pr["number"], pr["title"])
    tags = find_tags(pr["title"])
    for t in tags:
        if t not in all_tags:
            all_tags[t] = [0, []]

        all_tags[t][0] += 1
        all_tags[t][1].append(pr["title"])

new = []
for tag, data in all_tags.items():
    new.append((tag, *data))

new = list(sorted(new, key=lambda x: x[1]))

for tag, count, prs in new:
    if count <= 5:
        continue
    print(tag, count)