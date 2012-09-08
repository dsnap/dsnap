#ifndef _LOKI_H_
#define _LOKI_H_

#include <linux/debugfs.h>

/**
 * A Loki directory is found in the root of the debugfs directory and
 * contains the binary file that stores collected driver data.
 */
struct loki_dir {
	char *name;
	struct dentry *entry;
	char *path;
	struct loki_file *lfile;
};

/**
 * A Loki file contains a debugfs blob that is the object that actually
 * stores collected driver data.
 */
struct loki_file {
	char *name;
	struct dentry *entry;
	struct debugfs_blob_wrapper *blob;
	struct loki_record *lrecord;
	u8 *master;
	int records;
	int tot_size;
};

/**
 * A Loki record contains a debugfs blob, along with information about its
 * location in the master blob file.
 */
struct loki_record {
	char *name;
	int offset;		/* index of starting location in master blob */
	int size;		/* index of ending location in master blob */
	void *location;
	struct loki_record *next;
};

extern void loki_init(char *dir_name, struct loki_dir *loki_dir,
			char *file_name, char *debugfs_root);
extern void loki_cleanup(struct loki_dir *ldir);
extern void loki_add_to_blob(char *name, void *location, int size,
				struct loki_dir *ldir);

#endif
