import json
from collections import defaultdict
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
        all_tags[t][1].append(pr)

new = []
for tag, data in all_tags.items():
    new.append((tag, *data))

new = list(sorted(new, key=lambda x: x[1]))

for tag, count, prs in new:
    if count <= 3:
        continue

    authors = list(sorted([pr["author"]["login"] for pr in prs]))
    seen = defaultdict(lambda: 0)
    for a in authors:
        seen[a] += 1
    seen = dict(seen)

    # authors = list(sorted(set(pr["author"]["login"] for pr in prs)))
    # authors = [f"@{a}" for a in authors]
    # authors = " ".join(authors)
    authors = [(a, i) for a, i in seen.items()]
    authors = reversed(sorted(authors, key=lambda x: x[1]))
    authors = [f"@{a} ({i})" for a, i in authors]
    if count > 4:
        print(f"{tag} ({count} total)")
        # print("   ", " ".join(authors))
        # print("")