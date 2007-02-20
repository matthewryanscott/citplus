5/************************************************************************/
/*                                                                      */
/*              !   !  !!!   !!!!!  !!!!!   !!                          */
/*              !!  ! !   !    !    !       !!                          */
/*              ! ! ! !   !    !    !!!     !!                          */
/*              !  !! !   !    !    !                                   */
/*              !   !  !!!     !    !!!!!   !!                          */
/*                                                                      */
/* Citadel programs use readSysTab() and writeSysTab() to write an      */
/* image of the external variables in RAM to disk, and later restore    */
/* it.  The image is stored in ctdlTabl.sys .  If ctdlTabl.sys is lost, */
/* sysop.com will automatically reconstruct the hard way when invoked,  */
/* and write a new ctdlTabl.sys out when finished.  CtdlTabl.sys is     */
/* always destroyed after reading, to minimize the possibility of       */
/* reading an out-of-date version.  In general, the technique works     */
/* well and saves time and head-banging on bootup.  You should,         */
/* however, note carefully the following cautions:                      */
/*  o  "char firstExtern;" should >always< be the first                 */
/*     declaration in citadel.h                                         */
/*  o  "char lastExtern;"  shoule >always< be the last                  */
/*     declaration in citadel.h                                         */
/*  o  Whenever you change the declarations in 210Ctdl.h you should:    */
/*   -->  destroy the current ctdlTabl.sys file                         */
/*   -->  recompile and reload all citadel programs which access        */
/*        ctdlTabl.sys -- currently citadel.com & consifurg.com         */
/*   -->  use configur.com to build a new ctdlTabl.sys file             */
/*                                                                      */
/* If you ignore these warnings, little pixies will prick you in your   */
/* sleep for the rest of your life.                                     */
/************************************************************************/


/* -------------------------------------------------------------------- */
/*                                                                      */
/*              !   !  !!!   !!!!!  !!!!!   !!                          */
/*              !!  ! !   !    !    !       !!                          */
/*              ! ! ! !   !    !    !!!     !!                          */
/*              !  !! !   !    !    !                                   */
/*              !   !  !!!     !    !!!!!   !!                          */
/*                                                                      */
/* Citadel uses readTables() and writeTables() to write images          */
/* of the external variables in RAM to disk, and later restore          */
/* them.  The images are stored in Etc.tab and all the .TAB files.      */
/* Ctdl.Exe will automatically reconstruct these files the hard way     */
/* when necessary and write them out when finished.                     */
/* Etc.tab and the .TAB files are always destroyed after reading, to    */
/* minimize the possibility of reading out-of-date versions.  In        */
/* general, this technique works well and saves time and head-banging   */
/* upon bootup.  You should, however, note carefully the following      */
/* caution:                                                             */
/*  o  Whenever you change the declarations of any tables you should:   */
/*   -->  Destroy the current Etc.tab file.                             */
/*   -->  Recompile and reload Ctdl.Exe.                                */
/*   -->  Ctdl.Exe will automatically build new Etc.tab and tables.     */
/*                                                                      */
/* If you ignore this warning, little pixies will prick you in your     */
/* sleep for the rest of your life.                                     */
/* -------------------------------------------------------------------- */6
