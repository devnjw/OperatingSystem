#include <stdio.h>
#include "smalloc.h"

int 
main()
{
	void *p1, *p2, *p3, *p4, *p5, *p6, *p7 ;

	print_sm_containers() ;

	p1 = smalloc(2000) ; 
	printf("smalloc(2000):%p\n", p1) ; 
	print_sm_containers() ;

	p2 = smalloc(2500) ; 
	printf("smalloc(2500):%p\n", p2) ; 
	print_sm_containers() ;

	p3 = smalloc(1000) ; 
	printf("smalloc(1000):%p\n", p3) ; 
	print_sm_containers() ;

	sfree(p2) ; 
	printf("sfree(%p)\n", p2) ; 
	print_sm_containers() ;

	print_mem_uses() ;

	p4 = smalloc(1000) ; 
	printf("smalloc(1000):%p\n", p4) ; 
	print_sm_containers() ;

	printf("srealloc(%p, 3000)\n", p1) ; 
	p1 = srealloc(p1, 3000) ;
	print_sm_containers() ;

	printf("\n===== Sshinked Heap Memory =====\n") ;
	sshrink() ;
	print_sm_containers() ;
}
