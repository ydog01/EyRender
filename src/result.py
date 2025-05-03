import os

def main():
    # 获取当前目录下的所有文件
    files = os.listdir('.')
    # 筛选出.cpp和.hpp文件，排除子目录
    cpp_hpp_files = [
        f for f in files
        if os.path.isfile(f) and os.path.splitext(f)[1] in ('.cpp', '.hpp')
    ]
    # 按文件名排序
    cpp_hpp_files.sort()
    
    with open('result.txt', 'w', encoding='utf-8') as outfile:
        for filename in cpp_hpp_files:
            # 生成分隔线，左右各18个减号包围文件名
            separator = f'------------------{filename}------------------'
            outfile.write(f'{separator}\n')
            
            # 读取文件内容并确保末尾有换行符
            with open(filename, 'r', encoding='utf-8') as infile:
                content = infile.read()
            if content and content[-1] != '\n':
                content += '\n'
            outfile.write(content)
        
        # 写入最后的结束分隔线（36个减号）
        outfile.write('-' * 36 + '\n')

if __name__ == '__main__':
    main()