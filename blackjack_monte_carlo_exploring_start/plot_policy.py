import matplotlib.pyplot as plt
import numpy as np
import json
import sys

fn = "data.txt"
data = np.array(json.load(open(fn, "r")))
titles = ['No usable ace', "Usable ace"]
plt.figure(figsize=(10, 5))
for i in range(2):
    plt.subplot(1, 2, i+1)
    plt.title(titles[i])
    plt.imshow(data[12:,1:, i], origin='lower')
    plt.yticks(np.arange(10), [str(j) for j in range(12, 22)])
    plt.xticks(np.arange(10), [str(j) for j in range(1, 11)])
    plt.grid(True)

plt.tight_layout()
plt.savefig("policy.png")
plt.show()

