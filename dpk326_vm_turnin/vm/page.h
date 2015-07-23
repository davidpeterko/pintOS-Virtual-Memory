#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <stdint.h>
#include <hash.h>
#include "threads/palloc.h"
#include "filesys/file.h"
#include "vm/frame.h"
#include "threads/synch.h"

/* struct declarations */
//struct hash pages;

enum page_status
  {
    ALL_ZERO,         /* New page */
    IN_FRAME_TABLE,   /* Where the page resides */
    IN_SWAP_TABLE,
    IN_FILESYS
  };

/* page struct with hash element (hash_elem) */
/* page table will just a hash table containing the hashes */
/* create pages in process.c load_segment, maybe other places */
struct page
  {
    struct hash_elem hash_elem; /* Hash table element. */
    void *addr;                 /* Virtual address. */
    /* ...other members... */
    struct frame_entry *frame;  /* Occupied frame, if any */
    enum page_status status;    /* Where the page should be */
    bool is_stack_page;         /* If this is a stack page */
    bool writable;              /* Can you write to this page */
    struct file *file;          /* File to load page from */
    off_t offset;               /* File offset */
    size_t read_bytes;          /* Number of bytes to read from file */
    int block_sector;      /* Sector in the swap table */
    uint32_t *pagedir;          /* Owner thread's pagedir */
    struct lock DNR;            /* Lock for when it is safe to evict
                                   the frame this is occupying */
  };

unsigned page_hash (const struct hash_elem *, void *);
bool page_less (const struct hash_elem *, const struct hash_elem *, void *);
void page_destructor (struct hash_elem *, void *);
void sup_page_init (void);
struct page *page_lookup (void *);
bool page_do_not_remove (void *);
bool page_allow_remove (void *);

#endif /* vm/page.h */
