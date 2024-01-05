# 操作系统真相还原第14章：文件系统
## 实验小节c：
**这一小节主要为了能够在磁盘中添加文件,因此对文件系统中的各个结构进行完善和修改:**
1. Inode结构:添加了初始化inode与打开关闭inode函数,还添加了inode_locate函数获取inode节点的扇区相关信息.添加inode_sync函数,用于将inode信息写回inode数组中的对应位置.
2. Dentry结构添加了目录项查找与目录项创建函数,还有目录的打开与关闭
3. File结构添加了文件的创建函数并修改了file结构与各个依赖结构之间的关系.
4. 增加了parent_pid与cwd_inode_nr变量在thread.h头文件中,用于表明当前进程的父进程以及当前进程的工作目录
**具体操作如下：**
1. 修改thread.c添加进程对inode节点的操作:
- 添加inode_locate函数,inode_locate函数负责定位inode所在扇区和扇区内的偏移,通过传入inode在inode数组中的索引来计算.
- 添加inode_sync函数,用于将一个inode写入磁盘中inode数组对应的位置:
- 添加inode_open函数,用于打开一个inode,根据传入的inode索引找到inode并将其加载到内存中.。由于我们在管理分区的结构体struct partition中维护了一个本分区打开文件的inode链表，所以我们优先去这个链表中查找（这个链表在内存中），找不到再去磁盘中找，也就是调用inode_locate解析他在磁盘中的位置，然后读到内存中。由于打开的Inode链表需要对所有进程共享，所有我们存放inode的内存需要去内核堆区申请（因为所有的进程均可访问内核）
- inode_close用于关闭一个inode，也就是从内存中移除一个inode。由于一个inode可以被多次打开，我们需要判断此时是否还有进程/线程打开这个inode，再决定是否真正移除。移除的inode所占用的内存空间是内核的堆空间。
- 添加inode初始化函数inode_init,通过传入一个inode编号来初始化一个inode.
2. 接下来实现与目录相关的函数,包括目录打开,关闭以及在一个目录文件中寻找对应指定的目录项,初始化目录项并将目录项写入父目录中:
- 修改dir.c添加dir_open函数,dir_open用于根据传入的分区与inode偏移，申请一块内存区域存放目录，调用inode_open找到这个目录对应的inode并载入内存，然后让目录中的inode指针指向这个inode。
- 添加search_dir_entry函数,在某个目录内找到指定名称的目录项。核心原理就是目录结构体struct dir 有一个指向自己inode的成员，该成员记录了该目录在磁盘中存储的位置（inode的i_sectors[ ]）我们可以根据这个找到目录文件，然后遍历其中的目录项即可
- 添加dir_close函数
- 添加目录项创建函数
- 添加sync_dir_entry函数,用于将目录项写入父目录中。核心原理就是：父目录结构体struct dir有一个指向自己inode的成员，该成员记录了该父目录文件在磁盘中存储的位置（inode的i_sectors[ ]）我们可以根据这个找到目录文件，然后遍历目录文件找到空位，然后将目录项插入到这个空位即可。有些麻烦的是，需要判断inode的i_sectors[ ]这些元素是否为空，如果为空，那么我们还需要申请块用于承载目录文件。
- 实现路径解析函数path_parse函数:
- 创建path_depth_cnt函数用于返回路径深度，比如/home/kanshan/Desktop/test.c路径深度就是4。原理就是循环调用path_parse，看看返回值是不是空，决定是否继续调用path_parse.
修改dir.c中的sync_dir_entry函数,避免直接块数目达到12个时出错,导致数据丢失
