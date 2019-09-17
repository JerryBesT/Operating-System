#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#include <string.h>

#define stat xv6_stat  // avoid clash with host struct stat
#define dirent xv6_dirent  // avoid clash with host struct stat
#include "include/types.h"
#include "include/fs.h"
#include "include/stat.h"
#undef stat
#undef dirent

// File System Layout
// unused | superblock | inodes - 25 blocks | unused | bitmap | data
//   1          1               25               1       1       995

void err(char *Msg) {
	fprintf(stderr, "%s", Msg);
	exit(1);
}

// check if any directory is referenced twice or more
void check_dir(void* img_ptr, struct superblock *sb, int ref[]) {
	struct dinode *dip = (struct dinode*) (img_ptr + 2 * BSIZE);
	for(int i = 0;i < sb->ninodes;i++) {
		struct dinode curr = dip[i];
		if(curr.type != 1) continue;

		for(int j = 0;j < NDIRECT && curr.addrs[j] != 0;j++) {
			struct xv6_dirent *dir_content = img_ptr + curr.addrs[j] * BSIZE;
			for(int ndir = 0;ndir < BSIZE/sizeof(struct xv6_dirent);ndir++) {
				struct xv6_dirent r = dir_content[ndir];
				struct dinode *node = img_ptr + 2 * BSIZE;
				if(node[r.inum].type == 2)
					continue;
				if(strcmp(r.name, ".") != 0 && strcmp(r.name, "..") != 0) {
					ref[r.inum]++;
				}
			}	
		}
		
		if(curr.addrs[NDIRECT] == 0) continue;
		
		uint *blk = img_ptr + BSIZE * curr.addrs[NDIRECT];
		for(int k = 0;k < BSIZE/sizeof(uint) && blk[k] != 0;k++) {
			struct xv6_dirent *dir_content = img_ptr + blk[k] * BSIZE;
			for(int ndir = 0;ndir < BSIZE/sizeof(struct xv6_dirent);ndir++) {
				struct xv6_dirent r = dir_content[ndir];
				struct dinode *node = img_ptr + 2 * BSIZE;
				if(node[r.inum].type == 2)
					continue;
				if(strcmp(r.name, ".") != 0 && strcmp(r.name, "..") != 0)
					ref[r.inum]++;
			}	
		}
	}

	for(int i = 1;i < sb->ninodes;i++)
		if(ref[i] > 1)
			err("ERROR: directory appears more than once in file system.\n");
}

int check_parent(void* img_ptr, int current, int parent) {
	struct dinode *dip = img_ptr + 2 * BSIZE;
	struct dinode node = dip[parent];	
	if(node.type != T_DIR) {
		return -1;
	}

	// direct
	for(int i = 0;i < NDIRECT && node.addrs[i] != 0;i++) {
		struct xv6_dirent *dir = img_ptr + node.addrs[i] * BSIZE;
		for(int ndir = 0;ndir < BSIZE/sizeof(struct xv6_dirent);ndir++) {
			struct xv6_dirent r = dir[ndir];
			if(r.inum == current)
				return 1;
		}
	}

	if(node.addrs[NDIRECT] == 0) 
		return -1;

	// indirect
	uint *blk = img_ptr + BSIZE * node.addrs[NDIRECT];
	for(int k = 0;k < BSIZE/sizeof(uint) && blk[k] != 0;k++) {
		struct xv6_dirent *dir = img_ptr + blk[k] * BSIZE;
		for(int ndir = 0;ndir < BSIZE/sizeof(struct xv6_dirent);ndir++) {
			struct xv6_dirent r = dir[ndir];
			if(r.inum == current)
				return 1;
		}	
	}

	return -1;
}

int check_loop(void* img_ptr, int p_inum, int ninodes) {
	struct dinode *dip = img_ptr + 2 * BSIZE;

	int prev[ninodes];
	for(int i = 0;i < ninodes;i++)
		prev[i] = 0;
	int idx = 0;

	while(1) {
		for(int i = 0;i < ninodes;i++)
			if(prev[i] == p_inum)
				return -1;
		prev[idx++] = p_inum;
		struct dinode inode = dip[p_inum];
		if(inode.type != T_DIR)
			return -1;
		struct xv6_dirent *p = img_ptr + BSIZE * inode.addrs[0]; 
		int next_p_inum = p[1].inum;
		p_inum = next_p_inum;
		if(next_p_inum == 1)
			return 1;
	}
}

void repairing(void* img_ptr, struct superblock *sb) {
	int lost_found_inum = 0;
	struct dinode *dip = img_ptr + 2 * BSIZE;
	struct dinode root = dip[1];
	struct xv6_dirent *dir = img_ptr + BSIZE * root.addrs[0];
	for(int i = 0;i < BSIZE/sizeof(struct xv6_dirent);i++) {
		if(strcmp(dir[i].name, "lost_found") == 0)
			lost_found_inum = dir[i].inum;
	}

	for(int i = 0;i < sb->ninodes;i++) { 
		struct dinode curr = dip[i];
		if(curr.type != 0 && curr.nlink == 0) {
			printf("unfinished, fucking jerry, do this easy shit sometimes\n");
		}
	}
}

int main(int argc, char *argv[]) {
	int fd;
	int repair = 0;
	if(argc == 2) {
		fd = open(argv[1], O_RDONLY);
		if(fd < 0) {
			err("image not found.\n");
		}
	}
	else if(argc == 3) {
		char *permission = argv[1];
		if(strcmp(permission, "-r") != 0)
			err("Usage: xcheck <file_system_image> \n");
		else {
			fd = open(argv[2], RDRW);
			if(fd < 0) {
				err("image not found.\n");
			}
			repair = 1;
		}
	}
	else {
		err("Usage: xcheck <file_system_image> \n");
	}

	struct stat sbuf;
	fstat(fd, &sbuf);

	// mmap
	void *img_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	struct superblock *sb = (struct superblock*) (img_ptr + BSIZE);
	struct dinode *dip = (struct dinode*) (img_ptr + 2 * BSIZE);
	int num_blk = (sb->ninodes *sizeof(struct dinode))/BSIZE;

	// check if repairing
	if(repair)
		repairing(img_ptr, sb);

	int ref[sb->ninodes];
	for(int i = 0;i < sb->ninodes;i++)
		ref[i] = 0;

	// check directory is not referenced twice or more
	check_dir(img_ptr, sb, ref);

	int links[sb->ninodes];
	for(int i = 0;i < sb->ninodes;i++)
		links[i] = 0;

	int dir_dup[sb->size];
	int indir_dup[sb->size];
	int data_inuse[sb->size];
	for(int i = 0;i < sb->size;i++) {
		data_inuse[i] = 0;
		dir_dup[i] = 0;
		indir_dup[i] = 0;
	}

	int bmap[sb->size];
	uchar *bitmap = img_ptr + (num_blk + 3) * BSIZE;
	int count = 0;
	for(int i = 0;i < ((sb->size/BSIZE) * BSIZE)/8;i++) {
		for(int j = 0;j < 8;j++) {
			bmap[count] = (bitmap[i] >> j) & 1;
			count++;
		}
	}

	// check root is valid
	struct dinode root = dip[1];
  if(root.type != T_DIR)
    err("ERROR: root directory does not exist.\n");
  struct xv6_dirent *content = img_ptr + root.addrs[0] * BSIZE;
	if((content[0].inum != 1 || content[1].inum != 1) && 
			(strcmp(content[0].name, ".") == 0 && strcmp(content[1].name, "..") == 0))
   err("ERROR: root directory does not exist.\n");

	// starting all inodes
	for(int i = 0;i < sb->ninodes;i++) {
		struct dinode curr = dip[i];
		int type = dip[i].type;

		// check the inode is one of the three normal types
		if(type != 0 && type != T_DIR && type != T_FILE && type != T_DEV)
			err("ERROR: bad inode.\n");

		// check directory
		if(curr.type == T_DIR) {
			struct xv6_dirent *dir_content = img_ptr + curr.addrs[0] * BSIZE;
			if(strcmp(dir_content[0].name, ".") != 0 || strcmp(dir_content[1].name, "..") != 0)
				err("ERROR: directory not properly formatted.\n");
			if(dir_content[0].inum != i)
				err("ERROR: directory not properly formatted.\n");
			int tempIdx = -1;
			while(*(dir_content[++tempIdx].name) != 0) {
				links[dir_content[tempIdx].inum]++;
			}

			// check directory parent and child relation
			if(check_parent(img_ptr, dir_content[0].inum, dir_content[1].inum) == -1)
				err("ERROR: parent directory mismatch.\n");

			// check directory possible dead loop
			if(check_loop(img_ptr, dir_content[1].inum, sb->ninodes) == -1)
				err("ERROR: inaccessible directory exists.\n");
		}

		// check if the link is correct
		if(curr.type == 0 && curr.nlink >= 1)
			err("ERROR: inode referred to in directory but marked free.\n");

		// direct
		for(int j = 0;j < NDIRECT;j++) {
			uint addr = curr.addrs[j];
			if(addr != 0 && (addr < (num_blk + 4) || addr > sb->nblocks))
				err("ERROR: bad direct address in inode.\n");
			if(addr != 0 && curr.type != 0)
				dir_dup[addr]++;
		}
		// indirect
		uint addr = curr.addrs[NDIRECT];
		if(addr != 0 && (addr < (num_blk + 4) || addr > sb->nblocks))
			err("ERROR: bad indirect address in inode.\n");
		if(addr != 0 && curr.type != 0)
			indir_dup[addr]++;

		// check bitmap
		if(curr.type != 0) {
			if(curr.nlink == 0)
				err("ERROR: inode marked use but not found in a directory.\n");
	
			// direct data
			for(int j = 0;j < NDIRECT && curr.addrs[j] != 0;j++) {
				if(bmap[curr.addrs[j]] == 0)
					err("ERROR: address used by inode but marked free in bitmap.\n");
				data_inuse[curr.addrs[j]] = 1;
			}

			// indirect data
			if(curr.addrs[NDIRECT] != 0) {
				uint *indir = img_ptr + curr.addrs[NDIRECT] * BSIZE;
				for(int k = 0;k < BSIZE/sizeof(uint) && indir[k] != 0;k++) {
					if(bmap[indir[k]] == 0)
						err("ERROR: address used by inode but marked free in bitmap.\n");
					data_inuse[indir[k]] = 1;
				}
				data_inuse[curr.addrs[NDIRECT]] = 1;
			}
		}

	}

	// check bitmap
	for(int i = num_blk + 4;i < sb->size;i++) {
		if(bmap[i] != 0 && data_inuse[i] == 0)
			err("ERROR: bitmap marks block in use but it is not in use.\n");
	}

	// check reference counts of files
	for(int i = 0;i < sb->ninodes;i++) {
		struct dinode curr = dip[i];
		if(links[i] == 0)
			continue;
		if(curr.nlink != links[i] && curr.type == 2)
			err("ERROR: bad reference count for file.\n");
	}

	// check data No. duplicate
	for(int i = 0 ;i < sb->size;i++) {
		if(dir_dup[i] > 1)
			err("ERROR: direct address used more than once.\n");
		if(indir_dup[i] > 1)
			err("ERROR: indirect address used more than once.\n");
	}

	exit(0);
}

