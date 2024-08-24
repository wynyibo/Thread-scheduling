import matplotlib.pyplot as plt

def read_data(filename):
    """读取数据文件并解析为标签和数据"""
    labels = []
    data = {'Pthread': [], 'User_schedule': [], 'openMP': []}
    
    with open(filename, 'r') as file:
        for line in file:
            parts = line.strip().split(',')
            label = parts[0]
            values = list(map(float, parts[1:]))
            if label in data:
                data[label].extend(values)
    
    return data

def plot_data(data):
    """绘制柱状图"""
    labels = list(data.keys())
    bar_width = 0.25  # 调整柱宽以适应更多数据
    index = range(len(data['Pthread']))

    plt.figure(figsize=(12, 6))

    # 绘制 Pthread 数据
    plt.bar(index, data['Pthread'], bar_width, label='Pthread', color='blue')

    # 绘制 User_schedule 数据
    plt.bar([i + bar_width for i in index], data['User_schedule'], bar_width, label='User_schedule', color='orange')

    # 绘制 openMP 数据
    plt.bar([i + 2 * bar_width for i in index], data['openMP'], bar_width, label='openMP', color='green')

    plt.xlabel('CPU Core Index')
    plt.ylabel('CPU Usage Percentage')
    plt.title('CPU Usage Percentage with Different Scheduling Methods')
    plt.xticks([i + bar_width for i in index], [f'Core {i}' for i in index])
    plt.legend()
    plt.tight_layout()

    plt.savefig('../cpu_usage_comparison.png')
    plt.show()

if __name__ == "__main__":
    data = read_data('cpu_usage_data.txt')
    plot_data(data)
