#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/unistd.h>
#include <linux/cred.h>
#include <linux/sched.h>
#include <linux/string.h>
#define _CRT_SECURE_NO_WARNINGS

MODULE_LICENSE("GPL");

char filepath[128] = {
	0x0,
};
void **sctable;
bool hidden = false;
int pass_uid;
int cur_uid;
int option;
int reject_count = 0;
int killed_count = 0;
char user_name[100];
struct list_head *head;
asmlinkage long (*orig_sys_kill)(pid_t pid, int sig);
asmlinkage int (*orig_sys_open)(const char __user *filename, int flags, umode_t mode);

//Do mousehole_sys_kill instead of sys_kill
asmlinkage long mousehole_sys_kill(pid_t pid, int sig)
{
	cur_uid = current->cred->uid.val;

	if (option != 4)
	{
		if (pass_uid == cur_uid)
		{
			printk("Prevent killing processes created by %d.\n ", pass_uid);
			killed_count++;

			return -1;
		}
	}

	//return to original system call is executed
	return orig_sys_kill(pid, sig);
}

//Do mousehole_sys_open instead of sys_open
asmlinkage int mousehole_sys_open(const char __user *filename, int flags, umode_t mode)
{
	char fname[256];

	cur_uid = current->cred->uid.val;

	copy_from_user(fname, filename, 256);

	if (option != 3)
	{
		if (pass_uid == cur_uid)
		{
			if (strstr(filepath, fname) && filepath[0] != 0x0)
			{
				printk("uid %d can't open %s. Use other user name or filepath\n\n", pass_uid, fname);
				reject_count++;
				return -1;
			}
		}
	}

	//Return to original system open
	return orig_sys_open(filename, flags, mode);
}

static int mousehole_proc_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int mousehole_proc_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t mousehole_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset)
{
	char buf[256];
	ssize_t toread;

	if (reject_count > 0)
	{
		sprintf(buf, "uid %d is rejected to open %s\n", pass_uid, filepath);
	}

	if (killed_count > 0)
	{
		sprintf(buf, "pid %d is prevented from killed with %d uid\n", pass_uid);
	}

	toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset;

	if (copy_to_user(ubuf, buf + *offset, toread))
		return -EFAULT;

	*offset = *offset + toread;

	return toread;
}

static ssize_t mousehole_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset)
{
	char buf[256];

	if (*offset != 0 || size > 128)
	{
		return -EFAULT;
	}

	if (copy_from_user(buf, ubuf, size))
	{
		return -EFAULT;
	}

	printk("Successfully received the data.\n");

	switch (buf[0])
	{
	//Do mousehole_sys_open()
	case '1':
		sscanf(buf, "%d %d %s", &option, &pass_uid, filepath);
		printk("uid %d will open %s file.\n", pass_uid, filepath);
		break;
	//Do mousehole_sys_kill()
	case '2':
		sscanf(buf, "%d %d", &option, &pass_uid);
		break;
	//return sys_open()
	case '3':
		sscanf(buf, "%d", &option);
		break;
	//return sys_kill()
	case '4':
		sscanf(buf, "%d", &option);
		break;
	}

	reject_count = 0;
	killed_count = 0;

	*offset = strlen(buf);
	return *offset;
}

static const struct file_operations mousehole_fops = {
	.owner = THIS_MODULE,
	.open = mousehole_proc_open,
	.read = mousehole_proc_read,
	.write = mousehole_proc_write,
	.llseek = seq_lseek,
	.release = mousehole_proc_release,
};

static int __init mousehole_init(void)
{
	unsigned int level;
	pte_t *pte;

	proc_create("mousehole", S_IRUGO | S_IWUGO, NULL, &mousehole_fops);

	//Get the system call table
	sctable = (void *)kallsyms_lookup_name("sys_call_table");

	//Save original system calls
	orig_sys_open = sctable[__NR_open];
	orig_sys_kill = sctable[37];

	pte = lookup_address((unsigned long)sctable, &level);
	if (pte->pte & ~_PAGE_RW)
		pte->pte |= _PAGE_RW;

	//Replace the system call with my functions
	sctable[__NR_open] = mousehole_sys_open;
	sctable[37] = mousehole_sys_kill;

	return 0;
}

static void __exit mousehole_exit(void)
{
	unsigned int level;
	pte_t *pte;
	remove_proc_entry("mousehole", NULL);

	//Replace the system calls with the original
	sctable[__NR_open] = orig_sys_open;
	sctable[37] = orig_sys_kill;

	pte = lookup_address((unsigned long)sctable, &level);
	pte->pte = pte->pte & ~_PAGE_RW;
}

module_init(mousehole_init);
module_exit(mousehole_exit);
