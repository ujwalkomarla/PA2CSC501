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
		//for(j=0;j<NPROC;j++)
		//{
			frm_tab[i].fr_pid = -1;
			frm_tab[i].fr_vpno = -1;
		//}
		//frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = -1;
		frm_tab[i].fr_dirty = 0;
		frm_tab[i].fr_loadtime = 0;
	}
//Initialize free list of frames
	free_frm_list *tmp;
	freehead = usedhead = NULL;
	//iter = &freehead;
	for(i=0;i<5;i++){//First 5 Frames 'Hard' Used
		tmp = getmem(sizeof(used_frm_list));
		tmp->frameno = i;
		tmp->next = usedhead;
		usedhead = tmp;
	}

	for(i=5;i<NFRAMES;i++){
		tmp = getmem(sizeof(free_frm_list));
		tmp->frameno = i;
		tmp->next = freehead;
		freehead = tmp;
	}
/*#ifdef DEBUGuser
free_frm_list *t = usedhead;
while(t!=NULL){
kprintf("%x  %d   %x->",t,t->frameno,t->next);
t = t->next;
}
kprintf("\n");
#endif
#ifdef DEBUGuser
t = freehead;
while(t!=NULL){
kprintf("%x  %d   %x->",t,t->frameno,t->next);
t = t->next;
}
kprintf("\n");
#endif*/

  	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
#ifdef DEBUGuser
//kprintf("avail address %x, Val %d\n",&avail,*avail);
#endif
	if(page_replace_policy == FIFO){
		if(freehead != NULL){ //In FIFO, check if free list is not empty. Then transfer a frame to used list
			free_frm_list *freefrm = freehead;
			freehead = freehead->next;
			freefrm->next = NULL;
		
			//Add freefrm to used list(usedhead) at the tail.
			used_frm_list **t;
			t = &usedhead;
			while(*t != NULL){
#ifdef DEBUGuser
//kprintf("Frame %d, Type %d\n",(*t)->frameno,frm_tab[(*t)->frameno].fr_type);
#endif
				t = &(*t)->next;

			}
			*t = freefrm;
#ifdef DEBUGuser
//kprintf("Frame %d, Type %d\n",(*t)->frameno,frm_tab[(*t)->frameno].fr_type);
#endif
			//return the free frame number ranging from 0 to NFRAMES
			*avail = freefrm->frameno;
#ifdef DEBUGuser
//kprintf("avail free address %x, Val %d\n",&avail,*avail);
#endif
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
			if(*t == NULL){
 #ifdef DEBUGuser
kprintf("error");
#endif
return SYSERR;
}			used_frm_list *swapfrm = *t;
			if(*t == usedhead) usedhead = usedhead->next;
			else{			
				*t = (*t)->next;
			}
		
			swapfrm->next = NULL;
	#ifdef DEBUGuser
//kprintf("Frame %d, Type %d\n",(*t)->frameno,frm_tab[(*t)->frameno].fr_type);
#endif		
			free_frm(swapfrm->frameno);

			//Add swapfrm node to used list(usedhead) at the tail.
			t = &usedhead;
			while(*t != NULL)
				t = &(*t)->next;
			*t = swapfrm;

			*avail = swapfrm->frameno;
#ifdef DEBUGuser
//kprintf("avail swap frame Val %d\n",*avail);
#endif
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
	int procid,bs_id,offset,j;
	int *pagedir,dirty = 0,vpg;
	virt_addr_t addr;
	pd_t *pd;
	pt_t *pt;
	//for(j=0;j<frm_tab[i].fr_refcnt;j++){ 
		procid = frm_tab[i].fr_pid;
		pagedir = (int*)proctab[procid].pdbr;
		vpg = frm_tab[i].fr_vpno;
		addr.pd_offset = (((vpg)*NBPG) >> 22);
		addr.pt_offset = ((((vpg)*NBPG) >> 12) & 0x3FF);
		addr.pg_offset = (((vpg)*NBPG) & 0xFFF);
		pd = (pd_t*)(pagedir+addr.pd_offset);
		pt = (pt_t*)((int*)(pd->pd_base*NBPG)+addr.pt_offset);
		
		if(pt->pt_dirty == 1){
			dirty = 1;
			pt->pt_dirty = 0;
		}
		//ProcBSlist *ins = &proctab[procid].pBSlist;
if(proctab[currpid].vhpno <= vpg && (proctab[currpid].vhpno + proctab[currpid].vhpnpages) >vpg){//HEAP
		bs_id = proctab[currpid].store;
		offset = vpg - proctab[currpid].vhpno;
	}else{//FILES
		ProcBSlist *t = proctab[currpid].pBSlist;
		while(t != NULL){
			if(t->vhpno<=vpg && (t->vhpno + t->vhpnpages)>vpg){
				bs_id = t->store;
				offset = vpg - t->vhpno;
				break;
			}
			t = t->next;
		}
}
		/*while(ins != NULL){
			if(ins->vhpno*NBPG<vpg && (ins->vhpno+ins->vhpnpages)*NBPG > vpg) break;
			ins = ins->next;
		}
		if(ins==NULL) return SYSERR;//No Backing store mapped to process	
		bs_id = ins->store;
		//offset = (ins->vhpno+vhpnpages) - vpg;
		offset = vpg - ins->vhpno;*/
		
		
		
		//Invalidate Entry in page table?
		pt->pt_pres = 0;
		
		*(int*)(pt) = 0;
		frm_tab[i].fr_pid = -1;
		frm_tab[i].fr_vpno = -1;
	/*}*/
#ifdef DEBUGuser
//kprintf("Frame %d, Dirty %d\n",i,dirty);
//kprintf("BS_ID %d, offset %d\n",bs_id,offset);
#endif		
	if(dirty)
		write_bs(((i+FRAME0) * NBPG),bs_id,offset);

	frm_tab[i].fr_status = FRM_UNMAPPED;
	//frm_tab[i].fr_refcnt = 0;
	frm_tab[i].fr_type = -1;


	write_cr3(proctab[currpid].pdbr);	
  return OK;
}



