"""
-*- coding: utf-8 -*-
Created on JUN 4, 2019
@title: MCTS test function
@tudo: Monte-Carlo Tree Search without machine learning
@author: martha
"""

import numpy as np
import matplotlib.pyplot as plt

plt.figure()

file_pro = open("/root/Documents/CCD/CCDdata0703.txt", 'r')
data_list = []
data_diff = []
try:
    for line in file_pro:
        data_list.append(int(line, 16))
finally:
    file_pro.close()

print('origin data = ', data_list)

for i in range (len(data_list)  -1):
    data_diff.append(data_list[i+1] -data_list[i])
print(data_diff)
print(len(data_diff))

x = [0, 1]
y = [0, 1]
plt.figure()
plt.plot(x, y)


