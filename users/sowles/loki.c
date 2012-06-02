#include "loki.h"
#include "e1000.h"

// TODO: Global next_slot variable
// TODO: Global loki_dir variable
// TODO: Error checking

//// GLOBAL VARIABLES ////

int next_slot = 0;
struct loki_dir *ldir = NULL;

//// PROTOTYPES ////

struct loki_file *loki_create_loki_file(char *name);
struct dentry *loki_create_blob(char *name);
int loki_create_loki_blob(char *name, void *location, int size);
struct loki_blob *loki_find_loki_blob(char *name);

//// FUNCTIONS ////

/**
 * Initializes the Loki framework.
 * @name: the name of the root directory in /debug
 */
void loki_init(char *dir_name, char *file_name)
{
	// TODO: If ENODEV returned from create_dir call, debugfs not in kernel
	// TODO: Check for successful completion of operations
	printk("Loki: Initializing...\n");

	if (!(ldir = kmalloc(sizeof(struct loki_dir), GFP_KERNEL)))
	{
		printk("Loki: Unable to allocate memory for Loki directory '%s'.\n", dir_name);
		return;
	}

  	ldir->name = kstrdup(dir_name, GFP_KERNEL);
	
	if (!(ldir->entry = debugfs_create_dir(dir_name, NULL)))
	{
		printk("Loki: Unable to create Loki directory '%s'.\n", dir_name);
		return;
	}

  	if (!(ldir->lfile = loki_create_loki_file(file_name)))
	{
		printk("Loki: Unable to create Loki file '%s'.\n", file_name);
		return;
	}

	if (!(ldir->lfile->entry = loki_create_blob(file_name)))
	{
		printk("Loki: Unable to create blob '%s' (dentry is NULL).\n", file_name);
		return;
	}

	printk("Loki: Initialization complete.\n");
}

/**
 * Creates a Loki file.
 * @name: the name of the file to create
 * @return: a pointer to the created file
 */
struct loki_file *loki_create_loki_file(char *name)
{
	struct loki_file *lfile;

	printk("Loki: Creating Loki file '%s'...\n", name);
  	
	if (!(lfile = kmalloc(sizeof(struct loki_file), GFP_KERNEL)))
	{
		printk("Loki: Unable to allocate memory for Loki file '%s'.\n", name);
		return NULL;
	}

  	lfile->name = kstrdup(name, GFP_KERNEL); 
	lfile->entry = NULL;
	lfile->lblob = NULL;

	printk("Loki: Loki file created.\n");
 
	return lfile;
}

/**
 * Creates a blob that holds driver data.
 * @name: the name of the blob
 * @return: a pointer to the dentry object returned by the
 *			debugfs_create_blob call
 */
struct dentry *loki_create_blob(char *name)
{
  	struct debugfs_blob_wrapper *blob;
	struct dentry *entry;  

	printk("Loki: Creating blob '%s'...\n", name);
  	
	if (!(blob = kmalloc(sizeof(struct debugfs_blob_wrapper), GFP_KERNEL)))
	{
		printk("Loki: Unable to allocate memory for blob '%s'.\n", name);
		return NULL;
	}

	if (!(entry = debugfs_create_blob(name, S_IRUGO, ldir->entry, blob)))
	{
		printk("Loki: Unable to create blob '%s' (dentry is NULL).\n", name);
		return NULL;
	}
  	
	if (!blob)
	{
		printk("Loki: Unable to create blob '%s' (blob is NULL).\n", name);
		return NULL;
	}

	printk("FLAG 1\n");

	blob->data = NULL;
	printk("FLAG 2\n");
  	blob->size = sizeof(blob);
	printk("FLAG 3\n");

	ldir->lfile->blob = blob;

	printk("Loki: Blob created.\n");

	return entry;
}

/**
 * Adds data to the blob.
 * @name: the name of the data to add
 * @location: the memory location of the data to add
 * @size: the size of the data to add
 */
void loki_add_to_blob(char *name, void *location, int size)
{
	struct loki_blob *lblob;
	struct debugfs_blob_wrapper *blob;

	printk("Loki: Adding '%s' to blob...\n", name);

	// Data has not been added to blob yet, so add it
	if (!(lblob = loki_find_loki_blob(name)))
	{
		if ((loki_create_loki_blob(name, location, size)) == -1)
		{
			printk("Loki: Unable to create Loki blob '%s'.\n", name);
			return;
		}
	}

	// Data exists in blob, so update it
	else
	{
		printk("Loki: Updating blob...\n");

		blob = ldir->lfile->blob;
		// TODO: sizeof argument may change (sizeof lblob->blob->data maybe?)
		memcpy(&blob[lblob->start], &(lblob->blob), sizeof(lblob->blob));
	}
}

/**
 * Creates a Loki blob.
 * @name: the name of the Loki blob
 * @location: the location of the data to store
 * @size: the size of the data to store
 * @return: -1 if an error occurs; 0 otherwise
 */
int loki_create_loki_blob(char *name, void *location, int size)
{
	struct loki_blob *lblob, *curr;

	printk("Loki: Creating Loki blob '%s'...\n", name);
  	
	if (!(lblob = kmalloc(sizeof(struct loki_blob), GFP_KERNEL)))
	{
		printk("Loki: Unable to allocate memory for Loki blob '%s'.\n", name);
		return -1;
	}
	
	lblob->name = name;
	lblob->start = next_slot;
	lblob->end = lblob->start + size;
	lblob->blob = location;

	next_slot += size;

	// Add new Loki blob to the list
	curr = ldir->lfile->lblob;

	while (curr)
	{
		curr = curr->next;
	}

	curr = lblob;

	printk("Loki: Loki blob created.\n");

	return 0;
}

/**
 * Finds a specified Loki blob.
 * @name: the name of the Loki blob to find
 * @return: a pointer to the Loki blob if found, else null
 */
struct loki_blob *loki_find_loki_blob(char *name)
{
	struct loki_blob *curr;
	
	printk("Loki: Searching for Loki blob '%s'...\n", name);

	curr = ldir->lfile->lblob;

	while (curr)
	{
		if (strcmp(name, curr->name))
		{
			return curr;
		}

		curr = curr->next;
	}

	return NULL;
}

/**
 * Properly disposes of Loki resources.
 */
void loki_cleanup(void)
{
	// TODO: This hasn't been done yet.  
  	printk("Loki: Cleaning up...\n");
  	
	/*kfree(ldir->name);
  
  	struct loki_file *curr;
  	struct loki_file *del;
  	curr = loki_root->first_child;
  
	while (curr != NULL)
	{
    	printk("Deleting %s\n", curr->name);
    	del = curr;
    	debugfs_remove(del->entry);
    	kfree(del->name);
    
		//Delete blob info if it is a blob node
    	if (del->blob)
		{
    		kfree(del->blob->data);
    		kfree(del->blob);
    	}
    
		else if (del->blob_watch)
		{
      		kfree(del->blob);
    	}

   		curr = curr->next;
    	kfree(del);
  	}*/
    
  	debugfs_remove(ldir->entry);
  	kfree(ldir);
  	ldir = NULL;
	
	printk("Loki: Cleanup complete.\n");
}
