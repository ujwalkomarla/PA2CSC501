/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  	int i,j;
	for(i=0;i<NFRAMES;i++)
	{
		frm_tab[i].fr_status = FRM_UNMAPPED;
		for(j=0;j<NPROC;j++)
		{
			frm_tab[i].fr_pid[j] = -1;
			frm_tab[i].fr_vpno[j] = -1;
		}
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = -1;
		frm_tab[i].fr_dirty = 0;
		frm_tab[i].fr_loadtime = 0;
	}
//Initialize free list of frames
	free_frm_List *tmp;
	freehead = usedhead = NULL;
	//iter = &freehead;
	for(i=0;i<5;i++){
		tmp = getmem(sizeof(freeFrmList));
		tmp->frameno = i;
		tmp->next = usedhead;
		usedhead = tmp;
	}

	for(i=5;i<NFRAMES;i++){
		tmp = getmem(sizeof(freeFrmList));
		tmp->frameno = i;
		tmp->next = freehead;
		freehead = tmp;
	}

  	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
	if(page_replace_policy == FIFO){
		if(freehead != NULL){ //In FIFO, check if free list is not empty. Then transfer a frame to used list
			freefrm = freehead;
			freehead = freehead->next;
			freefrm->next = NULL;
		
			//Add freefrm to used list(usedhead) at the tail.
			used_frm_list **t;
			t = &usedhead;
			while(*t != NULL)
				t = &(*t)->next;
			*t = freefrm;

			//return the free frame number ranging from 0 to NFRAMES
			*avail = freefrm->frameno;
			return OK;
		}else{ //If free list is empty, choose a frame from used list and free it and add it back to the end of the used list
			used_frm_list **t;
			t = &usedhead;
			while(*t != NULL){
				if(frm_tab[(*t)->frameno].fr_type==FR_PAGE)
					break;
				t = &(*t)->next;
			}
		
			//Removing the Node from used list(usedhead) and copy to swapfrm
			if(*t == NULL) return SYSERR;
			used_frm_list *swapfrm = *t;
			if(*t == usedhead) usedhead = usedhead->next;
			else{			
				*t = (*t)->next;
			}
		
			swapfrm->next = NULL;
			free_frm(swapfrm->frameno);
		
			//Add swampfrm node to used list(usedhead) at the tail.
			t = &usedhead;
			while(*t != NULL)
				t = &(*t)->next;
			*t = swapfrm;

			*avail = swapfrm->frameno;
			return OK;
		}
	}//LRU

  return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{

  kprintf("To be implemented!\n");
  return OK;
}



