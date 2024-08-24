import matplotlib.pyplot as plt
import numpy as np

def read_data(filename):
    """ 从文件中读取数据并转换为 NumPy 数组 """
    with open(filename, 'r') as file:
        lines = file.readlines()
        
    data = []
    for line in lines:
        # 移除行末的换行符，并将百分号替换为空白
        line = line.strip().replace('%', '')
        # 将处理后的行拆分为数值，并将其转换为浮点数
        values = list(map(float, line.split()))
        data.append(values)
    
    return np.array(data)

def plot_cpu_usage_bar(data_dict, labels, title):
    """ 使用柱状图绘制 CPU 使用率 """
    # 获取 CPU 核心数
    num_cpus = data_dict[labels[0]].shape[1]
    # 准备数据
    avg_usage = {label: np.mean(data_dict[label], axis=0) for label in labels}
    
    x = np.arange(num_cpus)  # CPU 核心的 x 坐标
    width = 0.2  # 柱子的宽度

    fig, ax = plt.subplots(figsize=(12, 8))
    
    for i, label in enumerate(labels):
        ax.bar(x + i * width, avg_usage[label], width, label=label)

    # 设置横坐标和纵坐标的标签
    ax.set_xlabel('CPU Cores')
    ax.set_ylabel('Average CPU Usage (%)')
    ax.set_title(title)
    ax.set_xticks(x + width)
    ax.set_xticklabels([f'CPU {i}' for i in range(num_cpus)])
    ax.legend()
    ax.grid(True, linestyle='--', alpha=0.7)
    
    # 保存图像到文件
    plt.savefig('bar_chart_from_files.png')
    # 显示图像
    plt.show()

# 文件名和标签
files = ['ocpu.txt', 'pcpu.txt', 'ucpu.txt']
labels = ['pthread', 'User_schedule', 'openmp']

# 读取数据
data_dict = {label: read_data(file) for file, label in zip(files, labels)}

# 绘制并保存 CPU 使用率柱状图
plot_cpu_usage_bar(data_dict, labels, 'CPU Usage Comparison (Bar Chart)')
