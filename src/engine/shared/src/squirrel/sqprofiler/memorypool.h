#ifndef _SQUIRREL_MEMORYPOOL_H_
#define _SQUIRREL_MEMORYPOOL_H_

#ifndef SQUNICODE
#define scstrcpy strcpy
#else
#define scstrcpy wcscpy
#endif

template<int _chunksize_> struct memorypool {
	struct pool_chunk {
		pool_chunk(SQUnsignedInteger csize):size(csize) { used = 0; next = NULL; }
		SQUnsignedInteger used;
		const SQUnsignedInteger size;
		pool_chunk *next;
		unsigned char data[1];
	private:
		// No assignment
		pool_chunk& operator=(const pool_chunk &tmp);
	};
	~memorypool() {
		pool_chunk *chunk = chunks;
		while(chunk) {
			pool_chunk *x = chunk;
			chunk = x->next;
			free(x);
		}
		chunk = freechunks;
		while(chunk) {
			pool_chunk *x = chunk;
			chunk = x->next;
			free(x);
		}
	}
	memorypool()
	{
		freechunks = NULL;
		chunks = NULL;
	}
	pool_chunk *create_chunk(SQUnsignedInteger size)
	{
		void *data = malloc(sizeof(pool_chunk) + (size - 1) );
		return new (data) pool_chunk(size);
	}
	void *alloc(SQUnsignedInteger size) {

		if(!chunks || ((chunks->used + size) > chunks->size)) {
			pool_chunk *newc;
			if(freechunks && (freechunks->size >= size)) {
				newc = freechunks;
				newc->used = 0;
				freechunks = newc->next; 
			}
			else {
				SQUnsignedInteger newcsize = (size > _chunksize_) ? size : _chunksize_;
				newc = create_chunk(newcsize);
			}
			newc->next = chunks;
			chunks = newc;
		}
		void *ret = &chunks->data[chunks->used];
		chunks->used += size;
		return ret;
	}
	void *calloc(SQUnsignedInteger size) {
		void *chunk = alloc(size);
		memset(chunk,0,size);
		return chunk;
	}
	void reset()
	{
		if(freechunks)
		{
			pool_chunk *tail = freechunks;
			while(tail->next) {
				tail = tail->next;
			}
			tail->next = chunks;
		}
		else {
			freechunks = chunks;
		}
		chunks = NULL;
	}
	const SQChar *strdup(const SQChar *str)
	{
		SQChar *ret = (SQChar *)alloc((strlen(str) + 1) * sizeof(SQChar));
		scstrcpy(ret,str);
		return ret;
	}
	pool_chunk *freechunks;
	pool_chunk *chunks;
private:
	// No assignment
	memorypool& operator=(const memorypool &tmp);
};


#endif // _SQUIRREL_MEMORYPOOL_H_
