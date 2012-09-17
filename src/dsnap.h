/**
 * dsnap - dsnap.h
 *
 * This file provides the data structures and function headers for those
 * functions that are accessible to device drivers.
 */

#ifndef _LOKI_H_
#define _LOKI_H_

#include <linux/debugfs.h>

/**
 * A dsnap directory is found in the root of the debugfs directory and
 * contains the binary file that stores collected driver data.
 */
struct dsnap_dir {
	char *name;
	struct dsnap_file *dfile;
};

/**
 * A dsnap file contains a debugfs blob that is the object that actually
 * stores collected driver data.
 */
struct dsnap_file {
	char *name;
	struct dentry *entry;
	struct debugfs_blob_wrapper *blob;
	struct dsnap_record *drecord;
	u8 *master;
	int records;
	int tot_size;
};

/**
 * A dsnap record contains a debugfs blob, along with information about its
 * location in the master blob file.
 */
struct dsnap_record {
	char *name;
	int offset;		/* Index of starting location in master blob. */
	int size;		/* Index of ending location in master blob. */
	void *location;
	struct dsnap_record *next;
};

extern void dsnap_init(char *dir_name, struct dsnap_dir *ddir, char *file_name);
extern void dsnap_cleanup(struct dsnap_dir *ddir);
extern void dsnap_add_to_blob(char *name, void *location, int size,
				struct dsnap_dir *ddir);

#endif
