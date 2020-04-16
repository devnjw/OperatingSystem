#include <linux/syscalls.h>
#include <linux/seq_file.h>
#include <linux/kallsyms.h>
#include <asm/unistd.h>

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>   
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include <linux/cred.h>
#include <linux/sched.h>

#define _CRT_SECURE_NO_WARNINGS
#define BUFSIZE 256 

MODULE_LICENSE("GPL");

int userid_b = -1 ; //user id to block opening file
int userid_p = -1 ; //user id to prevent from killing

char filepath[100] = { 0x0, };
void ** sctable ;

asmlinkage long (*orig_sys_kill)(pid_t pid, int sig);
asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode);

asmlinkage long mousehole_sys_kill(pid_t pid, int sig)
{
	struct task_struct * t ;
	uid_t puid;
	
	for_each_process(t){
		puid = t->real_cred->uid.val;
		if(pid == t->pid && puid == userid_p){
			printk("mousehole: Killing process '%d' is prevented\n",pid);
			return 0;
		}
	}

	return orig_sys_kill(pid, sig);
}

asmlinkage int mousehole_sys_open(const char __user * filename, int flags, umode_t mode)
{
	char fname[256];
	int curr_id = current->cred->uid.val;
	
	copy_from_user(fname, filename, 256);

	if(filepath[0] != 0x0 && strstr(fname, filepath) != NULL){
		if(curr_id == userid_b){
			printk("mousehole: Fail to open file\n");
			return -1;
		}
	}
	return orig_sys_open(filename, flags, mode);
}

static
int mousehole_proc_open(struct inode *inode, struct file *file) {
	return 0;
}

static
int mousehole_proc_release(struct inode *inode, struct file *file){
	return 0;
}

static ssize_t mousehole_proc_write(struct file *file, const char __user *ubuf,size_t size, loff_t *offset) 
{
	char buf[BUFSIZE];
	int opt;
	
	if(*offset != 0 || size > BUFSIZE)
		return -EFAULT;
	
	if(copy_from_user(buf,ubuf,size))
		return -EFAULT;

	if(buf[0]=='1')	
		sscanf(buf,"%d %d %s",&opt, &userid_b, filepath);
	else if(buf[0]=='2')
		sscanf(buf,"%d %d", &opt, &userid_p);

	*offset = strlen(buf);
	
	return *offset;
}

static ssize_t mousehole_proc_read(struct file *file, char __user *ubuf,size_t size, loff_t *offset) 
{
	char buf[BUFSIZE];
	ssize_t toread ;
	
	sprintf(buf, "\n---Mousehole Status---\nBlocked Uid: %d File: %s\nKill Prevented Uid: %d\n", userid_b, filepath, userid_p);	

	toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;
	
	if(copy_to_user(ubuf,buf + *offset, toread))
		return -EFAULT;
	
	*offset = *offset + toread ;
	
	return toread ;
}

static const struct file_operations myops = 
{
	.owner = THIS_MODULE,
	.open = mousehole_proc_open,
	.read = mousehole_proc_read,
	.write = mousehole_proc_write,
	.llseek = seq_lseek,
	.release = mousehole_proc_release,
};

static
int __init mousehole_init(void)
{
	unsigned int level ;
	pte_t * pte ;

	proc_create("mousehole", S_IRUGO | S_IWUGO, NULL, &myops);
	printk("mousehole: hello...\n");
	
	sctable = (void *) kallsyms_lookup_name("sys_call_table") ;
	
	orig_sys_open = sctable[__NR_open] ;
	orig_sys_kill = sctable[__NR_kill];

	pte = lookup_address((unsigned long) sctable, &level) ;
	if(pte->pte &~ _PAGE_RW)
		pte->pte |= _PAGE_RW ;
	
	sctable[__NR_open] = mousehole_sys_open ;
	sctable[__NR_kill] = mousehole_sys_kill;
	return 0;
}

static
void __exit mousehole_exit(void)
{
	unsigned int level;
	pte_t * pte ;
	remove_proc_entry("mousehole", NULL) ;
	
	printk("mousehole: bye ...\n");

	sctable[__NR_open] = orig_sys_open ;
	sctable[__NR_kill] = orig_sys_kill;

	pte = lookup_address((unsigned long) sctable, &level) ;
	pte->pte = pte->pte &~ _PAGE_RW ;
}

module_init(mousehole_init);
module_exit(mousehole_exit);
