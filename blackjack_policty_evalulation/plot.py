import numpy as np
import matplotlib.pyplot as plt
import sys

for i in range(int(sys.argv[2])):
    fn = "{}_{}.txt".format(sys.argv[1], i)
    with open(fn, "r") as f:
        data = f.readline().strip().split(",")
    arr = np.array(list(map(float, data))).reshape((21, 21))
    plt.figure()
    plt.imshow(arr)

    plt.show()
