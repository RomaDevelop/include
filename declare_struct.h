#ifndef declare_struct_H
#define declare_struct_H

#define declare_struct_2_fields_move(struct_name, type1, name1, type2, name2)				\
    struct struct_name {											\
	type1 name1;											\
	type2 name2;											\
        struct_name() = default;										\
        struct_name(type1 name1, type2 name2): name1{std::move(name1)}, name2{std::move(name2)} {}	\
    }

#define declare_struct_2_fields_no_move(struct_name, type1, name1, type2, name2)	\
    struct struct_name { 							\
	type1 name1; 								\
	type2 name2; 								\
        struct_name() = default;							\
        struct_name(type1 name1, type2 name2): name1{name1}, name2{name2} {}	\
    }

#define declare_struct_3_fields_move(struct_name, type1, name1, type2, name2, type3, name3)						\
    struct struct_name {															\
	type1 name1;															\
	type2 name2;															\
	type3 name3;															\
        struct_name() = default;														\
        struct_name(type1 name1, type2 name2, type3 name3): name1{std::move(name1)}, name2{std::move(name2)}, name3{std::move(name3)} {}	\
    }

#define declare_struct_3_fields_no_move(struct_name, type1, name1, type2, name2, type3, name3) 		\
    struct struct_name { 										\
	type1 name1; 											\
	type2 name2; 											\
	type3 name3;											\
        struct_name() = default;										\
        struct_name(type1 name1, type2 name2, type3 name3): name1{name1}, name2{name2}, name3{name3} {}	\
    }

#define declare_struct_4_fields_no_move(struct_name, type1, name1, type2, name2, type3, name3, type4, name4) 		\
    struct struct_name { 										\
        type1 name1; 											\
        type2 name2; 											\
        type3 name3;											\
        type4 name4;											\
        struct_name() = default;									\
        struct_name(type1 name1, type2 name2, type3 name3, type4 name4): name1{name1}, name2{name2}, name3{name3}, name4{name4} {}	\
    }

#endif
