#include "vm/frame.h"
#include <stdio.h>
#include <bitmap.h>
#include <round.h>
#include "vm/page.h"
#include "vm/swap.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"


static struct bitmap *free_frames;
static struct frame_entry *frame_table;
static unsigned clock_ptr, clock_max;

static struct lock frame_lock;

void
frame_init (size_t user_pages)
{
  unsigned i;
  /* Same thing for calculating pages in init_pool from palloc.c */
  size_t bm_pages = DIV_ROUND_UP (bitmap_buf_size (user_pages), PGSIZE);
  if (bm_pages > user_pages)
    bm_pages = user_pages;
  user_pages -= bm_pages;

  clock_ptr = 0;
  clock_max = (unsigned) user_pages;
  /* The frame table contains one entry for each frame that contains a user page */
  frame_table = (struct frame_entry*)malloc(sizeof(struct frame_entry)*user_pages);
  free_frames = bitmap_create(user_pages);
  for(i = 0; i < user_pages; i++) {
    frame_table[i].num = i;
    frame_table[i].page_occupant = NULL;
  }

  lock_init (&frame_lock);
}

struct frame_entry *
frame_get_multiple (size_t page_cnt)
{
  lock_acquire (&frame_lock);
  size_t fnum = bitmap_scan_and_flip (free_frames, 0, page_cnt, false);
  if(fnum != BITMAP_ERROR) {
    frame_table[fnum].kpage = palloc_get_page(PAL_USER | PAL_ASSERT | PAL_ZERO);
    lock_release (&frame_lock);
    return &frame_table[fnum];
  }
  else {
    /* Evict frame */
    struct thread *t = thread_current ();
    bool ev_frame_not_found = true;
    while (ev_frame_not_found)
    {
      if (pagedir_is_accessed (t->pagedir, frame_table[clock_ptr].page_occupant->addr)) {
        pagedir_set_accessed (t->pagedir, frame_table[clock_ptr].page_occupant->addr, false);
        clock_ptr++;
        if(clock_ptr >= clock_max)
          clock_ptr = 0;
      } else {
        if (lock_try_acquire(&frame_table[clock_ptr].page_occupant->DNR)) {
          ev_frame_not_found = false;
        } else {
          clock_ptr++;
          if(clock_ptr >= clock_max)
            clock_ptr = 0;
        }
      }
    }
    /* Swap stuff */
    swap_insert (frame_table[clock_ptr].page_occupant);
    frame_table[clock_ptr].page_occupant->frame = NULL;
    frame_table[clock_ptr].page_occupant->status = IN_SWAP_TABLE;
    pagedir_clear_page (frame_table[clock_ptr].page_occupant->pagedir, frame_table[clock_ptr].page_occupant->addr);
    
    unsigned clock_prev = clock_ptr;
    clock_ptr++;
    if(clock_ptr >= clock_max)
        clock_ptr = 0;
    lock_release (&frame_table[clock_prev].page_occupant->DNR);
    lock_release (&frame_lock);
    return &frame_table[clock_prev];
  }
}

struct frame_entry *
get_frame ()
{
  return frame_get_multiple (1);
}

/* Free given frame occupied by terminating process */
void
free_frame (struct frame_entry *f)
{
  ASSERT (f != NULL);

  lock_acquire (&frame_lock);
  pagedir_clear_page (f->page_occupant->pagedir, f->page_occupant->addr);
  bitmap_reset (free_frames, f->num);
  palloc_free_page (frame_table[f->num].kpage);
  f->page_occupant = NULL;
  lock_release (&frame_lock);
}
