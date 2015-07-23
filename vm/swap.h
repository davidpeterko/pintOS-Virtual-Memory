#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "vm/page.h"

void swap_init (void);

void swap_insert (struct page *);
void swap_get (struct page *);
void swap_free (struct page *);

#endif /* vm/swap.h */
