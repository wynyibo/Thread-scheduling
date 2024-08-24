import subprocess
import re

def parse_pidstat_output(output):
    """
    解析 pidstat 命令的输出，并返回一个包含进程信息的列表。
    """
    lines = output.splitlines()
    process_info = []

    # 跳过前两行标题行
    data_lines = lines[2:]

    # 匹配每一行的数据
    for line in data_lines:
        if line.strip():  # 忽略空行
            parts = re.split(r'\s+', line.strip())
            if len(parts) >= 8:  # 确保行中有足够的列
                process_info.append({
                    'UID': parts[1],
                    'PID': parts[2],
                    'minflt/s': parts[3],
                    'majflt/s': parts[4],
                    'VSZ': parts[5],
                    'RSS': parts[6],
                    '%MEM': parts[7],
                    'Command': ' '.join(parts[8:])
                })
    return process_info

def get_pidstat_info():
    """
    执行 pidstat 命令并返回解析后的进程信息。
    """
    command = ['pidstat', '-p', 'ALL', '-r']
    result = subprocess.run(command, capture_output=True, text=True)
    output = result.stdout
    return parse_pidstat_output(output)

def filter_and_save_info(info, commands, output_file):
    """
    过滤进程信息并将符合条件的数据保存到文本文件中。
    
    :param info: 解析后的进程信息列表
    :param commands: 需要筛选的命令列表
    :param output_file: 输出文件路径
    """
    filtered_info = [process for process in info if process['Command'] in commands]
    
    with open(output_file, 'w') as file:
        for process in filtered_info:
            file.write(f"UID: {process['UID']}, PID: {process['PID']}, minflt/s: {process['minflt/s']}, majflt/s: {process['majflt/s']}, VSZ: {process['VSZ']}, RSS: {process['RSS']}, %MEM: {process['%MEM']}, Command: {process['Command']}\n")

if __name__ == "__main__":
    info = get_pidstat_info()
    commands = ['test', 'main','openMP']
    output_file = 'filtered_process_info.txt'
    filter_and_save_info(info, commands, output_file)
