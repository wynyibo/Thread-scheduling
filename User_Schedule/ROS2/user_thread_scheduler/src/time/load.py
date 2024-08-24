import matplotlib.pyplot as plt
import numpy as np

def read_data(filename):
    """ 从文件中读取数据并转换为 NumPy 数组 """
    with open(filename, 'r') as file:
        lines = file.readlines()
        
    data = []
    for line in lines:
        line = line.strip()
        values = list(map(float, line.split()))
        data.append(values)
    
    return np.array(data)

def compute_average(data):
    """ 计算每列数据的平均值 """
    return np.mean(data, axis=0)

# 文件名和标签
files = ['pload.txt', 'oload.txt', 'uload.txt']
labels = ['Pthread', 'OpenMP', 'User']

# 读取数据并计算平均值
data_dict = {label: read_data(file) for file, label in zip(files, labels)}

# 计算每列数据的平均值
averages = {label: compute_average(data) for label, data in data_dict.items()}

# 准备绘图数据
# x 轴标签：5min, 10min, 15min
x = np.arange(len(files))  # 负载时间段标签的索引
width = 0.2  # 柱状图宽度

# 计算数据用于绘图
data_for_plotting = np.array([averages[label] for label in labels])

fig, ax = plt.subplots(figsize=(12, 8))

# 绘制每个调度策略的数据
for i, label in enumerate(labels):
    ax.bar(x + i * width, data_for_plotting[i], width=width, label=label)

# 设置图表标签和标题
ax.set_xlabel('time')
ax.set_ylabel('averge')
ax.set_xticks(x + width)
ax.set_xticklabels(['5min', '10min', '15min'])
ax.legend()
ax.grid(True)

# 保存和显示图表
plt.tight_layout()
plt.savefig('cpu_load_comparison.png')
plt.show()
