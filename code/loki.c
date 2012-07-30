#include <linux/err.h>
#include "loki.h"
#include "e1000.h"

//// PROTOTYPES ////

static struct loki_file *loki_create_file(char *name);
static struct dentry *loki_create_record(char *name, struct loki_dir *ldir);
static struct loki_record *loki_find_record(char *name, struct loki_dir *ldir);
static int loki_create_blob(char *name, void *location, int size, struct loki_dir *ldir);
static void loki_construct_blob(struct loki_dir *ldir);

//// FUNCTIONS ////

/**
 * Initializes the Loki framework.
 * @name: the name of the root directory in /debug
 * @bdf_id: the unique bus-device-function identifier of the device
 */
void loki_init(char *dir_name, struct loki_dir *ldir, char *file_name)
{
	struct file *directory;

	printk("Loki: Initializing...\n");

    ldir->name = kstrdup(dir_name, GFP_KERNEL);
	ldir->entry = debugfs_create_dir(dir_name, NULL);

    if (!ldir->entry)
    {
        printk("Loki: Loki directory '%s' already exists.\n", dir_name);

		directory = filp_open("/debug/e1000", O_APPEND, S_IRWXU);
		ldir->entry = directory->f_dentry;

		kfree(directory);

		if (!ldir->entry)
		{
			printk("Loki: Could not open directory '%s'.\n", dir_name);
			return;
		}
    }

	else if (ldir->entry == ERR_PTR(-ENODEV))
	{
		printk("Error: Your kernel must have debugfs support enabled to run Loki.\n");
	}

    if (!(ldir->lfile = loki_create_file(file_name)))
    {
        printk("Loki: Unable to create Loki file '%s'.\n", file_name);
        return;
    }

    if (!(ldir->lfile->entry = loki_create_record(ldir->lfile->name, ldir)))
    {
        printk("Loki: Unable to create blob '%s' (dentry is NULL).\n", ldir->lfile->name);
        return;
    }

    // Set master size
    ldir->lfile->tot_size = (sizeof(u32)*2) + strlen(ldir->name); 
    
	// Allocate memory for master
    if (!(ldir->lfile->master = (u8 *)kmalloc(ldir->lfile->tot_size, GFP_KERNEL)))
    {
        printk("Loki: Unable to allocate memory for Loki master buffer.\n");
        return;
    }
    
    // Construct initial binary structure
    loki_construct_blob(ldir);
     
    printk("Loki: Initialization complete.\n");
}

/**
 * Creates a Loki binary structure from lfiles.
 * @name: the name of the file to create
 * @return: a pointer to the created file
 */
static void loki_construct_blob(struct loki_dir *ldir)
{
    struct loki_record *curr;
    struct loki_file *root = ldir->lfile;
    u8  *c_ptr = root->master;
    int i = 0;
    
	// Zero buffer
    for (i = 0; i < ldir->lfile->tot_size; i++)
    {
	    root->master[i] = 0;
	}

    *((u32 *)c_ptr) = strlen(ldir->name); 	// Should be module name
    c_ptr += sizeof(u32);
    memcpy(c_ptr,ldir->name, strlen(ldir->name));
    c_ptr += strlen(ldir->name); 			// Jump to count of records
    *((u32 *)c_ptr) = root->records;
    c_ptr += sizeof(u32); 					// Jump to first record

    curr = ldir->lfile->lblob; 
    
	while (curr != NULL)
    {
        if (strlen(curr->name) > 0xffffffff)
        {
		   return;
        }

		*((u32 *)c_ptr) = strlen(curr->name);
		c_ptr += sizeof(u32); 
		memcpy(c_ptr,curr->name,strlen(curr->name));
        c_ptr += strlen(curr->name); 		// Jump to count of records
        *((u32 *)c_ptr) = curr->size;
		c_ptr += sizeof(u32); 
		memcpy(c_ptr,curr->loc,curr->size);
        c_ptr+=curr->size;
        curr = curr->next;
    }
    
    // Debugfs needs to know what the location and new size of the blob are.
    root->blob->data = root->master;
    root->blob->size = root->tot_size;
}

/**
 * Creates a Loki file.
 * @name: the name of the file to create
 * @return: a pointer to the created file
 */
static struct loki_file *loki_create_file(char *name)
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

    // Location of master buffer
    lfile->records = 0;
    lfile->tot_size = 0;
    
    return lfile;
}

/**
 * Creates a blob that holds driver data.
 * @name: the name of the blob
 * @return: a pointer to the dentry object returned by the
 *          debugfs_create_blob call
 */
static struct dentry *loki_create_record(char *name, struct loki_dir *ldir)
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

    blob->data = NULL;
    blob->size = 0;

    ldir->lfile->blob = blob;

    printk("Loki: Blob '%s' created.\n", name);

    return entry;
}

/**
 * Adds data to the blob.
 * @name: the name of the data to add
 * @location: the memory location of the data to add
 * @size: the size of the data to add
 */
void loki_add_to_blob(char *name, void *location, int size, struct loki_dir *ldir)
{
    struct loki_record *lblob;

#ifdef DEBUG
    printk("Loki: Adding '%s' to blob...\n", name);
#endif

 	// Data has not been added to blob yet, so add it
    if (!(lblob = loki_find_record(name, ldir)))
    {
        if ((loki_create_blob(name, location, size, ldir)) == -1)
        {
            printk("Loki: Unable to create Loki blob '%s'.\n", name);
            return;
        }
    }

    // Data exists in blob, so update it
    else
    {
#ifdef DEBUG
        printk("Loki: Updating Loki blob '%s'...\n", name);
#endif

        if (size != lblob->size)
        {
            printk("Loki: FAIL passed in size != to size in list");
            return;
        }
        
		// Copy data to offset in blob
        memcpy(ldir->lfile->master + lblob->offset, location, size);

#ifdef DEBUG
        printk("Loki: Loki blob '%s' updated.\n", name);
#endif
    }
}

/**
 * Creates a Loki blob.
 * @name: the name of the Loki blob
 * @location: the location of the data to store
 * @size: the size of the data to store
 * @return: -1 if an error occurs; 0 otherwise
 */
int loki_create_blob(char *name, void *location, int size, struct loki_dir *ldir)
{
    struct loki_record *lblob,*curr;

    int new_size =   size 
                   + ldir->lfile->tot_size
                   + strlen(name) 
                   + (sizeof(u32) * 2); 	// Size and count

    printk("Loki: Creating Loki blob '%s'...\n", name);
    
    if (!(lblob = kmalloc(sizeof(struct loki_record), GFP_KERNEL)))
    {
        printk("Loki: Unable to allocate memory for Loki blob '%s'.\n", name);
        return -1;
    }
    
    lblob->name = kstrdup(name, GFP_KERNEL); 
    lblob->offset = new_size - size;    // Size of record - size of data
    lblob->size = size;                 // Size of data (in bytes)
    lblob->loc = location;              // Address in kernel memory

    // Add new Loki blob to the list
    if (!ldir->lfile->lblob)
    {
        ldir->lfile->lblob = lblob;
    }

    else
    {
        curr = ldir->lfile->lblob;
 
        while (curr && curr->next != NULL)
        {
		    curr = curr->next;
        }

		curr->next = lblob;    
    }

    lblob->next = NULL;
       
    printk("Loki: Loki blob '%s' created.\n", name);
    printk("Loki: Adding new Loki blob '%s' to master blob.\n", name);

    // Expand master blob to accomodate new data
    if (!(ldir->lfile->master = krealloc(ldir->lfile->master,new_size, GFP_KERNEL)))
    {
        printk("Loki: Unable to expand master blob.\n");
        return -1;
    }
    
    // Set new size and record count
    ldir->lfile->tot_size = new_size;
    ldir->lfile->records++;
    
    // Reconstruct master blob format
    loki_construct_blob(ldir);
    
	return 0;
}

/**
 * Finds a specified Loki blob.
 * @name: the name of the Loki blob to find
 * @return: a pointer to the Loki blob if found, else null
 */
static struct loki_record *loki_find_record(char *name, struct loki_dir *ldir)
{
    struct loki_record *curr;

#ifdef DEBUG    
    printk("Loki: Searching for Loki blob '%s'...\n", name);
#endif

    curr = ldir->lfile->lblob;
    
	while (curr != NULL)
    { 
    	if ((strcmp(name, curr->name)) == 0)
        {
#ifdef DEBUG
            printk("Loki: Loki blob '%s' found.\n", name);
#endif

            return curr;
        }

        curr = curr->next;
    }

    printk("Loki: Loki blob '%s' not found.\n", name);  

    return NULL;
}

/**
 * Properly disposes of Loki resources.
 */
void loki_cleanup(struct loki_dir *ldir)
{
  	struct loki_record *curr, *prev;

    printk("Loki: Cleaning up...\n");
    
	kfree(ldir->name);
    debugfs_remove(ldir->lfile->entry);
    
	// Remove master blob
    kfree(ldir->lfile->master);
    
    // Remove each llist name
	curr = ldir->lfile->lblob;
	
    while (curr)
    {
        kfree(curr->name);
        prev = curr;
        curr = curr->next;
        kfree(prev);
    }
        
	debugfs_remove(ldir->entry);
	kfree(ldir->lfile->name);
	kfree(ldir->lfile);
	kfree(ldir);

    printk("Loki: Cleanup complete.\n");
}
