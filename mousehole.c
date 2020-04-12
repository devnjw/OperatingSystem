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

char filepath[128] = { 0x0,} ;
void ** sctable ;
bool hidden = false;
int reject_count = 0;
int passed_uid = -1;
char user_name[100] ;
struct list_head* head;
asmlinkage long (*orig_sys_kill)(pid_t pid, int sig) ;
asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ;

/*Do mousehole_sys_kill instead of sys_kill
	If user didn't put user name 
*/
asmlinkage long mousehole_sys_kill(pid_t pid, int sig) {
	passed_uid = current->cred->uid.val;

	//if the uid is 1004, protect the processes from killing
	if(passed_uid == 1004){
		printk("Prevent killing processes created by %d.\n ", passed_uid);
		return -1;
	}

	//otherwise, original system call is executed
	printk("Kill processes created by %d.\n ", passed_uid);
	return orig_sys_kill(pid, sig);
}

//Do mousehole_sys_open instead of sys_open
asmlinkage int mousehole_sys_open(const char __user * filename, int flags, umode_t mode){
	char fname[256] ;
    char dontopen[256] = "dontopen.txt";
	passed_uid = current->cred->uid.val;

	copy_from_user(fname, filename, 256) ;

    //User is notyou
    if(passed_uid == 1004){
		//notyou try to open dontopen.txt
		if (strstr(fname, dontopen) != NULL ){
        	if (filepath[0] != 0x0 && strcmp(filepath, fname) == 0) {
            	printk("uid %d can't open %s. Use other user_name\n\n", passed_uid, fname);
				reject_count++;
				return -1;
        	}
		//notyou try to open other file
    	}else{
			printk("uid %d open %s successfully!\n", passed_uid, fname);
        }
    }
    //User is someone else
    else{
		printk("uid %d open %s successfully!\n", passed_uid, fname);
    }

	//Return to original system open
	return orig_sys_open(filename, flags, mode) ;
}


static int mousehole_proc_open(struct inode *inode, struct file *file) {
	return 0 ;
}

static int mousehole_proc_release(struct inode *inode, struct file *file) {
	return 0 ;
}

static ssize_t mousehole_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset){
	char buf[256] ;
	ssize_t toread ;

	//sprintf(buf, "%s:%d\n", filepath, count) ;
    if(reject_count > 0){
        sprintf(buf, "%d can't open %s\n", passed_uid, filepath) ;
    }

	toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;

	if (copy_to_user(ubuf, buf + *offset, toread)){
		return -EFAULT ;	
	}

	*offset = *offset + toread ;

	return toread ;
}

static ssize_t mousehole_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset){
    char buf[256];
    char copy[256];
    char *str;
    char *space = " ";
    char *parm1, *parm2, *parm3;	

        if (*offset != 0 || size > 128){
		return -EFAULT ;
	}

	if (copy_from_user(buf, ubuf, size)){
		return -EFAULT;
	}

	printk("Successfully receive the data\n");
        
        sscanf(buf, "%s", copy);
        str = copy;
        
        parm1 = strsep(&str, space);
        parm2 = strsep(&str, space);
        parm3 = strsep(&str, space);        

	if(strcmp(parm2, "one")){
		strcpy(filepath, parm3);
		strcpy(user_name, parm1);
	
	}else if(strcmp(parm2, "two")){
		strcpy(user_name, parm1);
        }
	
	*offset = strlen(buf) ;

	return *offset ;

}

static const struct file_operations mousehole_fops = {
	.owner = 	THIS_MODULE,
	.open = 	mousehole_proc_open,
	.read = 	mousehole_proc_read,
	.write = 	mousehole_proc_write,
	.llseek = 	seq_lseek,
	.release = 	mousehole_proc_release,
} ;

static int __init mousehole_init(void) {
	unsigned int level ;
	pte_t * pte ;

	proc_create("mousehole", S_IRUGO | S_IWUGO, NULL, &mousehole_fops) ;

	//Get the system call table
	sctable = (void *) kallsyms_lookup_name("sys_call_table") ;

	//Save original system calls 
	orig_sys_open = sctable[__NR_open] ;
	orig_sys_kill = sctable[37];

	pte = lookup_address((unsigned long) sctable, &level) ;
	if (pte->pte &~ _PAGE_RW)
		pte->pte |= _PAGE_RW ;

	//Replace the system call with my functions
	sctable[__NR_open] = mousehole_sys_open ;
	sctable[37] = mousehole_sys_kill ;

	return 0;
}

static void __exit mousehole_exit(void) {
	unsigned int level ;
	pte_t * pte ;
	remove_proc_entry("mousehole", NULL) ;

	//Replace the system calls with the original 
	sctable[__NR_open] = orig_sys_open ;
	sctable[37] = orig_sys_kill ;

	pte = lookup_address((unsigned long) sctable, &level) ;
	pte->pte = pte->pte &~ _PAGE_RW ;

}

module_init(mousehole_init);
module_exit(mousehole_exit);


