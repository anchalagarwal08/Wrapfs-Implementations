/*
 * Copyright (c) 1998-2011 Erez Zadok
 * Copyright (c) 2009	   Shrikar Archak
 * Copyright (c) 2003-2011 Stony Brook University
 * Copyright (c) 2003-2011 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*  			EXTRA_CREDIT_1
 * Added New- Debug option enabled in the respective function
 * two printk enabled in each function defined in ifdef EXTRA_CREDIT_1
 */
  
/*  			EXTRA_CREDIT_2
 *  Code is commented for decoding of filename. it is not implemented fully
 * as described in readme.  
 */
  
#include "wrapfs.h"

static ssize_t wrapfs_read(struct file *file, char __user *buf,
			   size_t count, loff_t *ppos)
{
  int err;
  struct file *lower_file;
  struct dentry *dentry = file->f_path.dentry;
  lower_file = wrapfs_lower_file(file);
	
#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"Debug_wrapfs_read_file");
#endif
	
  err = vfs_read(lower_file, buf, count, ppos);
  /* update our inode atime upon a successful lower read */
  if (err >= 0)
    fsstack_copy_attr_atime(dentry->d_inode,
			    lower_file->f_path.dentry->d_inode);
	
#ifdef EXTRA_CREDIT_1				
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"err : %d", err);
#endif
	
  return err;
}

static ssize_t wrapfs_write(struct file *file, const char __user *buf,
			    size_t count, loff_t *ppos)
{
  int err = 0;
  struct file *lower_file;
  struct dentry *dentry = file->f_path.dentry;
  lower_file = wrapfs_lower_file(file);
	
#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"Debug_wrapfs_write_file");
#endif
	
  err = vfs_write(lower_file, buf, count, ppos);
  /* update our inode times+sizes upon a successful lower write */
  if (err >= 0) {
    fsstack_copy_inode_size(dentry->d_inode,
			    lower_file->f_path.dentry->d_inode);
    fsstack_copy_attr_times(dentry->d_inode,
			    lower_file->f_path.dentry->d_inode);
  }
	
#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"err : %d", err);
#endif
  return err;
}

/* #ifdef EXTRA_CREDIT_2

   struct wrapfs_getdents_callback {
   void *dirent;
   struct dentry *dentry;
   filldir_t filldir;
   int err;
   int filldir_called;
   int entries_written;
   };*/

/*
  wrapfs_filldir(void *dirent, const char *name, int namlen, loff_t offset, u64 ino, unsigned int d_type)

  {
  struct wrapfs_getdents_callback *buf = (struct wrapfs_getdents_callback *) dirent;
  int err;
  char *key = "59c95189ac895fcc1c6e1c38d067e244";
  #ifdef EXTRA_CREDIT_2
  char *decoded_name;
  int decoded_length;
  #endif 
	
  buf->filldir_called++;*/
/*
  #ifdef FIST_FILTER_SCA
	
  if (namlen > INDEX_EXTENSION_LEN-1 &&
  strncmp(&name[namlen-INDEX_EXTENSION_LEN+1],
  INDEX_EXTENSION,
  INDEX_EXTENSION_LEN-1) == 0) {
  return 0;
  }
  #endif
*/
	
/*#ifdef EXTRA_CREDIT_2
//namlen = strlen(name);
decoded_length = cryptfs_decode_filename(name,namlen,decoded_name,key);
if (decoded_length < 0) {
return 0;			
}

err = buf->filldir(buf->dirent, decoded_name, decoded_length, offset, ino, d_type);
kfree(decoded_name);
#else 
err = buf->filldir(buf->dirent, name, namlen, offset, ino, d_type);
#endif 
if (err >= 0) {
buf->entries_written++;
}
	
out:

return err;
}
#endif 
*/

static int wrapfs_readdir(struct file *file, void *dirent, filldir_t filldir)
{
  int err = 0;
  struct file *lower_file = NULL;
  struct dentry *dentry = file->f_path.dentry;
  /*#ifdef EXTRA_CREDIT_2
    struct wrapfs_getdents_callback buf;
    #endif*/
  lower_file = wrapfs_lower_file(file);
  //#ifdef EXTRA_CREDIT_2
  /* prepare for callback */
  /*buf.dirent = dirent;
    buf.dentry = file->f_dentry;
    buf.filldir = filldir;
    retry:
    buf.filldir_called = 0;
    buf.entries_written = 0;
    buf.err = 0;
    err = vfs_readdir(lower_file, wrapfs_filldir, (void *) &buf);
    if (buf.err) {
    err = buf.err;
    }
    if (buf.filldir_called && !buf.entries_written) {
    goto retry;
    }*/	
#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"Debug_wrapfs_readdir");
#endif
	
  err = vfs_readdir(lower_file, filldir, dirent);
  file->f_pos = lower_file->f_pos;
  if (err >= 0)		/* copy the atime */
    fsstack_copy_attr_atime(dentry->d_inode,
    lower_file->f_path.dentry->d_inode);
	
#ifdef EXTRA_CREDIT_1				
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"err : %d", err);
#endif
	
  return err;
}

 static long wrapfs_unlocked_ioctl(struct file *file, unsigned int cmd,
    unsigned long arg)
 {

  long err = -ENOTTY;
  struct file *lower_file;
  char *user_key, *chksum;
  struct wrapfs_sb_info *sbi;
  int i,zero_count =0;
  int len = 0;
	
#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"Debug_wrapfs_unlocked_ioctl");
#endif

#ifdef WRAPFS_CRYPTO
  if(WRAPFS_IOCSETD == cmd)
    {
  printk("IOCTL set command passed \n");
  if(NULL == (char *)arg) {
  printk("Null key cannot be passed \n");
  return -EACCES;
}
  len = strlen((char *)arg);
  user_key = (char *)kmalloc(len, GFP_KERNEL);
  if (user_key == NULL) {
  printk("emem err for val\n");
  err =  -ENOMEM;
  goto out;
}
	    
  memset(user_key, 0 ,len);
  user_key = getname((char*)arg);
  len = strlen(user_key);
  for(i=0;i<len;i++) {
  if(user_key[i]!='0'){
  zero_count =1;
  break;
}
}
  if(zero_count == 0) {
  printk("Key is revoked/reset. Setting the key to NULL \n");
  sbi = (struct wrapfs_sb_info*)(file->f_path.dentry->d_sb->s_fs_info);
  sbi->sb_key = NULL;
  err =0;
  kfree(user_key);    
  goto out;
}

  chksum = kmalloc(2*MD5_SIGNATURE_SIZE+1, GFP_KERNEL);
  if(chksum == NULL)
    {   
  printk("wrapfs_setxattr : ERROR in memory allocation\n");
  err = -ENOMEM;
  kfree(user_key);
  kfree(chksum);
  goto out;
}   
  memset(chksum,0,2*MD5_SIGNATURE_SIZE+1);
  err = checksum(user_key,chksum);
  if(err<0) {
  kfree(user_key);
  kfree(chksum);
  printk("error computing hash value for key, error : %ld",err);
  goto out;
}   
	    
  sbi = (struct wrapfs_sb_info*)(file->f_path.dentry->d_sb->s_fs_info);	    
if(NULL == sbi->sb_key)
{
    //printk("\n Setting new key for super block which was either revoked earlier or is being set for thr first time");
    sbi->sb_key = (unsigned char*)kmalloc(2*MD5_SIGNATURE_SIZE+1 ,GFP_KERNEL);
    memcpy(sbi->sb_key, chksum, 2*MD5_SIGNATURE_SIZE+1);
    printk("\n:%s", sbi->sb_key);
    kfree(user_key);
    kfree(chksum);
    goto out;
} 

if(NULL != sbi && NULL!= sbi->sb_key)
{
    printk("\n Updating the existing key");
    memcpy(sbi->sb_key, chksum, 2*MD5_SIGNATURE_SIZE+1);
    kfree(user_key);
    kfree(chksum);
    goto out;
}
}
#endif
  lower_file = wrapfs_lower_file(file);

  /* XXX: use vfs_ioctl if/when VFS exports it */
  if (!lower_file || !lower_file->f_op)
    goto out;
  if (lower_file->f_op->unlocked_ioctl)
    err = lower_file->f_op->unlocked_ioctl(lower_file, cmd, arg);

 out:

#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"err : %ld", err);
#endif	

  return err;
}

#ifdef CONFIG_COMPAT
 static long wrapfs_compat_ioctl(struct file *file, unsigned int cmd,
    unsigned long arg)
 {
  long err = -ENOTTY;
  struct file *lower_file;

  lower_file = wrapfs_lower_file(file);
	
#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"Debug_wrapfs_compat_ioctl");
#endif
	
  /* XXX: use vfs_ioctl if/when VFS exports it */
  if (!lower_file || !lower_file->f_op)
    goto out;
  if (lower_file->f_op->compat_ioctl)
    err = lower_file->f_op->compat_ioctl(lower_file, cmd, arg);

 out:
#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"err : %d", err);
#endif	
  return err;
}
#endif

 static int wrapfs_mmap(struct file *file, struct vm_area_struct *vma)
 {
  int err = 0;
  bool willwrite;
  struct file *lower_file;
  const struct vm_operations_struct *saved_vm_ops = NULL;

#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"Debug_wrapfs_mmap");
#endif
	
  /* this might be deferred to mmap's writepage */
  willwrite = ((vma->vm_flags | VM_SHARED | VM_WRITE) == vma->vm_flags);

  /*
   * File systems which do not implement ->writepage may use
   * generic_file_readonly_mmap as their ->mmap op.  If you call
   * generic_file_readonly_mmap with VM_WRITE, you'd get an -EINVAL.
   * But we cannot call the lower ->mmap op, so we can't tell that
   * writeable mappings won't work.  Therefore, our only choice is to
   * check if the lower file system supports the ->writepage, and if
   * not, return EINVAL (the same error that
   * generic_file_readonly_mmap returns in that case).
   */
  lower_file = wrapfs_lower_file(file);
  if (willwrite && !lower_file->f_mapping->a_ops->writepage) {
  err = -EINVAL;
  printk(KERN_ERR "wrapfs: lower file system does not "
    "support writeable mmap\n");
  goto out;
}

  /*
   * find and save lower vm_ops.
   *
   * XXX: the VFS should have a cleaner way of finding the lower vm_ops
   */
  if (!WRAPFS_F(file)->lower_vm_ops) {
  err = lower_file->f_op->mmap(lower_file, vma);
  if (err) {
  printk(KERN_ERR "wrapfs: lower mmap failed %d\n", err);
  goto out;
}
  saved_vm_ops = vma->vm_ops; /* save: came from lower ->mmap */
  err = do_munmap(current->mm, vma->vm_start,
    vma->vm_end - vma->vm_start);
  if (err) {
  printk(KERN_ERR "wrapfs: do_munmap failed %d\n", err);
  goto out;
}
}

  /*
   * Next 3 lines are all I need from generic_file_mmap.  I definitely
   * don't want its test for ->readpage which returns -ENOEXEC.
   */
  file_accessed(file);
  vma->vm_ops = &wrapfs_vm_ops;
  vma->vm_flags |= VM_CAN_NONLINEAR;

  if(WRAPFS_SB(file->f_dentry->d_sb)->mmap_option_set==1)
    file->f_mapping->a_ops = &wrapfs_mmap_aops;
  else
    file->f_mapping->a_ops = &wrapfs_aops; /* set our aops */
	
  if (!WRAPFS_F(file)->lower_vm_ops) /* save for our ->fault */
    WRAPFS_F(file)->lower_vm_ops = saved_vm_ops;

 out:
#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"err : %d", err);
#endif	
  return err;
}

static int wrapfs_open(struct inode *inode, struct file *file)
{
  int err = 0;
  struct file *lower_file = NULL;
  struct path lower_path;

#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"Debug_wrapfs_open");
#endif
	
  /* don't open unhashed/deleted files */
  if (d_unhashed(file->f_path.dentry)) {
    err = -ENOENT;
    goto out_err;
  }

  file->private_data =
    kzalloc(sizeof(struct wrapfs_file_info), GFP_KERNEL);
  if (!WRAPFS_F(file)) {
    err = -ENOMEM;
    goto out_err;
  }

  /* open lower object and link wrapfs's file struct to lower's */
  wrapfs_get_lower_path(file->f_path.dentry, &lower_path);
  lower_file = dentry_open(lower_path.dentry, lower_path.mnt,
			   file->f_flags, current_cred());
  if (IS_ERR(lower_file)) {
    err = PTR_ERR(lower_file);
    lower_file = wrapfs_lower_file(file);
    if (lower_file) {
      wrapfs_set_lower_file(file, NULL);
      fput(lower_file); /* fput calls dput for lower_dentry */
    }
  } else {
    wrapfs_set_lower_file(file, lower_file);
  }

  if (err)
    kfree(WRAPFS_F(file));
  else
    fsstack_copy_attr_all(inode, wrapfs_lower_inode(inode));
 out_err:

#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"err : %d", err);
#endif
  return err;
}

static int wrapfs_flush(struct file *file, fl_owner_t id)
{
  int err = 0;
  struct file *lower_file = NULL;
/*
#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"Debug_wrapfs_flush");
#endif
*/	
  lower_file = wrapfs_lower_file(file);
  if (lower_file && lower_file->f_op && lower_file->f_op->flush)
    err = lower_file->f_op->flush(lower_file, id);
/*
#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"err : %d", err);
#endif
*/	
  return err;
}

/* release all lower object references & free the file info structure */
static int wrapfs_file_release(struct inode *inode, struct file *file)
{
  struct file *lower_file;
/*	
#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"Debug_wrapfs_file_release");
#endif
*/	
  lower_file = wrapfs_lower_file(file);
  if (lower_file) {
    wrapfs_set_lower_file(file, NULL);
    fput(lower_file);
  }

  kfree(WRAPFS_F(file));
/*	
#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"err : %d", 0);
#endif
*/	
  return 0;
}

static int wrapfs_fsync(struct file *file, loff_t start, loff_t end,
			int datasync)
{
  int err;
  struct file *lower_file;
  struct path lower_path;
  struct dentry *dentry = file->f_path.dentry;

#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"Debug_wrapfs_fsync");
#endif
	
  err = generic_file_fsync(file, start, end, datasync);
  if (err)
    goto out;
  lower_file = wrapfs_lower_file(file);
  wrapfs_get_lower_path(dentry, &lower_path);
  err = vfs_fsync_range(lower_file, start, end, datasync);
  wrapfs_put_lower_path(dentry, &lower_path);
 out:

#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"err : %d", err);
#endif
	
  return err;
}

static int wrapfs_fasync(int fd, struct file *file, int flag)
{
  int err = 0;
  struct file *lower_file = NULL;

#ifdef EXTRA_CREDIT_1
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"Debug_wrapfs_fasync");
#endif
	
  lower_file = wrapfs_lower_file(file);
  if (lower_file->f_op && lower_file->f_op->fasync)
    err = lower_file->f_op->fasync(fd, lower_file, flag);

#ifdef EXTRA_CREDIT_1	
  debug_file_ops(WRAPFS_SB(file->f_dentry->d_sb)->debug_file_ops,"err : %d", err);
#endif
	
  return err;
}

const struct file_operations wrapfs_main_fops = {
  .llseek		= generic_file_llseek,
  .read		= wrapfs_read,
  .write		= wrapfs_write,
  .unlocked_ioctl	= wrapfs_unlocked_ioctl,
#ifdef CONFIG_COMPAT
  .compat_ioctl	= wrapfs_compat_ioctl,
#endif
  .mmap		= wrapfs_mmap,
  .open		= wrapfs_open,
  .flush		= wrapfs_flush,
  .release	= wrapfs_file_release,
  .fsync		= wrapfs_fsync,
  .fasync		= wrapfs_fasync,
};

const struct file_operations wrapfs_main_mmap_fops = { 
  .llseek         = generic_file_llseek,
  .read           = do_sync_read,
  .write          = do_sync_write,
  .aio_read  =  generic_file_aio_read,
  .aio_write = generic_file_aio_write,
  .unlocked_ioctl = wrapfs_unlocked_ioctl,
#ifdef CONFIG_COMPAT
  .compat_ioctl   = wrapfs_compat_ioctl,
#endif
  .mmap           = generic_file_mmap,
  .open           = wrapfs_open,
  .flush          = wrapfs_flush,
  .release        = wrapfs_file_release,
  .fsync          = wrapfs_fsync,
  .fasync         = wrapfs_fasync,
};

/* trimmed directory options */
const struct file_operations wrapfs_dir_fops = {
  .llseek		= generic_file_llseek,
  .read		= generic_read_dir,
  .readdir	= wrapfs_readdir,
  .unlocked_ioctl	= wrapfs_unlocked_ioctl,
#ifdef CONFIG_COMPAT
  .compat_ioctl	= wrapfs_compat_ioctl,
#endif
  .open		= wrapfs_open,
  .release	= wrapfs_file_release,
  .flush		= wrapfs_flush,
  .fsync		= wrapfs_fsync,
  .fasync		= wrapfs_fasync,
};
