#include <stdio.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

//Process Page Directory
SYSCALL ProcPageDir(int pid)
{
	int *freeFrm,i;
	
	#ifdef DEBUGuser
	//kprintf("HereProcPageDirIn\n");
#endif
	get_frm(freeFrm);//GetFreeFrame returns the free frame number in range 0 to NFRAMES

	i = *freeFrm;
	proctab[pid].pdbr = (i+FRAME0) * NBPG;

	frm_tab[i].fr_status = FRM_MAPPED;
	frm_tab[i].fr_pid = pid;
	frm_tab[i].fr_type = FR_DIR;
	frm_tab[i].fr_vpno = 0;
	//frm_tab[i].fr_refcnt = 1;
	pd_t *pgDir = (pd_t*)((i + FRAME0) * NBPG);
	for(i=0;i<4;i++)
	{
		pgDir[i].pd_pres = 1;
		pgDir[i].pd_write = 1;
		pgDir[i].pd_base = FRAME0 + i;
	}
	#ifdef DEBUGuser
	//kprintf("HereProcPageDirOut\n");
#endif
	return OK;
}
