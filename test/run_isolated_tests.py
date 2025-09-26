import subprocess
import re
import os

# --- 配置 ---
TEST_EXECUTABLE = "./cmake-build-debug/semantic_test"  # 你的 Google Test 可执行文件路径
REPORT_DIR = "test_reports"      # 存储测试报告的目录

# 确保测试可执行文件存在
if not os.path.exists(TEST_EXECUTABLE):
  print(f"Error: Test executable '{TEST_EXECUTABLE}' not found.")
  print("Please compile your Google Test project first.")
  exit(1)

# 创建报告目录
os.makedirs(REPORT_DIR, exist_ok=True)


def get_all_tests(executable):
  """
  通过运行带有 --gtest_list_tests 选项的可执行文件来获取所有测试名称。
  """
  try:
    # 使用 check_output 而不是 run 来直接获取输出
    result = subprocess.check_output([executable, "--gtest_list_tests"], text=True)
  except subprocess.CalledProcessError as e:
    print(f"Error listing tests: {e}")
    print(f"Output: {e.stdout}")
    exit(1)

  tests = []
  current_test_suite = None
  for line in result.splitlines():
    line = line.strip()
    if not line:
      continue

    if line.endswith('.'):  # Test suite name
      current_test_suite = line[:-1]
    elif current_test_suite and not line.startswith(' '): # Test case name
      test_case_name = line
      tests.append(f"{current_test_suite}.{test_case_name}")
  return tests

def run_single_test_isolated(test_name, executable, report_path):
  """
  在一个独立的进程中运行单个测试。
  """
  print(f"--- Running test: {test_name} ---")
  command = [
    executable,
    f"--gtest_filter={test_name}",
    f"--gtest_output=xml:{report_path}" # 输出XML报告
  ]

  try:
    # 使用 shell=True 在某些情况下可能避免命令行长度限制，但通常不推荐
    # 在这里我们直接传递列表，更安全
    process = subprocess.run(command, capture_output=True, text=True)

    if process.returncode != 0:
      print(f"FAILURE IN {test_name}: Process exited with code {process.returncode}")
      print("STDOUT:")
      print(process.stdout)
      print("STDERR:")
      print(process.stderr)
      return False # 表示失败
    else:
      print(f"SUCCESS FOR {test_name}")
      print(process.stdout) # 可以选择打印标准输出
      return True # 表示成功

  except Exception as e:
    print(f"ERROR RUNNING {test_name}: {e}")
    return False # 表示失败

# --- 主执行逻辑 ---
if __name__ == "__main__":
  print(f"Getting all tests from {TEST_EXECUTABLE}...")
  all_tests = get_all_tests(TEST_EXECUTABLE)
  print(f"Found {len(all_tests)} tests.")

  failed_tests = []

  for i, test_name in enumerate(all_tests):
    # 为每个测试生成一个唯一的报告文件名
    report_filename = f"test_report_{test_name.replace('.', '_')}.xml"
    report_full_path = os.path.join(REPORT_DIR, report_filename)

    print(f"\n[{i+1}/{len(all_tests)}] Running '{test_name}'...")
    success = run_single_test_isolated(test_name, TEST_EXECUTABLE, report_full_path)

    if not success:
      failed_tests.append(test_name)

  print("\n--- Summary ---")
  if failed_tests:
    print(f"Total tests run: {len(all_tests)}")
    print(f"Failed tests: {len(failed_tests)}")
    for test in failed_tests:
      print(f"  - {test}")
    exit(1) # 以非零退出码表示有失败
  else:
    print(f"All {len(all_tests)} tests passed!")
    exit(0) # 以零退出码表示所有通过
