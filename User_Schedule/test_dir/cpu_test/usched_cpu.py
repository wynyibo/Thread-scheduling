import re
import numpy as np
import time
import subprocess

def get_cpu_usage():
    """获取当前的 CPU 使用情况"""
    data = open('/proc/stat', 'r').read()
    cpu = re.findall(r'cpu\d+\s(.+)', data)
    processed = [np.array(each.split(), dtype=int) for each in cpu]
    return np.array(processed)

def calculate_cpu_percentage(processed_1, processed_2):
    """计算 CPU 使用百分比"""
    rst = np.array(processed_2) - np.array(processed_1)
    percentage = [(each[0] + each[1] + each[2]) / each.sum() for each in rst]
    return np.array(percentage)

def run_test_program():
    """运行测试程序"""
    print("Running test program...")
    proc = subprocess.Popen(["../../ROS2/user_thread_scheduler/src/main"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = proc.communicate()
    print("Test program stdout:", stdout.decode())
    print("Test program stderr:", stderr.decode())
    return proc

def save_data_to_file(filename, label, data):
    """将数据保存到文件"""
    with open(filename, 'w') as file:
        file.write(f"{label},{','.join(map(str, data))}\n")
    print(f"Data saved to {filename}")

if __name__ == "__main__":
    print("Starting experiment with Pthread...")
    processed_1_no_omp = get_cpu_usage()
    time.sleep(5)  # 确保有足够的时间进行测试
    processed_2_no_omp = get_cpu_usage()
    no_omp_data = calculate_cpu_percentage(processed_1_no_omp, processed_2_no_omp)
    print("With User_schedule CPU usage percentages:", no_omp_data)

    save_data_to_file('cpu_usage_data.txt', 'Pthread', no_omp_data)
    # 程序结束
    print("Ending script.")
    exit()
