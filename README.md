# 操作系统真相还原第14章：文件系统
## 实验小节i:
实现功能:
1. 能够通过输入一个路径创建对应的目录文件.
实现函数:
1. sys_mkdir,用于根据传入的路径创建目录文件。
核心原理：
1. 调用search_file来确认待创建的新目录文件在文件系统上不存在；
2. 调用inode_bitmap_alloc来为新目录分配inode索引，并调用inode_init来初始化这个inode；
3. 调用block_bitmap_alloc来分配一个块用于承载该目录的目录文件。同时设定inode的i_sectors[0]，并调用bitmap_sync同步块位图；
4. 清零缓冲区，然后写入.与…两个目录项，之后将缓冲区内容写入到目录文件中，这就完成了.与…两个目录项的创建；
5. 创建并设定自己的目录项，调用sync_dir_entry来将目录项同步到父目录中；
6. inode_sync同步父目录的inode与自己的inode，bitmap_sync同步inode位图
