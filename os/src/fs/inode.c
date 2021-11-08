#include <stdbool.h>

#include "drivers.h"
#include "efs.h"
#include "external.h"
#include "fcntl.h"
#include "fs.h"
#include "log.h"

uint64_t inode_read_all(OSInode *osinode, uint8_t *buf) {
  uint64_t len = 0;
  while (1) {
    len = inode_read_at(&osinode->inode, osinode->offset, buf, BLOCK_SZ);
    if (len == 0) {
      break;
    }
    osinode->offset += len;
    buf += len;
  }
  return len;
}

static Inode ROOT_INODE;

void inode_root_init() {
  static EasyFileSystem efs;
  BlockDevice *device = virtio_block_device_init();
  block_cache_manager_init();
  efs_open(&efs, device);
  efs_root_inode(&ROOT_INODE, &efs);
}

void inode_list_apps() {
  info("/**** APPS ****\n");
  uint64_t ls_len = inode_ls_len(&ROOT_INODE);
  char **apps = bd_malloc(ls_len * sizeof(char *));
  for (uint64_t i = 0; i < ls_len; i++) {
    apps[i] = bd_malloc(NAME_LENGTH_LIMIT + 1);
  }
  inode_ls(&ROOT_INODE, apps);
  for (uint64_t i = 0; i < ls_len; i++) {
    info("%s\n", apps[i]);
  }
  for (uint64_t i = 0; i < ls_len; i++) {
    bd_free(apps[i]);
  }
  bd_free(apps);
  info("**************/\n");
}

OSInode *inode_open_file(char *name, uint32_t flags) {
  OSInode *osinode = bd_malloc(sizeof(OSInode));
  osinode->ref = 1;
  osinode->offset = 0;
  if ((flags & O_CREATE) != 0) {
    if (inode_find(&ROOT_INODE, name, &osinode->inode) == 0) {
      // clear size
      inode_clear(&osinode->inode);
      return osinode;
    } else {
      // create file
      if (inode_create(&ROOT_INODE, name, &osinode->inode) == 0) {
        return osinode;
      }
    }
  } else {
    if (inode_find(&ROOT_INODE, name, &osinode->inode) == 0) {
      if ((flags & O_TRUNC) != 0) {
        inode_clear(&osinode->inode);
      }
      return osinode;
    }
  }
  bd_free(osinode);
  return NULL;
}

int64_t inode_close_file(OSInode *osinode) {
  if (!osinode) {
    return -1;
  }
  if (--osinode->ref > 0) {
    return 0;
  }
  bd_free(osinode);
  return 0;
}

int64_t inode_read(OSInode *osinode, char *buf, uint64_t len) {
  uint64_t read_size = 0;
  read_size =
      inode_read_at(&osinode->inode, osinode->offset, (uint8_t *)buf, len);
  osinode->offset += read_size;
  return read_size;
}

int64_t inode_write(OSInode *osinode, char *buf, uint64_t len) {
  uint64_t write_size = 0;
  write_size =
      inode_write_at(&osinode->inode, osinode->offset, (uint8_t *)buf, len);
  assert(write_size == len);
  osinode->offset += write_size;
  return write_size;
}
