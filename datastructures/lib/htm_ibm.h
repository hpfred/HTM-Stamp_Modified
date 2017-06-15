/* Copyright (c) IBM Corp. 2014. */
#ifndef HTM_IBM_H
#define HTM_IBM_H 1

extern void tm_startup_ibm();
extern void tm_shutdown_ibm();

extern void tm_thread_enter_ibm();
extern void tm_thread_exit_ibm();

extern int tbegin_ibm(int region_id);
extern void tend_ibm();
extern void tabort_ibm();

#endif
