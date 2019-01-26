/*
	foidl_predicate.c
	Library predicate management

	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define PREDICATE_IMPL
#include <foidlrt.h>
#include <stdio.h>

//	Simple equality

PFRTAny foidl_truthy_qmark(PFRTAny el) {
	PFRTAny 	res = true;
	if(el == nil || el == false)
		res = false;
	return res;
}

PFRTAny foidl_falsey_qmark(PFRTAny el) {
	return foidl_truthy_qmark(el) == true ? false : true;
}

PFRTAny foidl_and(PFRTAny lhs, PFRTAny rhs) {
	return ((foidl_truthy_qmark(lhs) == true)
		&& (foidl_truthy_qmark(rhs) == true)) ? true : false;
}

PFRTAny foidl_or(PFRTAny lhs, PFRTAny rhs) {
	if(foidl_truthy_qmark(lhs) == true)
		return true;
	if(foidl_truthy_qmark(rhs) == true)
		return true;
	return false;
}

PFRTAny foidl_not(PFRTAny el) {
	return foidl_truthy_qmark(el) == true ? false : true;
}

//	Used in equality

PFRTAny scalar_equality(PFRTAny lhs, PFRTAny rhs) {
	PFRTAny 	res = true;

	// If used with two numbers
	if(lhs->ftype == number_type && rhs->ftype == number_type)
		res = foidl_num_equal(lhs, rhs);

	// If different objects dig deeper
	else if(lhs != rhs) {
		if(lhs->ftype == rhs->ftype) {
			if(lhs->ftype != string_type && lhs->ftype != keyword_type) {
				if(lhs->value != rhs->value)
					res = false;
			}
			else {
				if (0 != strcmp((char *)lhs->value,(char *)rhs->value))
					res = false;
			}
		}
		else {
			if( (lhs->ftype == string_type && rhs->ftype == keyword_type)
				||
				(rhs->ftype == string_type && lhs->ftype == keyword_type) ) {
				if (0 != strcmp((char *)lhs->value,(char *)rhs->value))
					res = false;
			}
			else
				res = false;
		}

	}
	return res;
}

//	=, !=, <, <=, >, >=

PFRTAny foidl_equal_qmark(PFRTAny lhs, PFRTAny rhs) {
	PFRTAny res = false;
	if(lhs->fclass == rhs->fclass) {
		switch(lhs->fclass) {
			case 	scalar_class:
				res = scalar_equality(lhs,rhs);
				break;
			case 	io_class:
				if( lhs == rhs )
					res = true;
				break;
			case 	collection_class:
				if( lhs == rhs )
					res = true;
				else
					unknown_handler();
				break;
			default:
				unknown_handler();
				break;
			}
	}
	return res;
}

PFRTAny foidl_not_equal_qmark(PFRTAny lhs, PFRTAny rhs) {
	PFRTAny res = foidl_equal_qmark(lhs,rhs);
	return (res == true) ? false : res;
}

PFRTAny foidl_lt_qmark(PFRTAny lhs, PFRTAny rhs) {
	PFRTAny res = false;
	if(lhs->fclass == rhs->fclass && lhs->ftype == rhs->ftype) {
		switch(lhs->ftype) {
			case 	character_type:
				res = lhs->value < rhs->value ? true : false;
				break;
			case 	number_type:
				res = foidl_num_lt(lhs, rhs);
				break;
			default:
				unknown_handler();
				break;
			}
	}
	return res;
}

PFRTAny foidl_lteq_qmark(PFRTAny lhs, PFRTAny rhs) {
	return (foidl_lt_qmark(lhs,rhs) == true ||
			foidl_equal_qmark(lhs,rhs) == true) ? true : false;
}

PFRTAny foidl_gt_qmark(PFRTAny lhs, PFRTAny rhs) {
	PFRTAny res = false;
	if(lhs->fclass == rhs->fclass && lhs->ftype == rhs->ftype) {
		switch(lhs->ftype) {
			case 	character_type:
				res = lhs->value > rhs->value ? true : false;
				break;
			case 	number_type:
				res = foidl_num_gt(lhs, rhs);
				break;
			default:
				unknown_handler();
				break;
			}
	}
	return res;
}

PFRTAny foidl_gteq_qmark(PFRTAny lhs, PFRTAny rhs) {
	return (foidl_gt_qmark(lhs,rhs) == true ||
			foidl_equal_qmark(lhs,rhs) == true) ? true : false;
}

// Class Equality

PFRTAny foidl_function_qmark(PFRTAny el) {
	return (el->fclass == function_class) ? true : false;
}

PFRTAny foidl_scalar_qmark(PFRTAny el) {
	return (el->fclass == scalar_class) ? true : false;
}

PFRTAny foidl_io_qmark(PFRTAny el) {
	return (el->fclass == io_class) ? true : false;
}


PFRTAny function_strict_arg(PFRTAny fn, PFRTAny cnt) {

	ft val = 0;
	if(cnt->ftype == number_type) {
		val = number_toft(cnt);
	}
	else {
		unknown_handler();
	}
	if(fn->ftype == funcref_type && ((PFRTFuncRef) fn)->argcount == val)
		return true;
	else if(fn->ftype == lambref_type &&
		((PFRTLambdaRef)fn)->ffuncref->argcount == val)
		return true;
	else if(fn->ftype == funcinst_type) {
		PFRTFuncRef2 fn2 = (PFRTFuncRef2) fn;
		if(val == (fn2->mcount - fn2->args->count))
			return true;
		else
			return false;
	}

	else
		return false;
}

PFRTAny foidl_collection_qmark(PFRTAny el) {
	return (el->fclass == io_class ||
		(el->fclass == collection_class &&
		(el->ftype == map2_type || el->ftype == list2_type
		 || el->ftype == list2_type || el->ftype == vector2_type
		 || el->ftype == set2_type|| el->ftype == series_type))) ? true : false;
}

PFRTAny foidl_extendable_qmark(PFRTAny el) {
	return (foidl_collection_qmark(el) == true
		|| el->ftype == string_type
		|| el->ftype == keyword_type) ?
		true : false;
}

PFRTAny foidl_empty_qmark(PFRTAny el) {
	if(foidl_collection_qmark(el) == true || el->ftype == string_type)
		return el->count == 0 ? true : false;
	else
		return true;
}

//	Type equalities

PFRTAny foidl_byte_qmark(PFRTAny el) {
	return (el->ftype == byte_type) ? true : false;
}

PFRTAny foidl_char_qmark(PFRTAny el) {
	return (el->ftype == character_type) ? true : false;
}

PFRTAny foidl_number_qmark(PFRTAny el) {
	return (el->ftype == number_type) ? true : false;
}

PFRTAny foidl_even_qmark(PFRTAny el) {
	PFRTAny res = false;
	if(foidl_number_qmark(el) == true) {
		return foidl_num_even(el);
	}
	else
		unknown_handler();
	return res;
}

PFRTAny foidl_odd_qmark(PFRTAny el) {
	PFRTAny res = false;
	if(foidl_number_qmark(el) == true) {
		return foidl_num_odd(el);
	}
	else
		unknown_handler();
	return res;
}

PFRTAny foidl_positive_qmark(PFRTAny el) {
	PFRTAny res = false;
	if(foidl_number_qmark(el) == true) {
		return is_number_positive(el);
	}
	else
		unknown_handler();
	return res;
}

PFRTAny foidl_negative_qmark(PFRTAny el) {
	PFRTAny res = false;
	if(foidl_number_qmark(el) == true) {
		return is_number_negative(el);
	}
	else
		unknown_handler();
	return res;
}

PFRTAny foidl_integer_qmark(PFRTAny el) {
	PFRTAny res = false;
	if(foidl_number_qmark(el) == true) {
		return is_number_integer(el);
	}
	else
		unknown_handler();
	return res;
}

PFRTAny foidl_string_qmark(PFRTAny el) {
	return (el->ftype == string_type) ? true : false;
}

PFRTAny foidl_keyword_qmark(PFRTAny el) {
	return (el->ftype == keyword_type) ? true : false;
}

PFRTAny foidl_list_qmark(PFRTAny el) {
	return (el->ftype == list2_type) ? true : false;
}

PFRTAny foidl_map_qmark(PFRTAny el) {
	return (el->ftype == map2_type) ? true : false;
}

PFRTAny foidl_mapEntry_qmark(PFRTAny el) {
	return (el->ftype == mapentry_type) ? true : false;
}

PFRTAny foidl_set_qmark(PFRTAny el) {
	return (el->ftype == set2_type) ? true : false;
}

PFRTAny foidl_vector_qmark(PFRTAny el) {
	return (el->ftype == vector2_type) ? true : false;
}

PFRTAny foidl_series_qmark(PFRTAny el) {
	return (el->ftype == series_type) ? true : false;
}


//	Internal type predicates

PFRTAny string_type_qmark(PFRTAny el) {
	return (el->ftype == string_type || el->ftype == keyword_type) ?
		true : false;
}
