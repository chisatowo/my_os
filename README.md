# 操作系统真相还原第14章：文件系统
## 实验小节j:
实现功能:
1. 使内核能够打开和关闭一个目录文件(其实就是将一个目录文件的路径通过调用search_file函数转换为inode索引,然后通过调用dir_open函数将对应的inode载入内存以实现快速查找)
实现函数:
1. sys_opendir用于根据传入的路径打开目录。原理：调用search_file将路径转换为inode索引，然后调用dir_open创建目录结构并将对应的inode载入内存
2. 增加sys_close函数,封装dir_close函数.
