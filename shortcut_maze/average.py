import numpy as np
import json
import matplotlib.pyplot as plt
data = []
for i in range(10):
    with open("reward_vs_step_0{}.json".format(i), "r") as f:
        data.append(json.load(f))

data = np.array(data)
data = np.mean(data, axis=0)
data1 = []
for i in range(10):
    with open("reward_vs_step_1{}.json".format(i), "r") as f:
        data1.append(json.load(f))

data1 = np.array(data1)
data1 = np.mean(data1, axis=0)


plt.plot(data, label='Dyna Q')
plt.plot(data1, label='Dyna Q +')
plt.legend()
plt.tight_layout()
plt.show()
