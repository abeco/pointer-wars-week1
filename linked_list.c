#include "linked_list.h"

// Function pointers to (potentially) custom malloc() and
// free() functions.
//
static void * (*malloc_fptr)(size_t size) = NULL;
static void   (*free_fptr)(void* addr)    = NULL; 

bool linked_list_register_malloc(void* (*malloc_func) (size_t) ) {
    if (malloc_func == NULL) {
        return false;
    }
    malloc_fptr = malloc_func;
    return true;
}

bool linked_list_register_free(void (*free_func)(void*)) {
    if (free_func == NULL) {
        return false;
    }
    free_fptr = free_func;
    return true;
}

struct linked_list* linked_list_create(void) {
    // if allocators aren't defined, don't create list
    if(!malloc_fptr || !free_fptr) {
        return NULL;
    }
    
    struct linked_list* list = malloc_fptr(sizeof(struct linked_list));
    // if allocation fails, return null
    if(list == NULL) {
        return NULL;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

bool linked_list_delete(struct linked_list *ll) {
    // if passed an already empty list, nothing to delete
    if(ll == NULL) {
        return false;
    }
    
    struct node* current = ll->head;
    while(current) {
        struct node* next = current->next;
        free_fptr(current);
        current = next;
    }
    
    ll->head = NULL;
    ll->tail = NULL;
    ll->size = 0;
    return true;
}

size_t linked_list_size(struct linked_list* ll) {
    return (ll == NULL) ? SIZE_MAX : ll->size; 
}

bool linked_list_insert_end(struct linked_list * ll,
                            unsigned int data) {
    if(ll == NULL || !malloc_fptr) {
        return false;
    }
    struct node *new_node = malloc_fptr(sizeof(struct node));
    if(new_node == NULL) {
        return false;
    }
    new_node->data = data;
    new_node->next = NULL;
     
    if(ll->head == NULL) {
        ll->head = new_node;
        ll->tail = new_node;
    } else {
        ll->tail->next = new_node;
        ll->tail = new_node;
    }
    
    ll->size++;
    return true;           
}                            

bool linked_list_insert_front(struct linked_list * ll,
                              unsigned int data) {
    if(ll == NULL || !malloc_fptr) {
        return false;
    }
    struct node *new_node = malloc_fptr(sizeof(struct node));
    if(new_node == NULL) {
        return false;
    }
    new_node->data = data;
    new_node->next = ll->head;
    ll->head = new_node;
    if(ll->tail == NULL) {
        ll->tail = new_node;
    }
    ll->size++;
    return true;                                 
}                              

bool linked_list_insert(struct linked_list * ll,
                        size_t index,
                        unsigned int data) {
    // what is the desired behavior if the index is too high? for now, just return false
    if(ll == NULL || !malloc_fptr || ll->size < index) {
        return false;
    }
    
    // Failing to insert into empty list. Use insert_front
    if(ll->head == NULL) {
        return linked_list_insert_front(ll, data);
    }
    struct iterator *new_iterator = linked_list_create_iterator(ll, index - 1);
    if(new_iterator == NULL) {
        return false;
    }
    
    struct node *new_node = malloc_fptr(sizeof(struct node));
    if(new_node == NULL) {
        linked_list_delete_iterator(new_iterator);
        return false;
    }
    
    new_node->data = data;
    new_node->next = new_iterator->current_node->next;
    new_iterator->current_node->next = new_node;
    
    if(index == 0) {
        ll->head = new_node;
    } else if(new_node->next == NULL) {
        ll->tail = new_node;
    }
    
    ll->size++;
    linked_list_delete_iterator(new_iterator);
    return true;                           
}                        

size_t linked_list_find(struct linked_list * ll,
                        unsigned int data) {
    if(ll == NULL) {
        return SIZE_MAX;
    }
    
    struct node * current = ll->head;
    size_t current_index = 0;
    
    while(current != NULL) {
        if(current->data == data) {
            return current_index;
        } else {
            current = current->next;
            current_index++;
        } 
    }
    
    return SIZE_MAX;
} 

bool linked_list_remove(struct linked_list * ll,
                        size_t index) {
    if(ll == NULL || !free_fptr || ll->size < index) {
        return false;
    }
    
    struct node * to_remove = NULL;
    
    if(index == 0) {
        to_remove = ll->head;
        ll->head = ll->head->next;
        
        if(ll->head == NULL) {
            ll->tail = NULL;
        } 
    } else {
        struct iterator * iter = linked_list_create_iterator(ll, index - 1);
        if(iter == NULL || iter->current_node == NULL || iter->current_node->next == NULL) {
            linked_list_delete_iterator(iter);
            return false;
        }
        
        to_remove = iter->current_node->next;
        if(to_remove->next == NULL) {
            ll->tail = iter->current_node;
            iter->current_node->next = NULL;
        } else {
            iter->current_node->next = to_remove->next;
        }
        
        linked_list_delete_iterator(iter);
    }
    
    free_fptr(to_remove);
    ll->size--;
    return true;                          
}


struct iterator * linked_list_create_iterator(struct linked_list * ll,
                                              size_t index) {
    if(ll == NULL || !malloc_fptr || ll->size < index || ll->size == 0) {
        return NULL;
    }
    
    struct iterator * new_iter = malloc_fptr(sizeof(struct iterator));
    if(new_iter == NULL) {
        return NULL;
    }
    
    struct node * current = ll->head;
    size_t current_index = 0;
    
    while(current != NULL && current_index < index) {
        current = current->next;
        current_index++;
    }
    
    new_iter->ll = ll;
    new_iter->current_node = current;
    new_iter->current_index = current_index;
    new_iter->data = new_iter->current_node->data;
    
    return new_iter;
                                             
} 

bool linked_list_delete_iterator(struct iterator * iter) {
    if(iter == NULL || !free_fptr) {
        return false;
    }
    
    free_fptr(iter);
    return true;
}

bool linked_list_iterate(struct iterator * iter) {
    if(iter == NULL || iter->current_node == NULL) {
        return false;
    }
    
    if(iter->current_node->next == NULL) {
        return false;
    }
    
    iter->current_node = iter->current_node->next;
    iter->current_index++;
    iter->data = iter->current_node->data;
    return true;
}