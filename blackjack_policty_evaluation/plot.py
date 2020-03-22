import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import json
import sys

titles = ["No usable ac", "Usable ace"]
fig = plt.figure(figsize=(12, 5))
for i in range(2):
    ax = fig.add_subplot(121 + i, projection='3d')
    plt.title(titles[i])
    data = np.array(json.load(open("data.txt", "r")))
    x = np.arange(0, 10, 1)
    y = np.arange(0, 10, 1)
    X, Y = np.meshgrid(x, y)
    ax.plot_wireframe(X, Y, data[12:, 1:, i])
    ax.set_zlim(-1, 1)
    plt.xticks(np.arange(10), [str(i) if i != 1 else "ace" for i in range(1, 11)])
    plt.yticks(np.arange(10), [str(i) for i in range(12, 22)])
plt.tight_layout()
plt.savefig("value_function.png")
plt.show()
