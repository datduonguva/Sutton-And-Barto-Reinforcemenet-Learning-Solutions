import json
import matplotlib.pyplot as plt

fn = "episode_vs_steps.json"
data = json.load(open(fn, "r"))

plt.plot(data)
plt.xlabel("steps", fontsize='medium')
plt.ylabel("episode", fontsize='medium')
plt.tight_layout()
plt.show()

