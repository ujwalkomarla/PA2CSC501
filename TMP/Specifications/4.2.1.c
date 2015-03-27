Add this struct in paging.h
	typedef struct _lookupBS4Vaddr{
			int     store;                  /* backing store */
		    int     vhpno;                  /* starting pageno */
		    int     vhpnpages; 				/* number of pages */
			//int isHeap;
			struct* ProcBSlist next;
	}ProcBSlist;
	





Add a variable in proc.h
	proctab[currpid].pBSlist;







In initialize : 'NULL' it
	proctab[pid].pBSlist = NULL;





xmmap : add to list
	SYSCALL xmmap(int virtpage, bsd_t source, int npages)
	{
	  /* sanity check ! */
	  STATWORD ps;	
	  disable(ps);
	  if ( (virtpage < 4096) || ( source < 0 ) || ( source > MAX_ID) ||(npages < 1) || ( npages > 100)){
	 	restore(ps);
		return SYSERR;
	  }
		if(bsm_tab[source].bs_pid[currpid] != 1) return SYSERR;//Backing store not mapped to process
		bsm_tab[source].bs_vpno[currpid] = virtpage;
	

		ProcBSlist *ins = &proctab[currpid].pBSlist;
		if(ins==NULL) return SYSERR;//No Backing store mapped to process
		while(ins->store!=bs_id){
			ins = ins->next;
		}
		ins->vhpnpages = bsm_tab[source].bs_npages;
		ins->vhpno = virtpage;
		  restore(ps);
		  return OK;
	}




xmunmap : remove from list

	SYSCALL xmunmap(int virtpage )
	{
	  /* sanity check ! */
	  if ( (virtpage < 4096) ){ 
		kprintf("xmummap call error: virtpage (%d) invalid! \n", virtpage);
		return SYSERR;
	  }


	  return SYSERR;
	}












Inverted page table : Use frm_tab to get pid n vpageno, then from ProcBSlist get the backing store number and offset page.

