#ifndef CreateStruct_H
#define CreateStruct_H

#define define_struct_2_fields_no_move(stuct_name, type1, name1, type2, name2)	\
    struct stuct_name { 							\
	type1 name1; 								\
	type2 name2; 								\
	stuct_name() = default;							\
	stuct_name(type1 name1, type2 name2): name1{name1}, name2{name2} {}	\
    };

#define define_struct_3_fields_no_move(stuct_name, type1, name1, type2, name2, type3, name3) 		\
    struct stuct_name { 										\
	type1 name1; 											\
	type2 name2; 											\
	type3 name3;											\
	stuct_name() = default;										\
	stuct_name(type1 name1, type2 name2, type3 name3): name1{name1}, name2{name2}, name3{name3} {}	\
    };

#endif
