if(page_replace_policy == FIFO)
{
	if(freehead != NULL) //In FIFO, check if free list is not empty. Then transfer a frame to used list
	{
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
	}
	else //If free list is empty, choose a frame from used list and free it and add it back to the end of the used list
	{
		
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
}
