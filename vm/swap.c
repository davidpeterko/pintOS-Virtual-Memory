#include "vm/swap.h"
#include <stdio.h>
#include <stdint.h>
#include <bitmap.h>
#include "devices/block.h"
#include "vm/frame.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"

static struct bitmap *used_blocks;
struct block *swap_table;

static struct lock block_lock;

void
swap_init (void)
{
  used_blocks = bitmap_create(1024);
  swap_table = block_get_role(BLOCK_SWAP);
  lock_init (&block_lock);
}

/* Block sector size is 512 bytes => 8 sectors = 1 page */

/* Insert the contents of a page into the swap table */
void
swap_insert (struct page *p)
{
  lock_acquire (&block_lock);
  char *c = (char *) p->frame->kpage;
  size_t sector_num = bitmap_scan_and_flip (used_blocks, 0, 1, false);
  p->block_sector = sector_num;
  int i;
  for (i=0; i<8; i++) {
    block_write (swap_table, sector_num*8+i, c);
    c += 512;
  }
  lock_release (&block_lock);
}

/* Read from swap into the page */
void
swap_get (struct page *p)
{
  lock_acquire (&block_lock);
  char *c = (char *) p->frame->kpage;
  unsigned read_sector = p->block_sector;
  int i;
  for (i=0; i<8; i++) {
    block_read (swap_table, read_sector*8+i, c);
    c += 512;
  }
  bitmap_reset (used_blocks, read_sector);
  lock_release (&block_lock);
}

void
swap_free (struct page *p)
{
  lock_acquire (&block_lock);
  bitmap_reset (used_blocks, p->block_sector);
  lock_release (&block_lock);
}
