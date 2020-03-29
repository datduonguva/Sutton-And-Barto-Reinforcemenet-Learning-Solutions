import numpy as np
import json
import matplotlib.pyplot as plt


random_policy_ordinary_sampling = np.array(json.load(open("random_policy_ordinary_importance_sampling.json")))
target_value = json.load(open("target_value.json"))

ordinary_error =  np.mean((random_policy_ordinary_sampling - target_value)**2, axis = 0)

random_policy_weighted_sampling = np.array(json.load(open("random_policy_weighted_importance_sampling.json")))

weighted_error =  np.mean((random_policy_weighted_sampling - target_value)**2, axis = 0)

plt.figure(figsize=(8, 5))
plt.plot(ordinary_error, label="ordinary importance sampling")
plt.plot(weighted_error, label="weighted importance sampling")
plt.xscale('log')
plt.xlabel("episode")
plt.ylabel("mean squared error")
plt.legend()
plt.savefig("off_policy_plot.png")
plt.show()

