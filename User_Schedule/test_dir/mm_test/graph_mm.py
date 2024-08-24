import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


def parse_filtered_data(file_path):
    """
    从文本文件中解析数据，返回包含 VSZ 和 RSS 的字典。
    
    :param file_path: 输入文件路径
    :return: 包含 VSZ 和 RSS 的字典
    """
    data = {'Pthread': {'VSZ': 0, 'RSS': 0}, 'User_schedule': {'VSZ': 0, 'RSS': 0},'openMP': {'VSZ': 0, 'RSS': 0}}
    
    with open(file_path, 'r') as file:
        for line in file:
            if 'Command: test' in line:
                parts = line.split(',')
                vsz = float(parts[4].split(': ')[1].strip())
                rss = float(parts[5].split(': ')[1].strip())
                data['Pthread']['VSZ'] = vsz
                data['Pthread']['RSS'] = rss
            elif 'Command: main' in line:
                parts = line.split(',')
                vsz = float(parts[4].split(': ')[1].strip())
                rss = float(parts[5].split(': ')[1].strip())
                data['User_schedule']['VSZ'] = vsz
                data['User_schedule']['RSS'] = rss
            elif 'Command: openMP' in line:
                parts = line.split(',')
                vsz = float(parts[4].split(': ')[1].strip())
                rss = float(parts[5].split(': ')[1].strip())
                data['openMP']['VSZ'] = vsz
                data['openMP']['RSS'] = rss
    return data
def plot_data(data):
    """
    绘制 VSZ 和 RSS 的柱状图并保存到文件。
    
    :param data: 包含 VSZ 和 RSS 的字典
    """
    labels = list(data.keys())
    vsz_values = [data[label]['VSZ'] for label in labels]
    rss_values = [data[label]['RSS'] for label in labels]

    x = range(len(labels))

    fig, ax = plt.subplots()
    ax.bar(x, vsz_values, width=0.4, label='VSZ', color='b', align='center')
    ax.bar(x, rss_values, width=0.4, label='RSS', color='r', align='edge')

    ax.set_xlabel('lib')
    ax.set_ylabel('Value')
    ax.set_title('VSZ and RSS values with Different Scheduling Methods')
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend()

    plt.savefig('../mm_info_compare.png')  # 保存图形到文件

if __name__ == "__main__":
    file_path = 'filtered_process_info.txt'
    data = parse_filtered_data(file_path)
    plot_data(data)
