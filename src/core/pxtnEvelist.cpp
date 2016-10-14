#include <stdlib.h>
#include <string.h>

#include "./pxTypedef.h"

#include "./pxtnEvelist.h"

void pxtnEvelist::Release( void )
{
	if( _recs ) free( _recs );
	_recs    = NULL;
	_start   = NULL;
	_max_num =    0;
}

pxtnEvelist::pxtnEvelist( void )
{
	_recs    = NULL;
	_start   = NULL;
	_max_num =    0;
}

pxtnEvelist::~pxtnEvelist( void )
{
	pxtnEvelist::Release();
}

void pxtnEvelist::Clear( void )
{
	if( _recs ) memset( _recs, 0, sizeof(EVERECORD) * _max_num );
	_start   = NULL;
}


bool pxtnEvelist::Allocate( s32 max_num )
{
	pxtnEvelist::Release();
	if( !(  _recs = (EVERECORD*)malloc( sizeof(EVERECORD) * max_num ) ) ) return false;
	memset( _recs, 0,                   sizeof(EVERECORD) * max_num );
	_max_num = max_num;
	return true;
}

s32  pxtnEvelist::get_Num_Max( void ) const
{
	if( !_recs ) return 0;
	return _max_num;
}

s32  pxtnEvelist::get_Max_Clock( void ) const
{
	s32 max_clock = 0;
	s32 clock;

	for( EVERECORD* p = _start; p; p = p->next )
	{
		if( Evelist_Kind_IsTail( p->kind ) ) clock = p->clock + p->value;
		else                          clock = p->clock           ;
		if( clock > max_clock ) max_clock = clock;
	}

	return max_clock;

}

s32  pxtnEvelist::get_Count( void ) const
{
	if( !_recs || !_start ) return 0;

	s32 count = 0;
	for( EVERECORD* p = _start; p; p = p->next ) count++;
	return count;
}

s32  pxtnEvelist::get_Count( u8 kind, s32 value ) const
{
	if( !_recs ) return 0;

	s32 count = 0;
	for( EVERECORD* p = _start; p; p = p->next ){ if( p->kind == kind && p->value == value ) count++; }
	return count;
}

s32  pxtnEvelist::get_Count( u8 unit_no ) const
{
	if( !_recs ) return 0;

	s32 count = 0;
	for( EVERECORD* p = _start; p; p = p->next ){ if( p->unit_no == unit_no ) count++; }
	return count;
}

s32  pxtnEvelist::get_Count( u8 unit_no, u8 kind ) const
{
	if( !_recs ) return 0;

	s32 count = 0;
	for( EVERECORD* p = _start; p; p = p->next ){ if( p->unit_no == unit_no && p->kind == kind ) count++; }
	return count;
}

s32  pxtnEvelist::get_Count( s32 clock1, s32 clock2, u8 unit_no ) const
{
	if( !_recs ) return 0;

	EVERECORD* p;
	for( p = _start; p; p = p->next )
	{
		if( p->unit_no == unit_no )
		{
			if(                                   p->clock            >= clock1 ) break;
			if( Evelist_Kind_IsTail( p->kind ) && p->clock + p->value >  clock1 ) break;
		}
	}

	s32 count = 0;
	for(           ; p; p = p->next )
	{
		if( p->clock != clock1 && p->clock >= clock2 ) break;
		if( p->unit_no == unit_no ) count++;
	}
	return count;
}


static s32 _DefaultKindValue( u8 kind )
{
	switch( kind )
	{
//	case EVENTKIND_ON        : return ;
	case EVENTKIND_KEY       : return EVENTDEFAULT_KEY      ;
	case EVENTKIND_PAN_VOLUME: return EVENTDEFAULT_PAN_VOLUME  ;
	case EVENTKIND_VELOCITY  : return EVENTDEFAULT_VELOCITY ;
	case EVENTKIND_VOLUME    : return EVENTDEFAULT_VOLUME   ;
	case EVENTKIND_PORTAMENT : return EVENTDEFAULT_PORTAMENT;
	case EVENTKIND_BEATCLOCK : return EVENTDEFAULT_BEATCLOCK;
	case EVENTKIND_BEATTEMPO : return EVENTDEFAULT_BEATTEMPO;
	case EVENTKIND_BEATNUM   : return EVENTDEFAULT_BEATNUM  ;
//	case EVENTKIND_REPEAT    : return ;
//	case EVENTKIND_LAST      : return ;
	case EVENTKIND_VOICENO   : return EVENTDEFAULT_VOICENO  ;
	case EVENTKIND_GROUPNO   : return EVENTDEFAULT_GROUPNO  ;
	case EVENTKIND_CORRECT   :
		f32 correct;
		correct = EVENTDEFAULT_CORRECT;
		return *( (s32*)&correct );
	case EVENTKIND_PAN_TIME  : return EVENTDEFAULT_PAN_TIME ;
	}
	return 0;
}

s32 pxtnEvelist::get_Value( s32 clock, u8 unit_no, u8 kind ) const
{
	if( !_recs ) return 0;

	EVERECORD* p;
	s32        val = _DefaultKindValue( kind );

	for( p = _start; p; p = p->next )
	{
		if( p->clock > clock ) break;
		if( p->unit_no == unit_no && p->kind == kind ) val = p->value;
	}

	return val;
}

const EVERECORD* pxtnEvelist::get_Records( void ) const
{
	if( !_recs ) return NULL;
	return _start;
}


void pxtnEvelist::_SetRecord( EVERECORD* p_rec, EVERECORD* prev, EVERECORD* next, s32 clock, u8 unit_no, u8 kind, s32 value )
{
	if( prev ) prev->next = p_rec;
	else       _start     = p_rec;
	if( next ) next->prev = p_rec;

	p_rec->next    = next   ;
	p_rec->prev    = prev   ;
	p_rec->clock   = clock  ;
	p_rec->kind    = kind   ;
	p_rec->unit_no = unit_no;
	p_rec->value   = value  ;
}

static s32 _ComparePriority( u8 kind1, u8 kind2 )
{
	static const s32 priority_table[ EVENTKIND_NUM ] =
	{
		  0,//EVENTKIND_NULL  = 0
		 50,//EVENTKIND_ON       
		 40,//EVENTKIND_KEY      
		 60,//EVENTKIND_PAN_VOLUME  
		 70,//EVENTKIND_VELOCITY 
		 80,//EVENTKIND_VOLUME   
		 30,//EVENTKIND_PORTAMENT
		  0,//EVENTKIND_BEATCLOCK
		  0,//EVENTKIND_BEATTEMPO
		  0,//EVENTKIND_BEATNUM  
		  0,//EVENTKIND_REPEAT   
		255,//EVENTKIND_LAST     
		 10,//EVENTKIND_VOICENO  
		 20,//EVENTKIND_GROUPNO  
		 90,//EVENTKIND_CORRECT  
		100,//EVENTKIND_PAN_TIME  
	};

	return priority_table[ kind1 ] - priority_table[ kind2 ];
}

void pxtnEvelist::_CutRecord( EVERECORD* p_rec )
{
	if( p_rec->prev ) p_rec->prev->next = p_rec->next;
	else              _start            = p_rec->next;
	if( p_rec->next ) p_rec->next->prev = p_rec->prev;
	p_rec->kind = EVENTKIND_NULL;
}

bool pxtnEvelist::Record_Add_f( s32 clock, u8 unit_no, u8 kind, f32 value_f )
{
	s32 value = *( (s32*)(&value_f) );
	return Record_Add_i( clock, unit_no, kind, value );
}

bool pxtnEvelist::Record_Add_i( s32 clock, u8 unit_no, u8 kind, s32 value )
{
	if( !_recs ) return false;

	EVERECORD* p_new  = NULL;
	EVERECORD* p_prev = NULL;
	EVERECORD* p_next = NULL;

	// 空き検索
	for( s32 r = 0; r < _max_num; r++ )
	{
		if( _recs[ r ].kind == EVENTKIND_NULL )
		{
			p_new = &_recs[ r ];
			break;
		}
	}
	if( !p_new ) return false;

	// first.
	if( !_start )
	{
	}
	// top.
	else if( clock < _start->clock )
	{
		p_next = _start;
	}
	else
	{

		for( EVERECORD* p = _start; p; p = p->next )
		{
			if( p->clock == clock ) // 同時
			{
				for( ; true; p = p->next )
				{
					if( p->clock != clock                        ){ p_prev = p->prev; p_next = p; break; } 
					if( unit_no == p->unit_no && kind == p->kind ){ p_prev = p->prev; p_next = p->next; p->kind = EVENTKIND_NULL; break; } // 置き換え
					if( _ComparePriority( kind, p->kind ) < 0    ){ p_prev = p->prev; p_next = p; break; }// プライオリティを検査
					if( !p->next                                 ){ p_prev = p; break; }// 末端
				}
				break;
			}
			else if( p->clock > clock ){ p_prev = p->prev; p_next = p      ; break; } // 追い越した
			else if( !p->next         ){ p_prev = p; break; }// 末端
		}
	}

	_SetRecord( p_new, p_prev, p_next, clock, unit_no, kind, value );

	// cut prev tail
	if( Evelist_Kind_IsTail( kind ) )
	{
		for( EVERECORD* p = p_new->prev; p; p = p->prev )
		{
			if( p->unit_no == unit_no && p->kind == kind )
			{
				if( clock < p->clock + p->value ) p->value = clock - p->clock;
				break;
			}
		}
	}

	// delete next
	if( Evelist_Kind_IsTail( kind ) )
	{
		for( EVERECORD* p = p_new->next; p && p->clock < clock + value; p = p->next )
		{
			if( p->unit_no == unit_no && p->kind == kind )
			{
				_CutRecord( p );
			}
		}
	}

	return true;
}

s32 pxtnEvelist::Record_Delete( s32 clock1, s32 clock2, u8 unit_no, u8 kind )
{
	if( !_recs  ) return 0;

	s32 count = 0;

	for( EVERECORD* p = _start; p; p = p->next )
	{
		if( p->clock != clock1 && p->clock >= clock2 ) break;
		if( p->clock >= clock1 && p->unit_no == unit_no && p->kind == kind ){ _CutRecord( p ); count++; }
	}

	if( Evelist_Kind_IsTail( kind ) )
	{
		for( EVERECORD* p = _start; p; p = p->next )
		{
			if( p->clock >= clock1 ) break;
			if( p->unit_no == unit_no && p->kind == kind && p->clock + p->value > clock1 )
			{
				p->value = clock1 - p->clock;
				count++;
			}
		}
	}

	return count;
}

s32 pxtnEvelist::Record_Delete( s32 clock1, s32 clock2, u8 unit_no )
{
	if( !_recs  ) return 0;

	s32 count = 0;

	for( EVERECORD* p = _start; p; p = p->next )
	{
		if( p->clock != clock1 && p->clock >= clock2 ) break;
		if( p->clock >= clock1 && p->unit_no == unit_no ){ _CutRecord( p ); count++; }
	}

	for( EVERECORD* p = _start; p; p = p->next )
	{
		if( p->clock >= clock1 ) break;
		if( p->unit_no == unit_no && Evelist_Kind_IsTail( p->kind ) && p->clock + p->value > clock1 )
		{
			p->value = clock1 - p->clock;
			count++;
		}
	}

	return count;
}


s32 pxtnEvelist::Record_UnitNo_Miss( u8 unit_no )
{
	if( !_recs  ) return 0;

	s32 count = 0;

	for( EVERECORD* p = _start; p; p = p->next )
	{
		if(      p->unit_no == unit_no ){ _CutRecord( p ); count++; }
		else if( p->unit_no >  unit_no ){ p->unit_no--;    count++; }
	}
	return count;
}

s32 pxtnEvelist::Record_UnitNo_Set( u8 unit_no )
{
	if( !_recs  ) return 0;

	s32 count = 0;
	for( EVERECORD* p = _start; p; p = p->next ){ p->unit_no = unit_no; count++; }
	return count;
}

s32 pxtnEvelist::Record_UnitNo_Replace( u8 old_u, u8 new_u )
{
	if( !_recs  ) return 0;

	s32 count = 0;
	
	if( old_u == new_u ) return 0;
	if( old_u <  new_u )
	{
		for( EVERECORD* p = _start; p; p = p->next )
		{
			if(      p->unit_no == old_u                        ){ p->unit_no = new_u; count++; }
			else if( p->unit_no >  old_u && p->unit_no <= new_u ){ p->unit_no--;       count++; }
		}
	}
	else
	{
		for( EVERECORD* p = _start; p; p = p->next )
		{
			if(      p->unit_no == old_u                        ){ p->unit_no = new_u; count++; }
			else if( p->unit_no <  old_u && p->unit_no >= new_u ){ p->unit_no++;       count++; }
		}
	}

	return count;
}


s32 pxtnEvelist::Record_Value_Set( s32 clock1, s32 clock2, u8 unit_no, u8 kind, s32 value )
{
	if( !_recs  ) return 0;

	s32 count = 0;

	for( EVERECORD* p = _start; p; p = p->next )
	{
		if( p->unit_no == unit_no && p->kind == kind && p->clock >= clock1 && p->clock < clock2 )
		{
			p->value = value;
			count++;
		}
	}

	return count;
}

s32 pxtnEvelist::Record_Value_Change( s32 clock1, s32 clock2, u8 unit_no, u8 kind, s32 value )
{
	if( !_recs  ) return 0;

	s32 count = 0;

	s32 max, min;

	switch( kind )
	{
	case EVENTKIND_NULL      : max =      0; min =   0; break;
	case EVENTKIND_ON        : max =    120; min = 120; break;
	case EVENTKIND_KEY       : max = 0xbfff; min =   0; break;
	case EVENTKIND_PAN_VOLUME: max =   0x80; min =   0; break;
	case EVENTKIND_PAN_TIME  : max =   0x80; min =   0; break;
	case EVENTKIND_VELOCITY  : max =   0x80; min =   0; break;
	case EVENTKIND_VOLUME    : max =   0x80; min =   0; break;
	default: max = 0; min = 0;
	}

	for( EVERECORD* p = _start; p; p = p->next )
	{
		if( p->unit_no == unit_no && p->kind == kind && p->clock >= clock1 )
		{
			if( clock2 == -1 || p->clock < clock2 )
			{
				p->value += value;
				if( p->value < min ) p->value = min;
				if( p->value > max ) p->value = max;
				count++;
			}
		}
	}

	return count;
}

s32 pxtnEvelist::Record_Value_Omit( u8 kind, s32 value )
{
	if( !_recs  ) return 0;

	s32 count = 0;
	
	for( EVERECORD* p = _start; p; p = p->next )
	{
		if( p->kind == kind )
		{
			if(      p->value == value ){ _CutRecord( p ); count++; }
			else if( p->value >  value ){ p->value--;      count++; }
		}
	}
	return count;
}


s32 pxtnEvelist::Record_Value_Replace( u8 kind, s32 old_value, s32 new_value )
{
	if( !_recs  ) return 0;

	s32 count = 0;
	
	if( old_value == new_value ) return 0;
	if( old_value <  new_value )
	{
		for( EVERECORD* p = _start; p; p = p->next )
		{
			if( p->kind == kind )
			{
				if(      p->value == old_value                          ){ p->value = new_value; count++; }
				else if( p->value >  old_value && p->value <= new_value ){ p->value--;           count++; }
			}
		}
	}
	else
	{
		for( EVERECORD* p = _start; p; p = p->next )
		{
			if( p->kind == kind )
			{
				if(      p->value == old_value                          ){ p->value = new_value; count++; }
				else if( p->value <  old_value && p->value >= new_value ){ p->value++;           count++; }
			}
		}
	}

	return count;
}


s32 pxtnEvelist::Record_Clock_Shift( s32 clock, s32 shift, u8 unit_no )
{
	if( !_recs  ) return 0;
	if( !_start ) return 0;
	if( !shift  ) return 0;

	s32           count = 0;
	s32           c;
	u8            k;
	s32           v;
	EVERECORD*    p_next;
	EVERECORD*    p_prev;
	EVERECORD*    p = _start;


	if( shift < 0 )
	{
		for( ; p; p = p->next ){ if( p->clock >= clock ) break; }
		while( p )
		{
			if( p->unit_no == unit_no )
			{
				c      = p->clock + shift;
				k      = p->kind         ;
				v      = p->value        ;
				p_next = p->next;

				_CutRecord( p );
				if( c >= 0 ) Record_Add_i( c, unit_no, k, v );
				count++;

				p = p_next;
			}
			else
			{
				p = p->next;
			}
		}
	}
	else if( shift > 0 )
	{
		while( p->next ) p = p->next;
		while( p )
		{
			if( p->clock < clock ) break;

			if( p->unit_no == unit_no )
			{
				c      = p->clock + shift;
				k      = p->kind         ;
				v      = p->value        ;
				p_prev = p->prev;

				_CutRecord( p );
				Record_Add_i( c, unit_no, k, v );
				count++;

				p = p_prev;
			}
			else
			{
				p = p->prev;
			}
		}
	}
	return count;
}

/////////////////////
// linear
/////////////////////

bool pxtnEvelist::Linear_Start( void )
{
	if( !_recs ) return false;
	Clear(); _linear = 0;
	return true;
}


void pxtnEvelist::Linear_Add_i(  s32 clock, u8 unit_no, u8 kind, s32 value )
{
	EVERECORD* p = &_recs[ _linear ];

	p->clock      = clock  ;
	p->unit_no    = unit_no;
	p->kind       = kind   ;
	p->value      = value  ;

	_linear++;
}

void pxtnEvelist::Linear_Add_f( s32 clock, u8 unit_no, u8 kind, f32 value_f )
{
	s32 value = *( (s32*)(&value_f) );
	Linear_Add_i( clock, unit_no, kind, value );
}

void pxtnEvelist::Linear_End( bool b_connect )
{
	if( _recs[ 0 ].kind != EVENTKIND_NULL )
	{
		_start = &_recs[ 0 ];
	}

	if( b_connect )
	{
		for( s32 r = 1; r < _max_num; r++ )
		{
			if( _recs[ r ].kind == EVENTKIND_NULL ) break;
			_recs[ r     ].prev = &_recs[ r - 1 ];
			_recs[ r - 1 ].next = &_recs[ r     ];
		}
	}
}




bool pxtnEvelist::x4x_Read_Start( void )
{
	if( !_recs ) return false;
	Clear();
	_linear    =    0;
	_p_x4x_rec = NULL;
	return true;
}

void pxtnEvelist::x4x_Read_NewKind( void )
{
	_p_x4x_rec = NULL;
}

void pxtnEvelist::x4x_Read_Add( s32 clock, u8 unit_no, u8 kind, s32 value )
{
	EVERECORD* p_new  = NULL;
	EVERECORD* p_prev = NULL;
	EVERECORD* p_next = NULL;

	p_new = &_recs[ _linear++ ];

	// first.
	if( !_start )
	{
	}
	// top
	else if( clock < _start->clock )
	{
		p_next = _start;
	}
	else
	{
		EVERECORD* p;

		if( _p_x4x_rec ) p = _p_x4x_rec;
		else             p = _start    ;

		for( ; p; p = p->next )
		{
			if( p->clock == clock ) // 同時
			{
				for( ; true; p = p->next )
				{
					if( p->clock != clock                        ){ p_prev = p->prev; p_next = p; break; } 
					if( unit_no == p->unit_no && kind == p->kind ){ p_prev = p->prev; p_next = p->next; p->kind = EVENTKIND_NULL; break; } // 置き換え
					if( _ComparePriority( kind, p->kind ) < 0    ){ p_prev = p->prev; p_next = p; break; }// プライオリティを検査
					if( !p->next                                 ){ p_prev = p; break; }// 末端
				}
				break;
			}
			else if( p->clock > clock ){ p_prev = p->prev; p_next = p; break; } // 追い越した
			else if( !p->next         ){ p_prev = p; break; }// 末端
		}
	}
	_SetRecord( p_new, p_prev, p_next, clock, unit_no, kind, value );

	_p_x4x_rec = p_new;
}



// ------------
// io
// ------------


bool pxtnEvelist::io_Write( pxwrDoc *p_doc, s32 rough ) const
{
	s32 eve_num        = get_Count();
	s32 ralatived_size = 0;
	s32 absolute       = 0;
	s32 clock;
	s32 value;

	// サイズのチェック
	for( const EVERECORD* p = get_Records(); p; p = p->next )
	{
		clock    = p->clock - absolute;

		ralatived_size += pxwrDoc_v_chk( p->clock );
		ralatived_size += 1;
		ralatived_size += 1;
		ralatived_size += pxwrDoc_v_chk( p->value );

		absolute = p->clock;
	}

	// 書き込み
	s32 size = sizeof(s32) + ralatived_size;
	if( !p_doc->w( &size   , sizeof(u32), 1 ) ) return false;
	if( !p_doc->w( &eve_num, sizeof(s32)         , 1 ) ) return false;

	absolute = 0;

	for( const EVERECORD* p = get_Records(); p; p = p->next )
	{
		clock    = p->clock - absolute;

		if( Evelist_Kind_IsTail( p->kind ) ) value = p->value / rough;
		else                                 value = p->value        ;

		if( !p_doc->v_w( clock / rough, NULL )    ) return false;
		if( !p_doc->w  ( &p->unit_no, sizeof(u8), 1 ) ) return false;
		if( !p_doc->w  ( &p->kind   , sizeof(u8), 1 ) ) return false;
		if( !p_doc->v_w( value        , NULL )    ) return false;

		absolute = p->clock;
	}

	return true;
}

bool pxtnEvelist::io_Read( pxwrDoc *p_doc, bool *pb_new_fmt ) 
{
	s32 size    ;
	s32 eve_num ;

	if( !p_doc->r( &size   , 4, 1 ) ) return false;
	if( !p_doc->r( &eve_num, 4, 1 ) ) return false;

	s32 clock   ;
	s32 absolute = 0;
	u8  unit_no ;
	u8  kind    ;
	s32 value   ;

	for( s32 e = 0; e < eve_num; e++ )
	{
		if( !p_doc->v_r( &clock         ) ) return false;
		if( !p_doc->r  ( &unit_no, 1, 1 ) ) return false;
		if( !p_doc->r  ( &kind   , 1, 1 ) ) return false;
		if( !p_doc->v_r( &value         ) ) return false;
		absolute += clock;
		clock     = absolute;
		Linear_Add_i( clock, unit_no, kind, value );
	}

	return true;
}

s32 pxtnEvelist::io_Read_EventNum( pxwrDoc *p_doc ) const
{
	s32 size   ;
	s32 eve_num;

	if( !p_doc->r( &size   , 4, 1 ) ) return 0;
	if( !p_doc->r( &eve_num, 4, 1 ) ) return 0;

	s32 count = 0;
	s32 clock   ;
	u8  unit_no ;
	u8  kind    ;
	s32 value   ;

	for( s32 e = 0; e < eve_num; e++ )
	{
		if( !p_doc->v_r( &clock         ) ) return 0;
		if( !p_doc->r  ( &unit_no, 1, 1 ) ) return 0;
		if( !p_doc->r  ( &kind   , 1, 1 ) ) return 0;
		if( !p_doc->v_r( &value         ) ) return 0;
		count++;
	}
	if( count != eve_num ) return 0;

	return eve_num;
}





// イベント構造体(12byte) =================
typedef struct
{
	u16 unit_index;
	u16 event_kind;
	u16 data_num;        // １イベントのデータ数。現在は 2 ( clock / volume ）
	u16 rrr;
	u32 event_num;
}
_x4x_EVENTSTRUCT;

/*
// イベント単純化構造体
typedef struct
{
//	EVENTLIST *p_event;
	s32       unit_no;
	s32       event_kind;
	s32       event_num;
	s32       data_num;
	s32       size;
}
_x4x_SIMPLEEVENTSTRUCT;


// イベント単純化 / オフセット変換
static bool _x4x_SimplifyEvent(
	_SIMPLEEVENTSTRUCT *p_simple_event,
//	cls_EVENTLIST      *p_eventclass,
	Evelist*           *p_evelist,
	EVENTLIST          *p_event_read,
	s32 unit_no, s32 kind,
	bool bTailAbsolute, s32 rough )
{
	EVENTLIST *p_eve;

	p_simple_event->unit_no    = unit_no;
	p_simple_event->event_kind = kind;
	p_simple_event->event_num  = 0;
	p_simple_event->size       = 0;
	p_simple_event->p_event    = NULL;
	p_simple_event->data_num   = 2;

	p_eve = p_event_read;

	while( p_eve )
	{
		p_eventclass->AddEvent( &p_eve->data, &p_simple_event->p_event, p_eve->clock );
		p_simple_event->event_num++;
		p_eve = p_eve->next;
	}

	if( !p_simple_event->event_num ) return false;

	// オフセットに変換してからサイズを計算
	{
		s32        absolute;
		s32        offset;

		p_eve    = p_simple_event->p_event;
		absolute = 0;
		while( p_eve )
		{
			offset       = p_eve->clock  - absolute;
			absolute     = p_eve->clock;
			p_eve->clock = offset;
			if( bTailAbsolute && Evelist_Kind_IsTail( p_eve->data.kind ) )
			{
				absolute += p_eve->data.value;
			}
			p_eve = p_eve->next;
		}
	}

	// 粗くする
	if( rough > 1 )
	{
		p_eve = p_simple_event->p_event;
		while( p_eve )
		{
			p_eve->clock /= rough;
			if( Evelist_Kind_IsTail( p_eve->data.kind ) ) p_eve->data.value /= rough;
			p_eve = p_eve->next;
		}
	}

	p_eve = p_simple_event->p_event;
	while( p_eve )
	{

//		p_eve->clock *= 2;
//		if( status == EVENTKIND_ON )
//		{
//			p_eve->data.value *= 2;
//		}

		p_simple_event->size += pxwrDoc_v_chk( p_eve->clock       );
		p_simple_event->size += pxwrDoc_v_chk( p_eve->data.value  );
		p_eve = p_eve->next;
	}

	return true;
}

// 書き込み(イベント)
bool TuneData_Unit_Write_x4x_EVENT( FILE *fp, EVENTLIST *p_event, s32 u, s32 kind, bool bTailAbsolute, s32 rough )
{
	cls_EVENTLIST       eve;
	_EVENTSTRUCT        evnt;
	_SIMPLEEVENTSTRUCT  simple_event;
	s32                 e;
	EVENTLIST           *pe;
	u32                 size;

	if( !fp ) return false;

	// 準備
	if( !eve.Initialize( TuneData_Unit_GetMaxEvent() ) ) return false;

	memset( &simple_event, 0, sizeof(_SIMPLEEVENTSTRUCT) );
	if( !_SimplifyEvent( &simple_event, &eve, p_event, u, kind, bTailAbsolute, rough ) ){
		eve.Release();
		return true;
	}

	evnt.unit_no    = (u16)simple_event.unit_no;
	evnt.event_kind = (u16)simple_event.event_kind;
	evnt.event_num  = (u32)simple_event.event_num;
	evnt.data_num   = (u16)simple_event.data_num;
	evnt.rrr        = 0;

	// 書き出し
	size = sizeof( _EVENTSTRUCT ) + simple_event.size;
	if( fwrite( &size, sizeof(u32),  1, fp ) != 1 ) return false;
	if( fwrite( &evnt, sizeof( _EVENTSTRUCT ), 1, fp ) != 1 ) return false;

	e    = 0;
	pe   = simple_event.p_event;
	while( pe )
	{
		if( !p_doc->v_w( pe->clock,      fp, NULL ) ) break;
		if( !p_doc->v_w( pe->data.value, fp, NULL ) ) break;
		pe = pe->next;
		e++;
	}

	if( e != evnt.event_num ){
		eve.Release();
		return false;
	}

	return true;
}
*/

// 読み込み(イベント)
bool pxtnEvelist::io_Unit_Read_x4x_EVENT( pxwrDoc *p_doc, bool bTailAbsolute, bool bCheckRRR, bool *pb_new_fmt )
{
	_x4x_EVENTSTRUCT evnt;
	s32              clock;
	s32              value;
	s32              absolute;
	s32              e;
	s32              size;

	if( !p_doc->r( &size, 4,                          1 ) ) return false;
	if( !p_doc->r( &evnt, sizeof( _x4x_EVENTSTRUCT ), 1 ) ) return false;

	// 今のところデータ数 2 だけに対応。
	if( evnt.data_num != 2               ){ *pb_new_fmt = true; return false; }
	if( evnt.event_kind >= EVENTKIND_NUM ){ *pb_new_fmt = true; return false; }
	if( bCheckRRR && evnt.rrr            ){ *pb_new_fmt = true; return false; }

	// ユニットを探す
//	p_unit = TuneData_Unit_GetPointer   ( evnt.unit_index ); if( !p_unit ) return false;

	absolute = 0;
	for( e = 0; e < (s32)evnt.event_num; e++ )
	{
		if( !p_doc->v_r( &clock ) ) break;
		if( !p_doc->v_r( &value ) ) break;
		absolute += clock;
		clock     = absolute;
		x4x_Read_Add( clock, (u8)evnt.unit_index, (u8)evnt.event_kind, value );
		if( bTailAbsolute && Evelist_Kind_IsTail( evnt.event_kind ) ) absolute += value;
	}
	if( e != evnt.event_num ) return false;

	x4x_Read_NewKind();

	return true;
}

s32 pxtnEvelist::io_Read_x4x_EventNum( pxwrDoc *p_doc ) const
{
	_x4x_EVENTSTRUCT evnt;
	s32              work;
	s32              e;
	s32              size;

	if( !p_doc->r( &size, 4,                          1 ) ) return false;
	if( !p_doc->r( &evnt, sizeof( _x4x_EVENTSTRUCT ), 1 ) ) return false;

	// support only 2
	if( evnt.data_num != 2 ) return 0;

	for( e = 0; e < (s32)evnt.event_num; e++ )
	{
		if( !p_doc->v_r( &work ) ) break;
		if( !p_doc->v_r( &work ) ) break;
	}
	if( e != evnt.event_num ) return 0;

	return evnt.event_num;
}






























///////////////////////
// global
///////////////////////

bool Evelist_Kind_IsTail( s32 kind )
{
	if( kind == EVENTKIND_ON || kind == EVENTKIND_PORTAMENT ) return true;
	return false;
}


