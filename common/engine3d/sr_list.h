#pragma once

namespace sr
{

#if 0
	template <typename T>
	struct list_node_s
	{
		list_node_s<T>* prev_node;
		list_node_s<T>* next_node;
	};

	template <typename T>
	struct linked_list_s
	{
		list_node_s<T>* iterator;

		list_node_s<T>* first_node;
		list_node_s<T>* last_node;
	};
#else
	template <typename T>
	struct list_node_s
	{
		T* prev_node;
		T* next_node;
	};

	template <typename T>
	struct linked_list_s
	{
		T* iterator;

		T* first_node;
		T* last_node;
	};
#endif

// api

template <typename T>
void linkedlist_initialize(linked_list_s<T>* list)
{
	list->iterator = nullptr;

	list->first_node = nullptr;
	list->last_node = nullptr;
}


template <typename T>
void linkedlist_erase_node(linked_list_s<T>* list, T* node)
{
	if (node->prev_node == nullptr)
	{
		if (node->next_node)
		{
			node->next_node->prev_node = nullptr;

			list->first_node = node->next_node;
		}

		return;
	}
	else if (node->next_node == nullptr)
	{
		list->last_node = node->prev_node;

		node->prev_node->next_node = nullptr;

		return;
	}

	node->prev_node->next_node = node->next_node;
	node->next_node->prev_node = node->prev_node;
}


template <typename T>
void linkedlist_insert_back(linked_list_s<T>* list, T* node)
{
	if (list->last_node)
	{
		list->last_node->next_node = node;
		node->prev_node = list->last_node;
	}
	else
	{
		node->prev_node = nullptr;
		list->first_node = node;
	}

	list->last_node = node;
	node->next_node = nullptr;
}


template <typename T>
void linkedlist_insert_front(linked_list_s<T>* list, T* node)
{
	if (list->first_node)
	{
		node->next_node = list->first_node;
		list->first_node->prev_node = node;
	}
	else
	{
		node->next_node = nullptr;
		list->last_node = node;
	}

	list->first_node = node;
	node->prev_node = nullptr;
}


template <typename T>
void linkedlist_swap_nodes(linked_list_s<T>* list,
	T* node1, T* node2)
{
	sr::swap(node1->next_node, node2->next_node);
	sr::swap(node1->prev_node, node2->prev_node);

	if (node1->next_node == nullptr)
	{
		list->last_node = node1;
	}
	else if (node2->next_node == nullptr)
	{
		list->last_node = node2;
	}

	if (node1->prev_node == nullptr)
	{
		list->first_node = node1;
	}
	else if (node2->prev_node == nullptr)
	{
		list->first_node = node2;
	}
}


template <typename T>
void linkedlist_reset_iterator(linked_list_s<T>* list)
{
	list->iterator = nullptr;
}


template <typename T>
T* linkedlist_iterate(linked_list_s<T>* list)
{
	if (list->iterator == nullptr)
	{
		return (list->iterator = list->first_node);
	}

	list->iterator = list->iterator->next_node;

	return list->iterator;
}


//void linkedlist_sort(linked_list_t* list);

}
