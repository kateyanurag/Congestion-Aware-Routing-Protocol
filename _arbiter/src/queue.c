#include <queue.h>
//#include <host.h>
#include <stdio.h>
#include <stdlib.h>

demand_qnode *demand_qstart, *demand_qend;
static int queuesize = 0;

inline int
queue_size(){
	return queuesize;
}

void
init_demand_queue() {
	demand_qstart = NULL;
	demand_qend = NULL;
}

short
is_demand_qempty(){
	if(demand_qstart != NULL)
		return 0;
	return 1;
}

void
free_demand_queue() {
	demand_qnode *ptr = demand_qstart;
	while(ptr != demand_qend) {
		 ptr = demand_qend->next;
        		free(demand_qend);
        		demand_qend = ptr;
	}
	free(demand_qend);
	demand_qstart = NULL;
	demand_qend = NULL;
}


void
enqueue_demand(demand_qnode_data *data, host_req_queue *queue) {
	/* struct demand_qnode *new_node = (struct demand_qnode *)malloc(sizeof(struct demand_qnode));
    	new_node->data = data;
    	new_node->prev = NULL;
    	new_node->next = NULL;
    	if(demand_qend == NULL && demand_qstart == demand_qend){
    		demand_qstart = new_node;
    		demand_qend = new_node;
    	}else{
    		demand_qend->next = new_node;
    		new_node->prev = demand_qend;
    		demand_qend = new_node;
    	}
	queuesize++; */
	
	struct demand_qnode *new_node = (struct demand_qnode *)malloc(sizeof(struct demand_qnode));
    	new_node->data = data;
    	new_node->prev = NULL;
    	new_node->next = NULL;
	pthread_mutex_lock(&(queue->lock));
    	if(queue->end == NULL && queue->start == queue->end){
    		queue->start = new_node;
    		queue->end = new_node;
    	}else{
    		queue->end->next = new_node;
    		new_node->prev = queue->end;
    		queue->end = new_node;
    	}
//	queuesize++;
	queue->qlen++;
	pthread_mutex_unlock(&(queue->lock));
}

void
dequeue_demand(demand_qnode_data *ret_data, host_req_queue *queue) {
	/*if(demand_qstart != NULL){
		demand_qnode *temp = demand_qstart;
		ret_data->src = temp->data->src;
		ret_data->dest = temp->data->dest;
		ret_data->flow = temp->data->flow;
		ret_data->port = temp->data->port;
		ret_data->vlanid = temp->data->vlanid;
		free(temp->data);
		if(demand_qstart == demand_qend){
			free(temp);
			demand_qstart = NULL;
			demand_qend = NULL;
			queuesize--;
			return;
		}
		demand_qstart = demand_qstart->next;
		demand_qstart->prev = NULL;
		free(temp);
		queuesize--;
	}
	return;*/
	
	pthread_mutex_lock(&(queue->lock));
	if(queue->start != NULL){
		demand_qnode *temp = queue->start;
		ret_data->src = temp->data->src;
		ret_data->dest = temp->data->dest;
		ret_data->flow = temp->data->flow;
		ret_data->port = temp->data->port;
		ret_data->vlanid = temp->data->vlanid;
		free(temp->data);
		if(queue->start == queue->end){
			free(temp);
			queue->start = NULL;
			queue->end = NULL;
			queue->qlen--;
			return;
		}
		queue->start = queue->start->next;
		queue->start->prev = NULL;
		free(temp);
		queue->qlen--;
	}
	pthread_mutex_unlock(&(queue->lock));
	return;
}

void
peep_dequeue_demand(demand_qnode_data *ret_data, host_req_queue *queue) {
	
	pthread_mutex_lock(&(queue->lock));
	if(queue->start != NULL){
		demand_qnode *temp = queue->start;
		ret_data->src = temp->data->src;
		ret_data->dest = temp->data->dest;
		ret_data->flow = temp->data->flow;
		ret_data->port = temp->data->port;
		ret_data->vlanid = temp->data->vlanid;
		//free(temp->data);
	/*	if(queue->start == queue->end){
			free(temp);
			queue->start = NULL;
			queue->end = NULL;
			queue->qlen--;
			return;
		}
		queue->start = queue->start->next;
		queue->start->prev = NULL;
		free(temp);
		queue->qlen--;*/
	}
	pthread_mutex_unlock(&(queue->lock));
	return;
}

void
print_demand_queue() {
	//demand_qnode *temp = demand_qstart;
	printf("\n Demands Queue: ");
	/*while(temp != demands_qend->next) {
		printf(" %d-%d-%d  ", temp->socket, temp->demand->flow, temp->demand->dest);
		temp = temp->next;
	}*/
}
