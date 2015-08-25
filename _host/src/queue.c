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
enqueue_demand(demand_qnode_data *data) {
	pthread_mutex_lock(&response_qlock);
	struct demand_qnode *new_node = (struct demand_qnode *)malloc(sizeof(struct demand_qnode));
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
	queuesize++;
	pthread_mutex_unlock(&response_qlock);
}

void
dequeue_demand(demand_qnode_data *ret_data) {
	pthread_mutex_lock(&response_qlock);
	if(demand_qstart != NULL){
		demand_qnode *temp = demand_qstart;
		ret_data->timeslot = temp->data->timeslot;
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
			pthread_mutex_unlock(&response_qlock);
			return;
		}
		demand_qstart = demand_qstart->next;
		demand_qstart->prev = NULL;
		free(temp);
		queuesize--;
	}
	pthread_mutex_unlock(&response_qlock);
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
