/*
 * static int file_t__load(file_t* obj)
 *
 *	Read file contents in memory. A (key,value) pairs tree is built.
 *	Return 1 on success, 0 on failure.
 */
static int file_t__load(file_t* obj)
{
	FILE* fd;
	data_t* cur_data;
	data_t* tree_data;
#ifdef USE_MMAP
	size_t file_length;
	char* start_of_file;
	char* cur_line;
	char* end_of_file;
#endif
	struct stat fstatus;
	
	tdestroy(obj->data,(__free_fn_t)data_t_meta.dtor);
	obj->data = NULL;

	/* we will make sure that we will not try to map directors*/
	if(stat(obj->path, &fstatus)){
		return status = 1;
	}
	if( S_ISDIR(fstatus.st_mode) )
	{
		//fprintf(stderr,/*###*/"file_t__load(): !!! Trying to map a directory '%s'...\n",obj->path);
		return status = 1;
	}
	fd = fopen(obj->path,/*###*/"r");
	if(fd == NULL) {
		if(errno != ENOENT) {
			//fprintf(stderr,/*###*/"file_t__load(): Could not read file '%s'...\n",obj->path);
			perror(/*###*/"fopen");
			return status = 0;
		}
		else
			return status = 1;
	}

#ifdef USE_MMAP
	fseek(fd,0,SEEK_END);
	file_length = ftell(fd);
	
	if( file_length == 0 )
	{
		fclose(fd);
		fd = fopen(obj->path,/*###*/"w");
		/* lets put a space, so that the file will not have the lenght 0 */
		fprintf(fd, /*###*/" ");
		fflush(fd);
		
		fclose(fd);
		fd = fopen(obj->path,/*###*/"r");
		fseek(fd,0,SEEK_END);
		file_length = ftell(fd);		
	}
	start_of_file = mmap(NULL,file_length,PROT_READ,MAP_SHARED,fileno(fd),0);
	if(start_of_file == MAP_FAILED) {
		//fprintf(stderr,/*###*/"file_t__load(): Could not mmap file '%s'...\n",obj->path);
		perror(/*###*/"mmap");
		fclose(fd);
		return status = 0;
	}
	end_of_file = start_of_file + file_length;
	
	cur_line = start_of_file;
	while(cur_line < end_of_file) {
		if((cur_data = data_t_meta.ctor_line(&cur_line)) == NULL)
			continue;
		tree_data = file_t_meta.create_data(obj,cur_data);
	if(tree_data == NULL ||tree_data->value_attribute==NULL || tree_data->value_attribute != cur_data->value_attribute)
		data_t_meta.dtor(cur_data);
	}
	
	if(munmap(start_of_file,file_length) == -1) {
		//fprintf(stderr,/*###*/"file_t__load(): Could not munmap file '%s'...\n",obj->path);
		perror(/*###*/"munmap");
	}
#else
	while(XP_GetLine(&iobuf,&iobuf_len,fd) != -1) {
		if((cur_data = data_t_meta.ctor_line(iobuf)) == NULL)
			continue;
		tree_data = file_t_meta.create_data(obj,cur_data);
	if(tree_data == NULL ||tree_data->value_attribute==NULL || tree_data->value_attribute != cur_data->value_attribute)
		data_t_meta.dtor(cur_data);
	}
#endif
	obj->flags &= ~DIRTY;


#if __XP_SIO_DEBUG_LEVEL == DEBUG_LEVEL__FILELOAD
	file_t_meta.toString(obj);
#endif

	fclose(fd);
	return status = 1;
}