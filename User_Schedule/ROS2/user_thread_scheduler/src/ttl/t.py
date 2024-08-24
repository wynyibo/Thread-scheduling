import matplotlib.pyplot as plt

def read_file(file_path):
    """读取文件中的数据"""
    bytes_list = []
    time_list = []
    with open(file_path, 'r') as file:
        for line in file:
            parts = line.split()
            if len(parts) == 2:
                # 将字节数乘以 1,000,000 以得到实际字节数
                bytes_list.append(int(parts[0]) * 1000000)
                time_list.append(int(parts[1]))
    return bytes_list, time_list

def calculate_throughput(bytes_list, time_list):
    """计算吞吐量"""
    throughput_list = [b / t if t != 0 else 0 for b, t in zip(bytes_list, time_list)]
    return throughput_list

def plot_results(file_paths, labels):
    """绘制吞吐量图表"""
    plt.figure(figsize=(10, 6))
    
    for file_path, label in zip(file_paths, labels):
        bytes_list, time_list = read_file(file_path)
        throughput_list = calculate_throughput(bytes_list, time_list)
        
        plt.plot(throughput_list, label=label)
    
    # plt.xlabel('记录编号')
    # plt.ylabel('吞吐量 (字节/秒)')
    # plt.title('吞吐量对比图')
    plt.legend()
    plt.grid(True)
    plt.savefig('throughput_comparison2.png')
    plt.show()

if __name__ == "__main__":
    file_paths = ['time11.txt', 'p.txt', 'o.txt']
    labels = ['User_schedule', 'Pthread', 'OpenMP']
    
    plot_results(file_paths, labels)
