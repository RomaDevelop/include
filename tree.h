#ifndef TREE_H
#define TREE_H

#include <list>
#include <memory>
#include <string>

template<class T>
struct node
{
	T value;

	node *parent = nullptr;
	std::list<std::unique_ptr<node>> childs;
	node *next_sibling = nullptr;

	///\brief обход всего дерева
	node* next_in_tree() const
	{
		if(!childs.empty()) return childs.front().get();

		if(next_sibling) return next_sibling;

		const node *tmp_node = this;
		while(tmp_node->parent)
		{
			tmp_node = tmp_node->parent;
			if(tmp_node->next_sibling)
				return tmp_node->next_sibling;
		}

		return nullptr;
	}

	///\brief обход ветви
	node* next_in_branch(const node *head_node) const
	{
		if(!childs.empty()) return childs.front().get();
		
		if(head_node == this) return nullptr;

		if(next_sibling) return next_sibling;

		const node *tmp_node = this;
		while(tmp_node->parent)
		{
			tmp_node = tmp_node->parent;
			if(tmp_node == head_node) return nullptr;
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

struct node_tests
{
	struct test_result { bool correct; std::string text; };
	static test_result test()
	{
		node<int> headStor;
		auto head = &headStor;

		head->value = 0;
		auto node11 = head->add_child(11);
		auto node12 = head->add_child(12);
		auto node13 = head->add_child(13);
		head->add_child(14);

		node11->add_child(111);
		node11->add_child(112);
		node11->add_child(113);

		auto node121 = node12->add_child(121);
		node12->add_child(122);

		node13->add_child(131);
		node13->add_child(132);

		test_result result;

		auto iter_node = head;
		while(iter_node)
		{
			result.text += std::to_string(iter_node->value) + ';';
			iter_node = iter_node->next_in_tree();
		}
		result.text += '\n';

		iter_node = node12;
		while(iter_node)
		{
			result.text += std::to_string(iter_node->value) + ';';
			iter_node = iter_node->next_in_tree();
		}
		result.text += '\n';

		iter_node = node11;
		while(iter_node)
		{
			result.text += std::to_string(iter_node->value) + ';';
			iter_node = iter_node->next_in_branch(node11);
		}
		result.text += '\n';

		iter_node = node121;
		while(iter_node)
		{
			result.text += std::to_string(iter_node->value) + ';';
			iter_node = iter_node->next_in_branch(node121);
		}
		result.text += '\n';

		std::string correctResult = "0;11;111;112;113;12;121;122;13;131;132;14;\n"
													 "12;121;122;13;131;132;14;\n"
									  "11;111;112;113;\n"
														"121;\n";

		result.correct = result.text == correctResult;
		if(!result.correct) result.text += "\nERROR: incorrect result, expected:\n" + correctResult;
		else result.text += "\nthe result is correct";
		return result;
	}
};

template<class T>
struct tree
{
	std::unique_ptr<node<T>> head;
};

#endif
