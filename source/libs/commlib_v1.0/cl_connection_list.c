#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>


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


#include "cl_connection_list.h"
#include "cl_message_list.h"
#include "cl_communication.h"


int cl_connection_list_setup(cl_raw_list_t** list_p, char* list_name) {  /* CR check */
   return cl_raw_list_setup(list_p,list_name , 1); /* enable list locking */
}

int cl_connection_list_cleanup(cl_raw_list_t** list_p) {  /* CR check */
   return cl_raw_list_cleanup(list_p);
}


int cl_connection_list_append_connection(cl_raw_list_t* list_p,cl_com_connection_t* connection , int do_lock) {

   int ret_val;
   cl_connection_list_elem_t* new_elem = NULL;

   if (connection == NULL || list_p == NULL) {
      return CL_RETVAL_PARAMS;
   }

   /* lock the list */
   if (do_lock != 0) {
      if (  ( ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }

   /* add new element list */
   new_elem = (cl_connection_list_elem_t*) malloc(sizeof(cl_connection_list_elem_t));
   if (new_elem == NULL) {
      if (do_lock != 0) {
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }

   new_elem->connection = connection;
   new_elem->raw_elem = cl_raw_list_append_elem(list_p, (void*) new_elem);
   if ( new_elem->raw_elem == NULL) {
      free(new_elem);
      if (do_lock != 0) {
         cl_raw_list_unlock(list_p);
      }
      return CL_RETVAL_MALLOC;
   }
   
   /* unlock the thread list */
   if (do_lock != 0) {
      if (  ( ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
   return CL_RETVAL_OK;
}


int cl_connection_list_remove_connection(cl_raw_list_t* list_p, cl_com_connection_t* connection, int do_lock ) {  /* CR check */
   int ret_val = CL_RETVAL_CONNECTION_NOT_FOUND;
   cl_connection_list_elem_t* elem = NULL;
   

   if (list_p == NULL || connection == NULL) {
      return CL_RETVAL_PARAMS;
   }

   /* lock list */
   if (do_lock != 0) {
      if ( (ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }

   elem = cl_connection_list_get_first_elem(list_p);
   while ( elem != NULL) { 
      if (elem->connection == connection) {
         /* found matching element */
         cl_raw_list_remove_elem(list_p, elem->raw_elem);
         free(elem);
         elem = NULL;
         ret_val = CL_RETVAL_OK;
         break;
      }
      elem = cl_connection_list_get_next_elem(list_p, elem);
   } 


   /* unlock the thread list */
   if (do_lock != 0) {
      if (  ( ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
   return ret_val;
}

/* this functions will free all connections, marked to close */
#ifdef __CL_FUNCTION__
#undef __CL_FUNCTION__
#endif
#define __CL_FUNCTION__ "cl_connection_list_destroy_connections_to_close()"
int cl_connection_list_destroy_connections_to_close(cl_raw_list_t* list_p, int do_lock ) {   /* CR check */
   int ret_val = CL_RETVAL_OK;
   cl_connection_list_elem_t* elem = NULL;
   cl_connection_list_elem_t* elem2 = NULL;
   cl_com_connection_t* connection = NULL;


   if (list_p == NULL ) {
      return CL_RETVAL_PARAMS;
   }

   /* lock list */
   if (do_lock != 0) {
      if ( (ret_val = cl_raw_list_lock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }

   CL_LOG(CL_LOG_INFO, "checking for connections to close ...");
   elem = cl_connection_list_get_first_elem(list_p);
   while ( elem != NULL) {
      elem2 = elem;
      connection = elem2->connection;
      elem = cl_connection_list_get_next_elem(list_p, elem);

#if 0
      CL_LOG_STR(CL_LOG_ERROR,"connection host is -->",connection->local->comp_host);
      CL_LOG_STR(CL_LOG_ERROR,"connection name is -->",connection->local->comp_name);
      CL_LOG_INT(CL_LOG_ERROR,"connection   id is -->",connection->local->comp_id);
#endif

      

      if (connection->data_flow_type == CL_CM_CT_MESSAGE ) {
         if (connection->connection_state == CL_COM_CONNECTED &&
             connection->connection_sub_state == CL_COM_DONE) {
            if( cl_raw_list_get_elem_count(connection->received_message_list) == 0 &&
                cl_raw_list_get_elem_count(connection->send_message_list) == 0) {
                 connection->connection_state = CL_COM_CLOSING;   
                 connection->connection_sub_state = CL_COM_DONE_FLUSHED;
                 CL_LOG(CL_LOG_INFO,"setting connection state to close this connection");
            } else {
               cl_message_list_elem_t* melem;
               CL_LOG(CL_LOG_ERROR,"wait for empty message buffers");
               CL_LOG_INT(CL_LOG_WARNING,"receive buffer:",cl_raw_list_get_elem_count(connection->received_message_list) );
               cl_raw_list_lock(connection->received_message_list);
               melem = cl_message_list_get_first_elem(connection->received_message_list);
               if (melem != NULL) {
                  CL_LOG_INT(CL_LOG_WARNING,"message_df =",melem->message->message_df );
               }
               cl_raw_list_unlock(connection->received_message_list);

               CL_LOG_INT(CL_LOG_WARNING,"send buffer   :",cl_raw_list_get_elem_count(connection->send_message_list) );
            }
         }
      }

      /* check connection timeout time */
      if (connection->handler != NULL) {
         if (connection->connection_state == CL_COM_CONNECTED  || 
             connection->connection_state == CL_COM_OPENING    ||
             connection->connection_state == CL_COM_CONNECTING ) {
            int h_timeout = connection->handler->connection_timeout;
            struct timeval now;
            gettimeofday(&now,NULL);
            CL_LOG(CL_LOG_INFO,"handle connection timeout times");
            if (connection->last_transfer_time.tv_sec + h_timeout <= now.tv_sec) {
               if (connection->data_flow_type == CL_CM_CT_MESSAGE) {
                  CL_LOG(CL_LOG_WARNING,"got connection transfer timeout");
                  if (connection->connection_state == CL_COM_CONNECTED) {
                     CL_LOG_STR(CL_LOG_WARNING,"closing connection to host:", connection->remote->comp_host );
                     CL_LOG_STR(CL_LOG_WARNING,"component name:            ", connection->remote->comp_name );
                     CL_LOG_INT(CL_LOG_WARNING,"component id:              ", connection->remote->comp_id );
                  } else {
                     CL_LOG(CL_LOG_WARNING,"closing unconnected connection object");
                  }
                  
                  connection->connection_state = CL_COM_CLOSING;
               } 
               if (connection->data_flow_type == CL_CM_CT_STREAM) {
                  CL_LOG(CL_LOG_INFO,"ignore connection transfer timeout for stream connection");
                  if (  connection->remote != NULL) {
                     CL_LOG_STR(CL_LOG_INFO,"component host:", connection->remote->comp_host );
                     CL_LOG_STR(CL_LOG_INFO,"component name:", connection->remote->comp_name );
                     CL_LOG_INT(CL_LOG_INFO,"component id:  ", connection->remote->comp_id );
                  }
               }
               if (connection->data_flow_type == CL_CM_CT_UNDEFINED) {
                  CL_LOG(CL_LOG_WARNING,"got connection transfer timeout for undefined connection type");
                  if (connection->local != NULL) {
                     if (connection->local->comp_host != NULL) {
                        CL_LOG_STR(CL_LOG_WARNING,"closing local connection object", connection->local->comp_host );
                     }
                     if (connection->local->comp_name != NULL) {
                        CL_LOG_STR(CL_LOG_WARNING,"component name:", connection->local->comp_name );
                     }
                     CL_LOG_INT(CL_LOG_WARNING,"component id:  ", connection->local->comp_id );
                  } else {
                     CL_LOG(CL_LOG_WARNING,"removing undefined connection object");
                  }
                  connection->connection_state = CL_COM_CLOSING;
               }
            }
         }
      }


      if (connection->connection_state == CL_COM_CLOSING ) {
         /* found connection to close  */
         cl_com_message_t* message = NULL;
         cl_message_list_elem_t* message_list_elem = NULL;

         cl_raw_list_lock( connection->received_message_list );
         while((message_list_elem = cl_message_list_get_first_elem(connection->received_message_list)) != NULL) {
            message = message_list_elem->message;
            if (message->message_state == CL_MS_READY) {
               CL_LOG(CL_LOG_ERROR,"deleting unread message for connection");
            }
            if (message->message_length != 0) {
               CL_LOG_INT(CL_LOG_ERROR,"connection sub_state:",connection->connection_sub_state);
               CL_LOG(CL_LOG_ERROR,"deleting read message for connection");
               CL_LOG_INT(CL_LOG_ERROR,"message length:", message->message_length);
               CL_LOG_INT(CL_LOG_ERROR,"message state:", message->message_state);
               CL_LOG_INT(CL_LOG_ERROR,"message df:", message->message_df);
               CL_LOG_INT(CL_LOG_ERROR,"message mat:", message->message_mat);
            }

            cl_raw_list_remove_elem( connection->received_message_list,  message_list_elem->raw_elem);
            free(message_list_elem);
            cl_com_free_message(&message);
         }
         cl_raw_list_unlock( connection->received_message_list );
         

         cl_raw_list_lock( connection->send_message_list );
         while((message_list_elem = cl_message_list_get_first_elem(connection->send_message_list)) != NULL) {
            message = message_list_elem->message;
            CL_LOG(CL_LOG_ERROR,"deleting unsend message for connection");
            cl_raw_list_remove_elem( connection->send_message_list,  message_list_elem->raw_elem);
            free(message_list_elem);
            cl_com_free_message(&message);
         }
         cl_raw_list_unlock( connection->send_message_list );

         cl_raw_list_remove_elem(list_p, elem2->raw_elem);
         free(elem2);
         elem2 = NULL;

         if (connection->handler != NULL) {
            connection->handler->statistic->bytes_sent = connection->handler->statistic->bytes_sent + connection->statistic->bytes_sent;
            connection->handler->statistic->bytes_received = connection->handler->statistic->bytes_received + connection->statistic->bytes_received;
            connection->handler->statistic->real_bytes_sent = connection->handler->statistic->real_bytes_sent + connection->statistic->real_bytes_sent;
            connection->handler->statistic->real_bytes_received = connection->handler->statistic->real_bytes_received + connection->statistic->real_bytes_received;
         }

         if ( (ret_val=cl_com_close_connection(&connection)) != CL_RETVAL_OK) {  
            CL_LOG(CL_LOG_ERROR, "error closing connection");
            if (do_lock != 0) {
               cl_raw_list_unlock(list_p);
            }
            return ret_val;
         }
         continue;
      }
   } 

   /* unlock list */
   if (do_lock != 0) {
      if ( (ret_val = cl_raw_list_unlock(list_p)) != CL_RETVAL_OK) {
         return ret_val;
      }
   }
   return ret_val;
}








cl_connection_list_elem_t* cl_connection_list_get_first_elem(cl_raw_list_t* list_p) {   /* CR check */
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_first_elem(list_p);
   if (raw_elem) {
      return (cl_connection_list_elem_t*) raw_elem->data;
   }
   return NULL;
}

cl_connection_list_elem_t* cl_connection_list_get_least_elem(cl_raw_list_t* list_p) {   /* CR check */
   cl_raw_list_elem_t* raw_elem = cl_raw_list_get_least_elem(list_p);
   if (raw_elem) {
      return (cl_connection_list_elem_t*) raw_elem->data;
   }
   return NULL;
}


cl_connection_list_elem_t* cl_connection_list_get_next_elem(cl_raw_list_t* list_p, cl_connection_list_elem_t* elem) {  /* CR check */

   cl_raw_list_elem_t* next_raw_elem = NULL;
   
   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      next_raw_elem = cl_raw_list_get_next_elem(raw_elem);
      if (next_raw_elem) {
         return (cl_connection_list_elem_t*) next_raw_elem->data;
      }
   }
   return NULL;
}


cl_connection_list_elem_t* cl_connection_list_get_last_elem(cl_raw_list_t* list_p, cl_connection_list_elem_t* elem) {  /* CR check */

   cl_raw_list_elem_t* last_raw_elem = NULL;
   
   if (elem != NULL) {
      cl_raw_list_elem_t* raw_elem = elem->raw_elem;
      last_raw_elem = cl_raw_list_get_last_elem(raw_elem);
      if (last_raw_elem) {
         return (cl_connection_list_elem_t*) last_raw_elem->data;
      }
   }
   return NULL;
}

