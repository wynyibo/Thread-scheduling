import matplotlib.pyplot as plt

# 读取数据函数
def read_data(filename):
    with open(filename, 'r') as file:
        data = [int(line.strip()) for line in file]
    return data

# 读取三个文件的数据
data1 = read_data('time.txt')
data2 = read_data('./time/time_p.txt')
data3 = read_data('./time/time_o.txt')

# 生成 x 轴上的位置（根据数据长度）
x1 = range(len(data1))
x2 = range(len(data2))
x3 = range(len(data3))

# 绘制折线图
plt.figure(figsize=(12, 6))

plt.plot(x1, data1, label='User_schedule', marker='o')
plt.plot(x2, data2, label='pthread', marker='s')
plt.plot(x3, data3, label='openMP', marker='^')

plt.xlabel('thread number')
plt.ylabel('time')
plt.title('Comparison of task latency')
plt.legend()
plt.grid(True)

# 保存图像到文件
plt.savefig('line_chart_from_files.png')

# 显示图像
plt.show()
