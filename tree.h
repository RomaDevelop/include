#ifndef TREE_H
#define TREE_H

#include <list>
#include <memory>

template<class T>
struct node
{
    T value;

    node *parent = nullptr;
    std::list<std::unique_ptr<node>> childs;
    node *next_sibling = nullptr;

    node* next_in_tree()
    {
	if(!childs.empty()) return childs.front().get();

	if(next_sibling) return next_sibling;

	node *tmp_node = this;
	while(tmp_node->parent)
	{
	    tmp_node = tmp_node->parent;
	    if(tmp_node->next_sibling)
		return tmp_node->next_sibling;
	}

	return nullptr;
    }

    node* add_child(std::unique_ptr<node> new_child)
    {
	node* new_child_ptr = new_child.get();

	node* prevSibling = nullptr;
	if(!childs.empty()) prevSibling = childs.back().get();

	childs.emplace_back(std::move(new_child));

	new_child_ptr->parent = this;
	if(prevSibling) prevSibling->next_sibling = new_child_ptr;
	return new_child_ptr;
    }

    node* add_child(const T &value)
    {
	std::unique_ptr<node> new_child = std::make_unique<node>();
	new_child->value = value;

	return add_child(std::move(new_child));
    }

};

template<class T>
struct tree
{
    std::unique_ptr<node<T>> head;
};

#endif
