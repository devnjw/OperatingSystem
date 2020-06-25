#include <unistd.h>
#include <stdio.h>
#include "smalloc.h" 

sm_container_t sm_head = {
	0,
	&sm_head, 
	&sm_head,
	0 
} ;

static 
void * 
_data (sm_container_ptr e)
{
	return ((void *) e) + sizeof(sm_container_t) ;
}

static 
void 
sm_container_split (sm_container_ptr hole, size_t size)
{
	sm_container_ptr remainder = (sm_container_ptr) (_data(hole) + size) ;

	remainder->dsize = hole->dsize - size - sizeof(sm_container_t) ;	
	remainder->status = Unused ;
	remainder->next = hole->next ;
	remainder->prev = hole ;
	hole->dsize = size ;
	hole->next->prev = remainder ;
	hole->next = remainder ;
}

static 
void * 
retain_more_memory (int size)
{
	sm_container_ptr hole ;
	int pagesize = getpagesize() ;
	int n_pages = 0 ;

	n_pages = (sizeof(sm_container_t) + size + sizeof(sm_container_t)) / pagesize  + 1 ;
	hole = (sm_container_ptr) sbrk(n_pages * pagesize) ;
	if (hole == 0x0)
		return 0x0 ;
	
	hole->status = Unused ;
	hole->dsize = n_pages * getpagesize() - sizeof(sm_container_t) ;
	return hole ;
}

void * 
smalloc (size_t size) 
{
	sm_container_ptr hole = 0x0, itr = 0x0 ;
	
	size_t min = 999999;
	for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
		if (itr->status == Busy)
			continue ;
		if ((itr->dsize == size) || (size + sizeof(sm_container_t) < itr->dsize)) {
			if(min >= itr->dsize)
			{
				min = itr->dsize;
				hole = itr ; 
			}
		}
	}
	if (hole == 0x0) {
		hole = retain_more_memory(size) ;
		if (hole == 0x0)
			return 0x0 ;
		hole->next = &sm_head ;
		hole->prev = sm_head.prev ;
		(sm_head.prev)->next = hole ;
		sm_head.prev = hole ;
	}
	if (size < hole->dsize) 
		sm_container_split(hole, size) ;
	hole->status = Busy ;
	return _data(hole) ;
}

static
void *
bestfit(size_t size){
    sm_container_ptr hole = 0x0, itr = 0x0 ;
    size_t min = 999999 ;

    for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next)
    {
	if (itr->status == Busy)
	    continue ;
	if ((itr->dsize == size) || size + sizeof(sm_container_t) < itr->dsize) {
	    if(min >= itr->dsize)
	    {
		min = itr->dsize ;
		hole = itr ;
	    }
	}
    }
    return hole ;
}

static
void *
extend(size_t size)
{
	sm_container_ptr hole = retain_more_memory(size) ;
	if (hole == 0x0)
		return 0x0 ;
	hole->next = &sm_head ;
	hole->prev = sm_head.prev ;
	(sm_head.prev)->next = hole ;
	sm_head.prev = hole ;

	return hole ;
}

static
void *
migrate (sm_container_ptr hole, sm_container_ptr temp)
{
    //*temp = *hole ;
    sfree(_data(hole)) ;
    return temp ;
}

void *
srealloc (void * p, size_t newsize)
{
    sm_container_ptr hole = 0x0, itr = 0x0, temp = 0x0 ;
    for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next){
	if (p == _data(itr)) {
	    hole = itr ;
	    break ;
	}
    }
    if(newsize < hole->dsize){
	    sm_container_split(hole, newsize) ;
	    sfree(_data(hole->next)) ;
    }
    else if(hole->next == &sm_head){
	    temp = extend(newsize - hole->dsize) ;
	    hole->dsize += temp->dsize + sizeof(sm_container_t) ;
	    hole->next = temp->next ;
	    sm_container_split(hole, newsize) ;
    }
    else if(hole->next->status != Unused){
	    temp = bestfit(newsize) ;
	    if(temp == 0x0){
		temp = extend(newsize) ;
	    }
	    hole = migrate(hole ,temp) ;
	    hole->status = Busy ;
	    sm_container_split(hole, newsize) ;
    }
    else if(hole->next->dsize + sizeof(sm_container_t) == newsize - hole->dsize){
	    hole->dsize = newsize ;
	    hole->next = hole->next->next ;
    }
    else if(hole->next->dsize > newsize - hole->dsize){
	    hole->dsize += hole->next->dsize + sizeof(sm_container_t) ;
	    hole->next = hole->next->next ;
	    sm_container_split(hole, newsize) ;
    }
    else{
	    temp = bestfit(newsize) ;
	    if(temp == 0x0){
		temp = extend(newsize) ;
	    }
	    hole = migrate(hole, temp) ;
	    hole->status = Busy ;
	    sm_container_split(hole, newsize) ;
    }
    return _data(hole) ;
}	
void
sfree(void * p)
{
    sm_container_ptr itr ;
    for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
	if (p == _data(itr)) {
	    itr->status = Unused ;
	    break ;
	}
    }
    for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
	if(itr->status == Unused && itr->next != &sm_head) {
	    if(itr->next->status == Unused){
		itr->dsize = itr->dsize + itr->next->dsize + sizeof(sm_container_t) ;
		itr->next = itr->next->next ;
		if(itr->next != &sm_head)
		    itr->next->prev = itr ;
		itr = itr->prev ;
	    }
        }
    }
}

void 
print_sm_containers ()
{
	sm_container_ptr itr ;
	int i ;

	printf("==================== sm_containers ====================\n") ;
	for (itr = sm_head.next, i = 0 ; itr != &sm_head ; itr = itr->next, i++) {
		printf("%3d:%p:%s:", i, _data(itr), itr->status == Unused ? "Unused" : "  Busy") ;
		printf("%8d:", (int) itr->dsize) ;

		int j ;
		char * s = (char *) _data(itr) ;
		for (j = 0 ; j < (itr->dsize >= 8 ? 8 : itr->dsize) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;

}

void
print_mem_uses ()
{
    sm_container_ptr itr ;
    int i ;
    int total = 0;
    int mem_alloc = 0;
    int mem_remain = 0;

    printf("==================== mem_uses ====================\n") ;
    for (itr = sm_head.next, i = 0 ; itr != &sm_head ; itr = itr->next, i++) {
	    total += itr->dsize + sizeof(sm_container_t) ;
	    if(itr->status == Unused)
		mem_remain += itr->dsize ;
	    else
		mem_alloc += itr->dsize ;
    }
    printf("The amount of memory retained so far: %d\n", total) ;
    printf("The amount of memory allocated at this moment: %d\n", mem_alloc) ;
    printf("The amount of memory retained but not allocated: %d\n\n", mem_remain) ;
}

void
sshrink()
{
    sm_container_ptr itr ;
    for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
	if(itr->status == Unused)
	{
	    void * curr_brk = sbrk(0) ;
	    
	    int temp = (int)itr->dsize + sizeof(sm_container_t);
	    itr->prev->next = itr->next ;
	    itr->next->prev = itr->prev ;
	    brk(curr_brk - temp) ;
	}
	else
	    break ;
    }

}
