#ifndef _SCHEDD_MESSAGE_H_
#define _SCHEDD_MESSAGE_H_
/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/

#ifdef  __cplusplus
extern "C" {
#endif

#include "cull.h"

#define MAXMSGLEN 256

void schedd_mes_initialize(void);

void schedd_mes_release(void);

lListElem *schedd_mes_obtain_package(void);

void schedd_add_message(u_long32 job_number, u_long32 message_number, ...);

void schedd_add_global_message(u_long32 message_number, ...);

void schedd_log_schedd_info(int bval);

void schedd_mes_commit(lList *job_list, int ignore_category);

void schedd_mes_rollback_job(u_long32 jobid);

void schedd_mes_rollback(void);

#ifdef  __cplusplus
}
#endif


#endif /* _SCHEDD_MESSAGE_H_ */



