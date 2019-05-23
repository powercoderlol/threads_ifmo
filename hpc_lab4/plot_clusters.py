import matplotlib.pyplot as plt
import numpy as np
import json
import os
import sys

filename = sys.argv[1]
json_object = json.load(open(filename, "r"))
imported_data_ = json_object["data"]

data = np.array(np.array_split(np.array(imported_data_),len(imported_data_) / 3))
plt.scatter(data[:, 0], data[:, 1], c=data[:, 2])
plt.show()