import re
import numpy as np
import time
import os
import subprocess

def get_cpu_usage():
    """获取当前的每个CPU使用情况"""
    with open('/proc/stat', 'r') as file:
        data = file.read()
    cpu_lines = re.findall(r'cpu\d+\s+(.+)', data)
    processed = [np.array(line.split(), dtype=int) for line in cpu_lines]
    return np.array(processed)

def calculate_cpu_percentage(processed_1, processed_2):
    """计算每个CPU的使用百分比"""
    deltas = np.array(processed_2) - np.array(processed_1)
    percentages = [(delta[0] + delta[1] + delta[2]) / delta.sum() * 100 for delta in deltas]
    return np.array(percentages)

def save_cpu_usage(filename, percentages):
    """将每个CPU的使用情况保存到文件"""
    with open(filename, 'a') as file:
        for i, percentage in enumerate(percentages):
            file.write(f"{percentage:.2f}%  ")
        file.write("\n")
    print(f"CPU usage data saved to {filename}")

def get_cpu_load():
    """获取系统的5分钟、10分钟、15分钟的CPU负载"""
    load_1, load_5, load_15 = os.getloadavg()
    return load_1, load_5, load_15

def save_cpu_load(filename, load_data):
    """将CPU负载信息保存到文件"""
    with open(filename, 'a') as file:
        file.write(f"{load_data[0]:.2f}  ")
        file.write(f"{load_data[1]:.2f}  ")
        file.write(f"{load_data[2]:.2f}\n")
    print(f"CPU load data saved to {filename}")

def run_test_program():
    """运行C语言测试程序"""
    print("Running test program...")
    proc = subprocess.Popen(["../la"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return proc

if __name__ == "__main__":
    # 运行测试程序
    test_program = run_test_program()

    while True:
        # 获取初始的CPU使用情况
        processed_1 = get_cpu_usage()

        # 等待一段时间（例如5秒钟），以计算CPU使用率变化
        time.sleep(5)

        # 获取等待后的CPU使用情况
        processed_2 = get_cpu_usage()

        # 计算CPU使用百分比
        cpu_percentages = calculate_cpu_percentage(processed_1, processed_2)

        # 保存CPU使用情况到cpu.txt
        save_cpu_usage('ucpu.txt', cpu_percentages)

        # 获取并保存5分钟、10分钟、15分钟的CPU负载到load.txt
        cpu_load = get_cpu_load()
        save_cpu_load('uload.txt', cpu_load)

        # 等待测试程序结束
        if test_program.poll() is not None:  # 检查程序是否已经结束
            stdout, stderr = test_program.communicate()
            print("Test program stdout:", stdout.decode())
            print("Test program stderr:", stderr.decode())
            break  # 程序结束，退出循环

    print("Script finished.")
