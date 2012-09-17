/**
 * dsnap - dsnap.c
 *
 * This file provides the implementation of dsnap's core driver-side
 * functionality.
 */

#include "dsnap.h"
#include <linux/debugfs.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

/* GLOBALS */

static const u64 magic_number = 0x70616E7364;	/* "dsnap" in ASCII */
static const int magic_number_length = 5;
static const u8 format_version = 1;
static struct dentry *debugfs_root;

/* PROTOTYPES */

static struct dsnap_file *dsnap_create_file(char *name);
static struct dentry *dsnap_create_blob(char *name, struct dsnap_dir *ldir);
static struct dsnap_record *dsnap_find_record(char *name, struct dsnap_dir *ldir);
static int dsnap_create_record(char *name, void *location, int size,
				struct dsnap_dir *ldir);
static void dsnap_construct_blob(struct dsnap_dir *ldir);

/* FUNCTIONS */

/**
 * Loads the kernel module.
 * @return: 0 on success, non-zero on failure.
 */
int init_module(void)
{
	printk(KERN_INFO "dsnap: Module loaded.\n");
	return 0;
}

/**
 * Initializes the dsnap framework.
 * @dir_name: the name of the debugfs root directory
 * @ldir: the dsnap directory to operate in
 * @file_name: the name of the blob file
 * @debugfs_root: the path to the debugfs root directory
 */
void dsnap_init(char *dir_name, struct dsnap_dir *ldir, char *file_name)
{
	struct file *directory;
	struct dentry *debugfs_dentry;

	printk(KERN_INFO "dsnap: Initializing...\n");

	ldir->name = kstrdup(dir_name, GFP_KERNEL);
	debugfs_dentry = debugfs_create_dir(dir_name, NULL);

	if (debugfs_dentry != NULL)
		debugfs_root = debugfs_dentry;

	else if (debugfs_dentry == ERR_PTR(-ENODEV))
		printk(KERN_ERR "dsnap: Your kernel must have debugfs support "
				"enabled to run dsnap.\n");

	ldir->lfile = dsnap_create_file(file_name);

	if (!ldir->lfile) {
		printk(KERN_ERR "dsnap: Unable to create dsnap file '%s'.\n",
				file_name);
		return;
	}

	ldir->lfile->entry = dsnap_create_blob(ldir->lfile->name, ldir);

	if (!ldir->lfile->entry) {
		printk(KERN_ERR "dsnap: Unable to create blob '%s' "
				"(dentry is NULL).\n",
				ldir->lfile->name);
		return;
	}

	/* Set master size */
	ldir->lfile->tot_size = (sizeof(u32) * 2) +
				strlen(ldir->name) +
				magic_number_length +
				sizeof(format_version);

	/* Allocate memory for master */
	ldir->lfile->master = kmalloc(ldir->lfile->tot_size, GFP_KERNEL);

	if (!ldir->lfile->master) {
		printk(KERN_ERR "dsnap: Unable to allocate memory for "
				"dsnap master buffer.\n");
		return;
	}

	/* Construct initial binary structure */
	dsnap_construct_blob(ldir);

	printk(KERN_INFO "dsnap: Initialization complete.\n");
}

EXPORT_SYMBOL(dsnap_init);

/**
 * Builds a debugfs binary structure from lfiles.
 * @ldir: the dsnap directory to operate in
 */
static void dsnap_construct_blob(struct dsnap_dir *ldir)
{
	struct dsnap_record *curr;
	struct dsnap_file *root = ldir->lfile;
	u8 *c_ptr = root->master;
	int i = 0;

	/* Zero buffer */
	for (i = 0; i < ldir->lfile->tot_size; i++)
		root->master[i] = 0;

	memcpy(c_ptr, &magic_number, magic_number_length);
	c_ptr += magic_number_length;		/* Jump to version number */

	memcpy(c_ptr, &format_version, sizeof(format_version));
	c_ptr += sizeof(format_version);	/* Jump to module name */

	*((u32 *)c_ptr) = strlen(ldir->name);
	c_ptr += sizeof(u32);
	memcpy(c_ptr, ldir->name, strlen(ldir->name));
	c_ptr += strlen(ldir->name);		/* Jump to count of record */

	*((u32 *)c_ptr) = root->records;
	c_ptr += sizeof(u32);			/* Jump to first record */

	curr = ldir->lfile->lrecord;

	while (curr != NULL) {
		if (strlen(curr->name) > 0xffffffff)
			return;

		*((u32 *)c_ptr) = strlen(curr->name);
		c_ptr += sizeof(u32);

		memcpy(c_ptr, curr->name, strlen(curr->name));
		c_ptr += strlen(curr->name);	/* Jump to count of records */

		*((u32 *)c_ptr) = curr->size;
		c_ptr += sizeof(u32);

		memcpy(c_ptr, curr->location, curr->size);
		c_ptr += curr->size;

		curr = curr->next;
	}

	/*
	Debugfs needs to know what the location and new
	size of the blob are.
	*/
	root->blob->data = root->master;
	root->blob->size = root->tot_size;
}

/**
 * Creates a dsnap file.
 * @name: the name of the file to create
 * @return: a pointer to the created file
 */
static struct dsnap_file *dsnap_create_file(char *name)
{
	struct dsnap_file *lfile;

	lfile = kmalloc(sizeof(struct dsnap_file), GFP_KERNEL);

	if (!lfile) {
		printk(KERN_ERR "dsnap: Unable to allocate memory for dsnap "
				"file '%s'.\n",
				name);
		return NULL;
	}

	lfile->name = kstrdup(name, GFP_KERNEL);
	lfile->entry = NULL;
	lfile->lrecord = NULL;

	/* Location of master buffer */
	lfile->records = 0;
	lfile->tot_size = 0;

	return lfile;
}

/**
 * Creates a debugfs blob that holds driver data.
 * @name: the name of the blob
 * @ldir: the dsnap directory to operate in
 * @return: a pointer to the dentry object returned by the
 *          debugfs_create_blob call
 */
static struct dentry *dsnap_create_blob(char *name, struct dsnap_dir *ldir)
{
	struct debugfs_blob_wrapper *blob;
	struct dentry *entry;

	blob = kmalloc(sizeof(struct debugfs_blob_wrapper), GFP_KERNEL);

	if (!blob) {
		printk(KERN_ERR "dsnap: Unable to allocate memory for "
				"blob '%s'.\n",
				name);
		return NULL;
	}

	entry = debugfs_create_blob(name, S_IRUGO, debugfs_root, blob);

	if (!entry) {
		printk(KERN_ERR "dsnap: Unable to create blob '%s' "
				"(dentry is NULL).\n",
				name);
		return NULL;
	}

	blob->data = NULL;
	blob->size = 0;

	ldir->lfile->blob = blob;

	return entry;
}

/**
 * Adds data to the blob.
 * @name: the name of the data to add
 * @location: the memory location of the data to add
 * @size: the size of the data to add
 * @ldir: the dsnap directory to operate in
 */
void dsnap_add_to_blob(char *name, void *location, int size,
			struct dsnap_dir *ldir)
{
	struct dsnap_record *lrecord;

	lrecord = dsnap_find_record(name, ldir);
	
	if (!lrecord) {
		/* Data has not been added to blob yet, so add it */
		if ((dsnap_create_record(name, location, size, ldir)) == -1) {
			printk(KERN_ERR "dsnap: Unable to create dsnap "
					"blob '%s'.\n", name);
			return;
		}
	} else {
		/* Data already exists in blob */
		if (size != lrecord->size) {
			printk(KERN_ERR "dsnap: Passed in size not equal "
					"to size in list\n.");
			return;
		}

		/* Change location to value passed in */
		lrecord->location = location;
		
		/* Copy data to offset in blob */
		memcpy(ldir->lfile->master + lrecord->offset, location, size);
	}
}

EXPORT_SYMBOL(dsnap_add_to_blob);

/**
 * Creates a dsnap record.
 * @name: the name of the dsnap record
 * @location: the location of the data to store
 * @size: the size of the data to store
 * @ldir: the dsnap directory to operate in
 * @return: -1 if an error occurs
 *	     0 otherwise
 */
int dsnap_create_record(char *name, void *location, int size,
			struct dsnap_dir *ldir)
{
	struct dsnap_record *lrecord, *curr;

	int new_size = size +
			ldir->lfile->tot_size +
			strlen(name) +
			(sizeof(u32) * 2);	/* Size and count */

	lrecord = kmalloc(sizeof(struct dsnap_record), GFP_KERNEL);

	if (!lrecord) {
		printk(KERN_ERR "dsnap: Unable to allocate memory for "
				"dsnap blob '%s'.\n",
				name);
		return -1;
	}

	lrecord->name = kstrdup(name, GFP_KERNEL);
	lrecord->offset = new_size - size;	/* Record size - data size */
	lrecord->size = size;			/* Size of data (in bytes) */
	lrecord->location = location;		/* Address in kernel memory */

	/* Add new dsnap blob to the list */
	if (!ldir->lfile->lrecord) {
		ldir->lfile->lrecord = lrecord;
	} else {
		curr = ldir->lfile->lrecord;

		while (curr && curr->next != NULL)
			curr = curr->next;

		curr->next = lrecord;
	}

	lrecord->next = NULL;

	/* Expand master blob to accomodate new data */
	ldir->lfile->master = krealloc(ldir->lfile->master,
					new_size,
					GFP_KERNEL);

	if (!ldir->lfile->master) {
		printk(KERN_ERR "dsnap: Unable to expand master blob.\n");
		return -1;
	}

	/* Set new size and record count */
	ldir->lfile->tot_size = new_size;
	ldir->lfile->records++;

	/* Reconstruct master blob format */
	dsnap_construct_blob(ldir);

	return 0;
}

/**
 * Finds a specified dsnap record.
 * @name: the name of the dsnap record to find
 * @ldir: the dsnap directory to operate in
 * @return: a pointer to the dsnap record if found, else null
 */
static struct dsnap_record *dsnap_find_record(char *name, struct dsnap_dir *ldir)
{
	struct dsnap_record *curr;

	curr = ldir->lfile->lrecord;

	while (curr != NULL) {
		if ((strcmp(name, curr->name)) == 0)
			return curr;

		curr = curr->next;
	}

	return NULL;
}

/**
 * Properly disposes of dsnap resources.
 * @ldir: the dsnap directory to operate in
 */
void dsnap_cleanup(struct dsnap_dir *ldir)
{
	struct dsnap_record *curr, *prev;

	printk(KERN_INFO "dsnap: Cleaning up...\n");

	kfree(ldir->name);
	debugfs_remove(ldir->lfile->entry);

	/* Remove master blob */
	kfree(ldir->lfile->master);

	/* Remove each llist name */
	curr = ldir->lfile->lrecord;

	while (curr) {
		kfree(curr->name);
		prev = curr;
		curr = curr->next;
		kfree(prev);
	}

	debugfs_remove(debugfs_root);
	kfree(ldir->lfile->name);
	kfree(ldir->lfile);
	kfree(ldir);

	printk(KERN_INFO "dsnap: Cleanup complete.\n");
}

EXPORT_SYMBOL(dsnap_cleanup);

/**
 * Unloads the kernel module.
 */
void cleanup_module(void)
{
	printk(KERN_INFO "dsnap: Module unloaded.\n");
}

MODULE_AUTHOR("David Huddleson <huddlesd@cs.pdx.edu>, "
		"Kyle Pelton <peltonkyle@gmail.com>, "
		"Devin Quirozoliver <arik182@cs.pdx.edu>, "
		"Ekaterina Ryabtseva <ekaterir@cs.pdx.edu>, "
		"John Sackey <sackey@gmail.com>, "
		"Jacob Sowles <sowlesj@gmail.com>");

MODULE_DESCRIPTION("A driver state snapshot utility.");

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
