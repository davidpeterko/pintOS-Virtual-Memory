BUILD_SUBDIRS = threads userprog vm filesys

all::
	@echo "Run 'make' in subdirectories: $(BUILD_SUBDIRS)."
	@echo "This top-level make has only 'clean' targets."

CLEAN_SUBDIRS = $(BUILD_SUBDIRS) examples utils

clean::
	for d in $(CLEAN_SUBDIRS); do $(MAKE) -C $$d $@; done
	rm -f TAGS tags

distclean:: clean
	find . -name '*~' -exec rm '{}' \;

TAGS_SUBDIRS = $(BUILD_SUBDIRS) devices lib
TAGS_SOURCES = find $(TAGS_SUBDIRS) -name \*.[chS] -print

TAGS::
	etags --members `$(TAGS_SOURCES)`

tags::
	ctags -T --no-warn `$(TAGS_SOURCES)`

cscope.files::
	$(TAGS_SOURCES) > cscope.files

cscope:: cscope.files
	cscope -b -q -k

##################
# Handin your work
##################
turnin.tar: clean
	tar cvf turnin.tar `find . -type f | grep -v \.tar$$ | grep -v \.tar\.gz$$ | grep -v /\.git | grep -v \.swp$$ | grep -v CVS | grep -v svn | grep -v \.pl$$ | grep -v ~$$` 

LAB1_NAME := threads
LAB2_NAME := user
LAB3_NAME := vm
LAB4_NAME := fs

turnin_threads: turnin.tar
	mv turnin.tar $(USER)_threads_turnin.tar
	@echo "Created $(USER)_threads_turnin.tar."

turnin_user: turnin.tar
	mv turnin.tar $(USER)_userprog_turnin.tar
	@echo "Created $(USER)_userprog_turnin.tar."

turnin_vm: turnin.tar
	mv turnin.tar $(USER)_vm_turnin.tar
	@echo "Created $(USER)_vm_turnin.tar."

turnin_fs: turnin.tar
	mv turnin.tar $(USER)_fs_turnin.tar
	@echo "Created $(USER)_fs_turnin.tar."
