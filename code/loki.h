
/*  Copyright(C) 2012 Computer Science Capstone (Spring/Summer) Team
             2.718 Portland State University

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


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
