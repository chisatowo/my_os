# 操作系统真相还原第14章：文件系统
## 实验小节d：
1. 增添文件打开函数file_open:用于打开文件，实质就是让文件结构指向一个内存中的inode。原理：传入需要打开文件的inode数组索引，调用get_free_slot_in_global在全局打开文件结构数组中找个空位，调用inode_open将inode调入内存中，然后让该文件结构指向这个inode，最后调用pcb_fd_install安装文件描述符。为了同步性，以写的方式打开的文件不应该是正在写入的文件，需要判断该文件对应inode中的正在写入标志。
2. 将file_open函数封装进入sys_open函数以实现内核状态下文件的打开
3. 增加file_close函数,file_close用传入的文件结构指针关闭文件。步骤：将文件正在写标志关闭，调用inode_close移除内存中的inode，并解除文件结构与inode的关系。打开关闭文件，就是在处理文件结构与inode的关系。
4. 增添fd_local2global函数，用于将文件描述符转换为全局打开文件结构数组下标，原理就是去当前正在运行的进程或线程的pcb中的fd_table数组中找到文件描述符对应的元素，这里面存的就是全局打开文件结构数组下标。
5. sys_close用传入的文件描述符关闭文件，原理：调用fd_close2global将文件描述符转换为全局打开文件结构数组下标，然后调用file_close关闭这个文件结构对应的文件，最后清空pcb中的文件描述符位
