#include "loki.h"
#include "e1000.h"

struct loki *loki_root=NULL;

static struct loki_file *create_file(char * name);



void loki_init(char *name) {
  loki_root = kmalloc(sizeof(struct loki), GFP_KERNEL);
  loki_root->entry = debugfs_create_dir(name,NULL);
  loki_root->name = kstrdup(name,GFP_KERNEL);  
  loki_root->first_child = NULL;
 }


static struct loki_file * create_file(char *name){
  struct loki_file *lfile,*curr,*prev;
  lfile = kmalloc(sizeof(struct loki_file),GFP_KERNEL);

  lfile->next =NULL;
  lfile->name = kstrdup(name,GFP_KERNEL); 
  if (loki_root->first_child != NULL){
    curr= loki_root->first_child;
    prev= curr;
    printk("Start\n");
    while(curr!=NULL){
      prev = curr;
      curr=curr->next;
      printk("Next!!\n");
    }
     prev->next = lfile;
  } else{
    loki_root->first_child = lfile;
  }
  return lfile;
}


void loki_create_test_binary(char *name,void *loc, int size){
  
  u32 start=0xdeadbeef;
  u32 end=0xfeebdaed;
  int m_size=sizeof(u32)*4+size+strlen(name);

  u32 * buffer = kmalloc(m_size,GFP_KERNEL);
  char *c_buffer = buffer;
  buffer[0] = start;
  buffer[1] = strlen(name);
  memcpy(buffer+2,name,strlen(name));
  memcpy(c_buffer+8+strlen(name),loc,size); 
  loki_create_blob_watch("new_binary_test",buffer,m_size);

  c_buffer = c_buffer+(m_size-4);
  *(u32 *)c_buffer =end; 
}

void loki_create_blob_watch(char *name, void *loc, int size){
  struct debugfs_blob_wrapper *blob;
  struct loki_file *lfile;
  
  lfile = create_file(name);
 
  blob = kmalloc(sizeof(struct debugfs_blob_wrapper),GFP_KERNEL);
    lfile->entry=debugfs_create_blob(name,S_IRUGO,
			   loki_root->entry,
			    blob);
  lfile->blob_watch = blob;
  lfile->blob = NULL;
  blob->data = loc;;
  blob->size = size;
}

void loki_create_blob(char *name, void *loc, int size){
  struct debugfs_blob_wrapper *blob;
  struct loki_file *lfile;
  void *data;

  lfile = create_file(name);
 
  blob = kmalloc(sizeof(struct debugfs_blob_wrapper),GFP_KERNEL);
  data = kmalloc(size,GFP_KERNEL);
 
  lfile->entry=debugfs_create_blob(name,S_IRUGO,
			   loki_root->entry,
			    blob);
  lfile->blob = blob;
  lfile->blob_watch = NULL;
  memcpy(loc,data,size); 
  blob->data = data;
  blob->size = size;
 
}
void loki_cleanup() {
  
  kfree(loki_root->name);
  printk("Cleaning up Loki!!!\n");
  
  struct loki_file *curr;
  struct loki_file *del;
  curr = loki_root->first_child;
  while(curr!=NULL){
    printk("Deleteing %s\n",curr->name);
    del = curr;
    debugfs_remove(del->entry);
    kfree(del->name);
    //Delete blob info if it is a blob node
    if(del->blob) {
      kfree(del->blob->data);
      kfree(del->blob);
    }
    else if(del->blob_watch) {
      kfree(del->blob);
    }

    curr=curr->next;
    kfree(del);
  }
    
  debugfs_remove(loki_root->entry);
  kfree(loki_root);
  loki_root = NULL;

}
