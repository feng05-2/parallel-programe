import subprocess
import re
import csv

executable = "./main"
num_values = [10000 + i * 10000 for i in range(15)]
repeat_times = 2

guess_pattern = re.compile(r"Guess time:([0-9.]+)seconds")
hash_pattern = re.compile(r"Hash time:([0-9.]+)seconds")

results = []

for num in num_values:
    guess_times = []
    hash_times = []

    for i in range(1, repeat_times + 1):
        try:
            result = subprocess.run([executable, str(num)], capture_output=True, text=True, check=True)
            output = result.stdout

            guess_match = guess_pattern.search(output)
            hash_match = hash_pattern.search(output)

            if guess_match and hash_match:
                guess_time = float(guess_match.group(1))
                hash_time = float(hash_match.group(1))
                guess_times.append(guess_time)
                hash_times.append(hash_time)
                print(f"num={num} 运行第{i}次: Guess time={guess_time:.6f}秒, Hash time={hash_time:.6f}秒")
            else:
                print(f"警告：未能解析输出，num={num}，第{i}次")
        except subprocess.CalledProcessError as e:
            print(f"错误：执行失败，num={num}，第{i}次\n错误信息:\n{e}")

    avg_guess = sum(guess_times) / len(guess_times) if guess_times else 0.0
    avg_hash = sum(hash_times) / len(hash_times) if hash_times else 0.0
    results.append((num, f"{avg_guess:.6f}seconds", f"{avg_hash:.6f}seconds"))

with open("timing_results.csv", "w", newline='') as csvfile:
    writer = csv.writer(csvfile)
    for row in results:
        writer.writerow(row)

print("结果已写入 timing_results.csv")
