commual_path = $(subst executor,,$(shell sh -c 'pwd'))includes

COMMUAL_OBJS = $(commual_path)/event.o $(commual_path)/lock.o $(commual_path)/map.o\
			   $(commual_path)/memory.o $(commual_path)/network.o $(commual_path)/queue.o\
			   $(commual_path)/util.o $(commual_path)/dstr.o
.PHONY:commual
commual:$(COMMUAL_OBJS)
%.o:%.c
	$(CC) $(DEBUG) $(STD) $ -c $< -o $@ $(CFLAGS) $(INCLUDE)

.PHONY:clean_commual
clean_commual:
	rm -f $(SOURCE_PATH)/*.o