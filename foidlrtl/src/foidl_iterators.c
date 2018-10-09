/*
	foidl_iterators.c
	Library support for data iterations
	
	Copyright Frank V. Castellucci
	All Rights Reserved
*/

#define ITERATORS_IMPL

#include	<foidlrt.h>
#include 	<stdio.h>

//	set iterator: Find the next node that has a value

static 	PFRTAny trieiterator_searchNextValueNode(PFRTTrie_Iterator itr) {
	while(itr->currentStackLevel >= 0) {
		int ccI = itr->currentStackLevel*2;
		int clI = ccI + 1;
		int nodeCursor = itr->nodeCursorAndLength[ccI];
		int nodeLength   = itr->nodeCursorAndLength[clI];
		if( nodeCursor < nodeLength ) {
			PFRTBitmapNode nextNode = 
				itr->ftype == set_iterator_type ?
				set_getNode(itr->nodes[itr->currentStackLevel],nodeCursor) :
				getNode(itr->nodes[itr->currentStackLevel],nodeCursor);
			itr->nodeCursorAndLength[ccI] = nodeCursor+1;
			if( nextNode->nodemap != 0 ) {
				int nextStackLevel = ++itr->currentStackLevel;
				int nextCursorIndex = nextStackLevel * 2;
				int nextLengthIndex = nextCursorIndex + 1;
				itr->nodes[nextStackLevel] = nextNode;
				itr->nodeCursorAndLength[nextCursorIndex] = 0;
				itr->nodeCursorAndLength[nextLengthIndex] = nodeArity(nextNode);

			}
			if( nextNode->datamap != 0 ) {
				itr->currentValueNode = nextNode;
				itr->currentValueCursor = 0;
				itr->currentValueLength = payloadArity(nextNode);
				return true;
			}
		}
		else {
			--itr->currentStackLevel;
		}
	}
	return false;
} 

static PFRTAny trieiterator_hasNext(PFRTTrie_Iterator itr) {
	if(itr->currentValueCursor < itr->currentValueLength) {
		return true;
	}
	else {
		return trieiterator_searchNextValueNode(itr);
	}
}

static PFRTAny trieiterator_nextKey(PFRTTrie_Iterator itr) {
	PFRTAny res = end;
	if( trieiterator_hasNext(itr) == true)
		res = itr->get((PFRTAny) itr->currentValueNode,itr->currentValueCursor++);
	return res;
}

PFRTAny vectoriterator_next(PFRTVector_Iterator itr) {
	PFRTAny res = end;
	if(itr->index < itr->vector->count) {
		if(itr->index - itr->base == WCNT) {
			itr->node = (PFRTHamtNode) itr->get((PFRTAny) itr->vector, itr->index);
			itr->base += WCNT;
		}
		res = itr->node->slots[itr->index & MASK];
		itr->index += 1;
	}
	return res;
}

PFRTAny 	listiterator_next(PFRTList_Iterator itr) {
	PFRTAny res = end;
	if(itr->node != empty_link) {
		res = itr->node->data;
		itr->node = itr->node->next;
	}
	return res;
}

PFRTAny seriesiterator_next(PFRTIterator sitr) {
	PFRTAny res = end;
	PFRTSeries_Iterator serItr = (PFRTSeries_Iterator) sitr;

	//	If we are already at end
	if(serItr->lastValue == end)
		return serItr->lastValue;

	//	If this is the first element
	if(serItr->counter == -1) {
		++serItr->counter;
		res = serItr->lastValue =  serItr->initialValue;		
	}
	//	Otherwise, step and validate
	else {
		++serItr->counter;
		//	First step
		PFRTAny iterRes;
		PFRTAny shouldStop=false;

		if(serItr->series == infinite) {
			iterRes = dispatch2(ss_defstep,serItr->lastValue,one);
		}
		else {
			if(serItr->series->step->ftype == integer_type) {
				iterRes =  dispatch2(ss_defstep,serItr->lastValue,serItr->series->step);
			}
			else {
				iterRes =  dispatch1(serItr->series->step,serItr->lastValue);	
			}
		}
		//	Then test end
		if(serItr->series == infinite)
			shouldStop = false;	// do nothing
		else if(serItr->series->stop->ftype == integer_type) {
			shouldStop =  dispatch2(ss_defend,iterRes,serItr->series->stop);
		}
		else if(serItr->series->stop->ftype == boolean_type) {
			shouldStop = serItr->series->stop;
		}
		else 
			shouldStop = dispatch1(serItr->series->stop,iterRes);

		if(shouldStop == true)
			res = serItr->lastValue = end;
		else {
			res = serItr->lastValue = iterRes;			
		}

	}
	return res;
}

PFRTIterator seriesiterator_initiate(PFRTIterator sitr) {
	PFRTSeries_Iterator serItr = (PFRTSeries_Iterator) sitr;
	if( serItr->series == infinite) {
		serItr->initialValue = zero;
	}
	else if(foidl_function_qmark(serItr->series->start) == true) {
		serItr->initialValue = dispatch0(serItr->series->start);	
	}
	else 
		serItr->initialValue = serItr->series->start;	
	return sitr;
}

PFRTIterator channeliterator_setup(PFRTIOChannel chan) {
	PFRTIterator citr = (PFRTIterator) nil;
	if(chan->openflag == open_read_only || 
		chan->openflag == open_read_write) {
		if(chan->bufferptr->ftype != no_buffer) {
			if(chan->readhandler == byte_reader)
				citr = allocChannelIterator(chan->bufferptr,io_nextByteReader);
			else if(chan->readhandler == char_reader)
				citr = allocChannelIterator(chan->bufferptr,io_nextCharReader);
			else 
				citr = allocChannelIterator(chan->bufferptr,io_nextStringReader);
		}
		else {
			foidl_fail();	
		}
	}
	else {
		foidl_fail();
	}

	return citr;
}

PFRTIterator iteratorFor(PFRTAny t) {
	PFRTIterator i = (PFRTIterator) nil;
	switch(t->fclass) {
		case 	scalar_class:
			switch(t->ftype) {
				case 	string_type:
				case 	integer_type:
				case 	character_type:
					unknown_handler();
			}
			break;
		case 	collection_class:
			switch(t->ftype) {
				case 	vector2_type:
					i = allocVectorIterator(
						(PFRTVector) t,					 	
						(itrNext) vectoriterator_next);
					break;
				case 	list2_type:
					i = allocListIterator(
						(PFRTList) t,
						(itrNext) listiterator_next);
					break;
				case 	map2_type:
				case 	set2_type:
					i = allocTrieIterator(
							(PFRTAssocType)t,							
							(itrNext) trieiterator_nextKey);
					break;
				case 	series_type:
					i = seriesiterator_initiate(
							allocSeriesIterator(
								(PFRTSeries) t,
								(itrNext) seriesiterator_next));
					break;
				default:
					unknown_handler();

			}
			break;
		case 	io_class:
			i = channeliterator_setup((PFRTIOChannel) t);
			break;
		default:
			unknown_handler();

	}
	return i;
} 

PFRTAny iteratorNext(PFRTIterator i) {
	return i->next(i);
}
