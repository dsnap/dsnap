#ifndef _LOKI_H_
#define _LOKI_H
 
#include <linux/debugfs.h>
struct loki{
  char *name;
  struct dentry *entry;
  struct loki_file *first_child;
};

struct loki_file {
  char *name;
  struct dentry *entry;
  struct debugfs_blob_wrapper *blob;
  struct debugfs_blob_wrapper *blob_watch;  
  struct loki_file *next;
};

//extern struct loki *loki_root;  
extern void loki_init(char *);
extern void loki_create_blob(char *name, void *loc,int size);
extern void loki_create_blob_watch(char *name, void *loc,int size);
extern void loki_create_test_binary(char *name,void *loc, int size);
extern void loki_cleanup();
#endif
