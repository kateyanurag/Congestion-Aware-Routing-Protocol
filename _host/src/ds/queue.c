#include <"queue.h">
#include <"host.h">
#include <stdio.h>

void
init_demand_queue() {
	demand_qstart = NULL;
	demand_qend = NULL;
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
enqueue_demand(int socket, host_demand *demand) {
	struct demand_qnode *new_node = (struct demand_qnode *)malloc(sizeof(struct demand_qnode));
    	new_node->socket = socket;
    	new_node->demand = demand;
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
}

void
dequeue_demand(int *socket, host_demand **demand) {
	if(demands_qstart != NULL){
		demand_qnode *temp = demands_qstart;
		demand = temp->demand;
		socket = temp->socket;
		if(demands_qstart == demands_qend){
			free(temp);
			demands_qstart = NULL;
			demands_qend = NULL;
			return;
		}
		demands_qstart = demands_qstart->next;
		demands_qstart->prev = NULL;
		free(temp);
	}
}

void
print_demand_queue() {
	demand _qnode *temp = demands_qstart;
	printf("\n Demands Queue: ")
	while(temp != demands_qend->next) {
		printf(" %d-%d-%d  ", temp->socket, temp->demand->flow, temp->demand->dest);
		temp = temp->next;
	}

}
