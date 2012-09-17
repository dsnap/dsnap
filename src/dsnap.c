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
static struct dentry *dsnap_create_blob(char *name, struct dsnap_dir *ddir);
static struct dsnap_record *dsnap_find_record(char *name, struct dsnap_dir *ddir);
static int dsnap_create_record(char *name, void *location, int size,
				struct dsnap_dir *ddir);
static void dsnap_construct_blob(struct dsnap_dir *ddir);

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
 * @ddir: the dsnap directory to operate in
 * @file_name: the name of the blob file
 * @debugfs_root: the path to the debugfs root directory
 */
void dsnap_init(char *dir_name, struct dsnap_dir *ddir, char *file_name)
{
	struct dentry *debugfs_dentry;

	printk(KERN_INFO "dsnap: Initializing...\n");

	ddir->name = kstrdup(dir_name, GFP_KERNEL);
	debugfs_dentry = debugfs_create_dir(dir_name, NULL);

	if (debugfs_dentry != NULL)
		debugfs_root = debugfs_dentry;

	else if (debugfs_dentry == ERR_PTR(-ENODEV))
		printk(KERN_ERR "dsnap: Your kernel must have debugfs support "
				"enabled to run dsnap.\n");

	ddir->dfile = dsnap_create_file(file_name);

	if (!ddir->dfile) {
		printk(KERN_ERR "dsnap: Unable to create dsnap file '%s'.\n",
				file_name);
		return;
	}

	ddir->dfile->entry = dsnap_create_blob(ddir->dfile->name, ddir);

	if (!ddir->dfile->entry) {
		printk(KERN_ERR "dsnap: Unable to create blob '%s' "
				"(dentry is NULL).\n",
				ddir->dfile->name);
		return;
	}

	/* Set master size */
	ddir->dfile->tot_size = (sizeof(u32) * 2) +
				strlen(ddir->name) +
				magic_number_length +
				sizeof(format_version);

	/* Allocate memory for master */
	ddir->dfile->master = kmalloc(ddir->dfile->tot_size, GFP_KERNEL);

	if (!ddir->dfile->master) {
		printk(KERN_ERR "dsnap: Unable to allocate memory for "
				"dsnap master buffer.\n");
		return;
	}

	/* Construct initial binary structure */
	dsnap_construct_blob(ddir);

	printk(KERN_INFO "dsnap: Initialization complete.\n");
}

EXPORT_SYMBOL(dsnap_init);

/**
 * Builds a debugfs binary structure from dfiles.
 * @ddir: the dsnap directory to operate in
 */
static void dsnap_construct_blob(struct dsnap_dir *ddir)
{
	struct dsnap_record *curr;
	struct dsnap_file *root = ddir->dfile;
	u8 *c_ptr = root->master;
	int i = 0;

	/* Zero buffer */
	for (i = 0; i < ddir->dfile->tot_size; i++)
		root->master[i] = 0;

	memcpy(c_ptr, &magic_number, magic_number_length);
	c_ptr += magic_number_length;		/* Jump to version number */

	memcpy(c_ptr, &format_version, sizeof(format_version));
	c_ptr += sizeof(format_version);	/* Jump to module name */

	*((u32 *)c_ptr) = strlen(ddir->name);
	c_ptr += sizeof(u32);
	memcpy(c_ptr, ddir->name, strlen(ddir->name));
	c_ptr += strlen(ddir->name);		/* Jump to count of record */

	*((u32 *)c_ptr) = root->records;
	c_ptr += sizeof(u32);			/* Jump to first record */

	curr = ddir->dfile->drecord;

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
	struct dsnap_file *dfile;

	dfile = kmalloc(sizeof(struct dsnap_file), GFP_KERNEL);

	if (!dfile) {
		printk(KERN_ERR "dsnap: Unable to allocate memory for dsnap "
				"file '%s'.\n",
				name);
		return NULL;
	}

	dfile->name = kstrdup(name, GFP_KERNEL);
	dfile->entry = NULL;
	dfile->drecord = NULL;

	/* Location of master buffer */
	dfile->records = 0;
	dfile->tot_size = 0;

	return dfile;
}

/**
 * Creates a debugfs blob that holds driver data.
 * @name: the name of the blob
 * @ddir: the dsnap directory to operate in
 * @return: a pointer to the dentry object returned by the
 *          debugfs_create_blob call
 */
static struct dentry *dsnap_create_blob(char *name, struct dsnap_dir *ddir)
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

	ddir->dfile->blob = blob;

	return entry;
}

/**
 * Adds data to the blob.
 * @name: the name of the data to add
 * @location: the memory location of the data to add
 * @size: the size of the data to add
 * @ddir: the dsnap directory to operate in
 */
void dsnap_add_to_blob(char *name, void *location, int size,
			struct dsnap_dir *ddir)
{
	struct dsnap_record *drecord;

	drecord = dsnap_find_record(name, ddir);
	
	if (!drecord) {
		/* Data has not been added to blob yet, so add it */
		if ((dsnap_create_record(name, location, size, ddir)) == -1) {
			printk(KERN_ERR "dsnap: Unable to create dsnap "
					"blob '%s'.\n", name);
			return;
		}
	} else {
		/* Data already exists in blob */
		if (size != drecord->size) {
			printk(KERN_ERR "dsnap: Passed in size not equal "
					"to size in list\n.");
			return;
		}

		/* Change location to value passed in */
		drecord->location = location;
		
		/* Copy data to offset in blob */
		memcpy(ddir->dfile->master + drecord->offset, location, size);
	}
}

EXPORT_SYMBOL(dsnap_add_to_blob);

/**
 * Creates a dsnap record.
 * @name: the name of the dsnap record
 * @location: the location of the data to store
 * @size: the size of the data to store
 * @ddir: the dsnap directory to operate in
 * @return: -1 if an error occurs
 *	     0 otherwise
 */
int dsnap_create_record(char *name, void *location, int size,
			struct dsnap_dir *ddir)
{
	struct dsnap_record *drecord, *curr;

	int new_size = size +
			ddir->dfile->tot_size +
			strlen(name) +
			(sizeof(u32) * 2);	/* Size and count */

	drecord = kmalloc(sizeof(struct dsnap_record), GFP_KERNEL);

	if (!drecord) {
		printk(KERN_ERR "dsnap: Unable to allocate memory for "
				"dsnap blob '%s'.\n",
				name);
		return -1;
	}

	drecord->name = kstrdup(name, GFP_KERNEL);
	drecord->offset = new_size - size;	/* Record size - data size */
	drecord->size = size;			/* Size of data (in bytes) */
	drecord->location = location;		/* Address in kernel memory */

	/* Add new dsnap blob to the list */
	if (!ddir->dfile->drecord) {
		ddir->dfile->drecord = drecord;
	} else {
		curr = ddir->dfile->drecord;

		while (curr && curr->next != NULL)
			curr = curr->next;

		curr->next = drecord;
	}

	drecord->next = NULL;

	/* Expand master blob to accomodate new data */
	ddir->dfile->master = krealloc(ddir->dfile->master,
					new_size,
					GFP_KERNEL);

	if (!ddir->dfile->master) {
		printk(KERN_ERR "dsnap: Unable to expand master blob.\n");
		return -1;
	}

	/* Set new size and record count */
	ddir->dfile->tot_size = new_size;
	ddir->dfile->records++;

	/* Reconstruct master blob format */
	dsnap_construct_blob(ddir);

	return 0;
}

/**
 * Finds a specified dsnap record.
 * @name: the name of the dsnap record to find
 * @ddir: the dsnap directory to operate in
 * @return: a pointer to the dsnap record if found, else null
 */
static struct dsnap_record *dsnap_find_record(char *name, struct dsnap_dir *ddir)
{
	struct dsnap_record *curr;

	curr = ddir->dfile->drecord;

	while (curr != NULL) {
		if ((strcmp(name, curr->name)) == 0)
			return curr;

		curr = curr->next;
	}

	return NULL;
}

/**
 * Properly disposes of dsnap resources.
 * @ddir: the dsnap directory to operate in
 */
void dsnap_cleanup(struct dsnap_dir *ddir)
{
	struct dsnap_record *curr, *prev;

	printk(KERN_INFO "dsnap: Cleaning up...\n");

	kfree(ddir->name);
	debugfs_remove(ddir->dfile->entry);

	/* Remove master blob */
	kfree(ddir->dfile->master);

	/* Remove each llist name */
	curr = ddir->dfile->drecord;

	while (curr) {
		kfree(curr->name);
		prev = curr;
		curr = curr->next;
		kfree(prev);
	}

	debugfs_remove(debugfs_root);
	kfree(ddir->dfile->name);
	kfree(ddir->dfile);
	kfree(ddir);

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
